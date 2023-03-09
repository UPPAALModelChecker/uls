#include <uls/declarations.h>
#include <uls/server.h>
#include "utap_extension.h"
#include <string>
#include <iostream>
#include <memory>
#include <variant>
#include <exception>

struct Sym{
    UTAP::type_t type;
    UTAP::position_t position;
    UTAP::symbol_t symbol;

    explicit Sym(UTAP::symbol_t symbol): type{symbol.get_type()}, position{symbol.get_position()}, symbol{std::move(symbol)} {}
    Sym(UTAP::type_t type, UTAP::position_t position): type{std::move(type)}, position{position} {}
};

std::optional<Sym> find_symbol(UTAP::Document& doc, UTAP::declarations_t& decls, std::string_view name){
    std::optional<Sym> result{};
    DeclarationsWalker{doc, false}.visit_symbols(decls, [&](const UTAP::symbol_t& symbol, const TextRange& range){
        if(!result.has_value() && symbol.get_name() == name){
            result = std::make_optional<Sym>(Sym{symbol});
            //TODO: add early termination when symbol is found
        }
    });
    return result;
}

class DotSeparator{
    std::string_view str;
public:
    explicit DotSeparator(std::string_view str): str{str} {}

    bool has_next(){
        return str.size() > 0;
    }

    std::string_view next(){
        auto pos = str.find_first_of(".");
        auto result = str.substr(0, pos);
        
        if(pos == std::string::npos)
            str = std::string_view{""};
        else
            str = str.substr(pos + 1);
        
        return result;
    }
};

std::optional<Sym> find_symbol(UTAP::type_t struct_type, std::string_view name){
    if(struct_type.is_constant())
        struct_type = struct_type.get(0);

    for(int i = 0; i < struct_type.size(); ++i){
        if(struct_type.get_label(i) == name)
            return std::make_optional<Sym>(Sym{struct_type.get(i), struct_type.get_field_position(i)});
    }

    return std::nullopt;
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class SymFinder{
public:
    using SymbolSource = std::variant<UTAP::declarations_t*, UTAP::type_t>;
    SymFinder(UTAP::Document& doc, UTAP::declarations_t& source): doc{doc}, symbol_source{&source} {}
    void set_source(UTAP::type_t source) { symbol_source = SymbolSource{std::move(source)}; }
    void set_source(UTAP::declarations_t& source) { symbol_source = SymbolSource{&source}; }

    std::optional<Sym> find(std::string_view name){
        return std::visit(overloaded{
            [&, this](UTAP::declarations_t* decls) { return find_symbol(doc, *decls, name); },
            [&name](UTAP::type_t struct_type) { return find_symbol(struct_type, name); }
        }, symbol_source);
    }

private:
    UTAP::Document& doc;
    SymbolSource symbol_source;
};

UTAP::declarations_t& find_process(UTAP::Document& doc, std::string_view name){
    for(auto& process : doc.get_processes()){
        if(process.uid.get_name() == name){
            if(process.templ)
                return *process.templ;
            else
                throw std::logic_error("Process did not have a template");
        }
    }
    throw std::logic_error("No such process");
}

std::optional<Sym> find_sym(UTAP::Document& doc, const Identifier& id){
    UTAP::declarations_t& decls = navigate_xpath(doc, id.xpath, id.offset);

    SymFinder finder{doc, decls};
    auto name_it = DotSeparator{id.identifier};
    auto label = name_it.next();
    while(name_it.has_next()){
        auto struct_symbol = finder.find(label);
        if(struct_symbol.has_value()){
            if(struct_symbol->type.is_constant())
                finder.set_source(struct_symbol->type.get(0).get(0));
            else if(struct_symbol->type.is(UTAP::Constants::INSTANCE))
                finder.set_source(find_process(doc, label));
            else
                finder.set_source(struct_symbol->type.get(0));
        }else{
            return std::nullopt;
        }

        label = name_it.next();
    }

    return finder.find(label);
}


std::optional<UTAP::symbol_t> find_declaration(UTAP::Document& doc, const Identifier& params){
    std::optional<Sym> sym = find_sym(doc, params);
    if(!sym.has_value() || sym->symbol == UTAP::symbol_t{})
        return std::nullopt;

    return std::make_optional(sym->symbol);
}

std::optional<TextLocation> find_goto_result(UTAP::Document& doc, const Identifier& params){
    std::optional<Sym> symbol = find_sym(doc, params);
    if(!symbol.has_value())
        return std::nullopt;

    return std::make_optional(TextLocation{
        doc,
        symbol->position
    });
}

nlohmann::json find(SystemRepository& doc_repo, const Identifier& params) {
    UTAP::Document& doc = doc_repo.get_document();

    auto result_opt = find_goto_result(doc, params);

    // TODO use exceptions instead
    if(result_opt.has_value() && !(*result_opt->path == "/nta/system" && params.xpath.find("/template[") != std::string::npos))
        return Serializer<TextLocation>::serialize(*result_opt);
    else
        return FAIL_RESPONSE;
}

void DeclarationsModule::configure(Server& server){
    server.add_command<Identifier>("goto_decl", [this](const Identifier& params){
        return find(doc, params);
    });
}