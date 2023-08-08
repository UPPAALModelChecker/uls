#include <uls/renaming.h>
#include <uls/server.h>
#include <uls/declarations.h>
#include <uls/utap_extension.h>
#include <utap/statement.h>
#include <vector>
#include <set>
#include <iostream>

using UTAP::Constants::kind_t;

template<typename Func>
void expr_accept(UTAP::expression_t expr, Func f){
    if(expr.empty())
        return;

    if(expr.get_kind() == kind_t::IDENTIFIER){
        f(expr);
    }

    for(size_t i = 0; i < expr.get_size(); ++i){
        expr_accept(expr.get(i), f);
    }
}

std::set<UTAP::symbol_t> get_symbols(const UTAP::expression_t& expr){
    std::set<UTAP::symbol_t> symbols;
    expr.get_symbols(symbols);
    return symbols;
}


class UsageFinder : public UTAP::DocumentVisitor, public UTAP::ExpressionVisitor {
    UTAP::symbol_t target;
    UTAP::frame_t parent_scope;
    std::vector<TextLocation> matches;

    UTAP::Document* doc;

    virtual void visitDocBefore(UTAP::Document& doc) override {
        matches.emplace_back(doc, target.get_position());
        this->doc = &doc;
        if(is_symbol_visible(doc.get_globals().frame)){
            check_declarations(doc.get_globals());
            for(const UTAP::instance_t& instance : doc.get_instances()){
                if(instance.templ->uid == target){
                    throw std::logic_error("Cannot rename template names");
                }
            }
            std::cerr << "Query count: " << doc.get_queries().size() << '\n'; 
        }
        else{
            for(UTAP::function_t& func : doc.get_globals().functions){
                if(is_symbol_visible(func.body->get_frame()))
                    func.body->accept(this);
            }
        }

    }

    virtual bool visitTemplateBefore(UTAP::template_t& templ) override { 
        return is_symbol_visible(templ.frame);
    }

    virtual void visitTemplateAfter(UTAP::template_t& templ) override {
        check_declarations(templ);
        for(UTAP::location_t location : templ.locations){
            check_expr(location.cost_rate);
            check_expr(location.exp_rate);
            check_expr(location.invariant);
            check_expr(location.name);
        }
        for(UTAP::edge_t edge : templ.edges){
            check_expr(edge.assign);
            check_expr(edge.guard);
            check_expr(edge.sync);
            check_expr(edge.prob);
        }
    }

    virtual void visitExpression(UTAP::expression_t expr) override{
        check_expr(expr);
    }

    virtual void visitTypeDef(UTAP::symbol_t symbol) override {
        check_type(symbol.get_type());
    }

    void check_declarations(UTAP::declarations_t& decls){
        for(UTAP::variable_t var : decls.variables){
            check_expr(var.init);
            if(!var.uid.get_type().is(UTAP::Constants::LABEL)) // Label is used as a prefix for type references
                check_type(var.uid.get_type());
        }
        for(UTAP::function_t& func : decls.functions){
            func.body->accept(this);
        }
    }

    void check_type(UTAP::type_t type){
        // Catches case where array size is defined with a typedef
        if(type.is_array() && type.get(1).is(UTAP::Constants::LABEL))
            return;

        check_expr(type.get_expression());
        for(size_t i = 0; i < type.size(); ++i){
            check_type(type.get(i));
        }
    }

    void check_expr(UTAP::expression_t expr){
        expr_accept(expr, [this](const UTAP::expression_t& expr){
            if(expr.get_symbol() == target){
                matches.emplace_back(*doc, expr.get_position());
            }
        });
    }

    bool is_symbol_visible(UTAP::frame_t frame){
        if(frame == parent_scope)
            return true;

        while(frame.has_parent()){
            frame = frame.get_parent();

            if(frame == parent_scope)
                return true;
        }

        return false;
    }
public:
    UsageFinder(UTAP::symbol_t target, UTAP::frame_t frame): target{std::move(target)}, parent_scope{std::move(frame)}{}
    std::vector<TextLocation> get_matches(){
        return matches;
    }
};

std::vector<TextLocation> find_usages(SystemRepository& doc_repo, const Identifier& id){
    UTAP::Document& doc = doc_repo.get_document();
    auto& decls = navigate_xpath(doc, id.xpath, id.offset);
    UTAP::symbol_t symbol = std::get<UTAP::symbol_t>(find_declaration(doc, decls, id.identifier).value());
    if(symbol.get_type().is(UTAP::Constants::INSTANCE))
        throw std::logic_error{"Cannot rename processes"};
    
    UsageFinder usages{symbol, symbol.get_frame()};
    doc.accept(usages);

    return usages.get_matches();
}


void RenamingModule::configure(Server& server) {
    server.add_command<Identifier>("find_usages", [this](const Identifier& id){
        return find_usages(doc, id);
    });
}