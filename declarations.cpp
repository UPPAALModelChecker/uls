#include <uls/declarations.h>
#include <uls/server.h>
#include "utap_extension.h"
#include <string>
#include <iostream>
#include <optional>
#include <memory>
#include <variant>
#include <exception>

struct GotoParams{
    std::string xpath;
    uint32_t offset;
    std::string identifier;
};

struct GotoResult{
    std::shared_ptr<std::string> path;
    uint32_t start;
    uint32_t end;
};

template<>
struct Deserializer<GotoParams> {
    static GotoParams deserialize(const nlohmann::json& message) {
        return {
            message["xpath"].get<std::string>(),
            message["offset"].get<uint32_t>(),
            message["identifier"].get<std::string>()
        };
    }
};

template<>
struct Serializer<GotoResult> {
    static nlohmann::json serialize(const GotoResult& result){
        return {
            {"xpath", *result.path},
            {"start", result.start},
            {"end", result.end}
        };
    }
};

struct Sym{
    UTAP::type_t type;
    UTAP::position_t position;
};

std::optional<Sym> find_symbol(UTAP::Document& doc, UTAP::declarations_t& decls, std::string_view name){
    std::optional<Sym> result{};
    DeclarationsWalker{doc, false}.visit_symbols(decls, [&](const UTAP::symbol_t& symbol, const TextRange& range){
        if(!result.has_value() && symbol.get_name() == name){
            result = std::make_optional<Sym>(Sym{symbol.get_type(), symbol.get_position()});
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
}

std::optional<GotoResult> find_declaration(UTAP::Document& doc, const GotoParams& params){
    UTAP::declarations_t& decls = navigate_xpath(doc, params.xpath, params.offset);
    
    auto identifier = std::string_view(params.identifier);
    SymFinder finder{doc, decls};

    auto name_it = DotSeparator{identifier};
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

    std::optional<Sym> symbol = finder.find(label);
    if(!symbol.has_value())
        return std::nullopt;

    auto pos = symbol->position;
    auto line = doc.find_first_position(pos.start);
    uint32_t offset = pos.start - line.position;
    uint32_t length = pos.end - pos.start;

    return std::make_optional(GotoResult{
        line.path,
        offset,
        offset + length
    });
}

nlohmann::json find(SystemRepository& doc_repo, const GotoParams& params) {
    UTAP::Document& doc = doc_repo.get_document();

    auto result_opt = find_declaration(doc, params);

    // TODO use exceptions instead
    if(result_opt.has_value() && !(*result_opt->path == "/nta/system" && params.xpath.find("/template[") != std::string::npos))
        return Serializer<GotoResult>::serialize(*result_opt);
    else
        return FAIL_RESPONSE;
}

void DeclarationsModule::configure(Server& server){
    server.add_command<GotoParams>("goto_decl", [this](const GotoParams& params){
        return find(doc, params);
    });
}