#include <uls/declarations.h>
#include <uls/server.h>
#include <string>
#include <iostream>
#include <memory>
#include <variant>
#include <exception>

struct Sym
{
    UTAP::type_t type;
    UTAP::position_t position;
    UTAP::symbol_t symbol;

    explicit Sym(UTAP::symbol_t symbol):
        type{symbol.get_type()}, position{symbol.get_position()}, symbol{std::move(symbol)}
    {}
    Sym(UTAP::type_t type, UTAP::position_t position): type{std::move(type)}, position{position} {}
};

std::optional<Sym> find_symbol(UTAP::Document& doc, UTAP::declarations_t& decls, std::string_view name)
{
    std::optional<Sym> result{};
    DeclarationsWalker{doc, false}.visit_symbols(decls, [&](const UTAP::symbol_t& symbol, const TextRange&) {
        if (!result.has_value() && symbol.get_name() == name) {
            result = std::make_optional<Sym>(Sym{symbol});
            return true;  // Terminate early
        }
        return false;
    });
    return result;
}

class DotSeparator
{
    std::string_view str;

public:
    explicit DotSeparator(std::string_view str): str{str} {}

    bool has_next() { return str.size() > 0; }

    std::string_view next()
    {
        auto pos = str.find_first_of(".");
        auto result = str.substr(0, pos);

        if (pos == std::string::npos)
            str = std::string_view{""};
        else
            str = str.substr(pos + 1);

        return result;
    }
};

std::optional<Sym> find_symbol(UTAP::type_t struct_type, std::string_view name)
{
    if (struct_type.is_constant())
        struct_type = struct_type.get(0);

    for (size_t i = 0; i < struct_type.size(); ++i) {
        if (struct_type.get_label(i) == name)
            return std::make_optional<Sym>(Sym{struct_type.get(i), struct_type.get_field_position(i)});
    }

    return std::nullopt;
}

class SymFinder
{
public:
    using SymbolSource = std::variant<UTAP::declarations_t*, UTAP::type_t>;
    SymFinder(UTAP::Document& doc, UTAP::declarations_t& source): doc{doc}, symbol_source{&source} {}
    void set_source(SymbolSource source) { symbol_source = std::move(source); }

    std::optional<Sym> find(std::string_view name)
    {
        return std::visit(overloaded{[&, this](UTAP::declarations_t* decls) { return find_symbol(doc, *decls, name); },
                                     [&name](UTAP::type_t struct_type) { return find_symbol(struct_type, name); }},
                          symbol_source);
    }

private:
    UTAP::Document& doc;
    SymbolSource symbol_source;
};

std::optional<Sym> find_sym(UTAP::Document& doc, UTAP::declarations_t& decls, std::string_view id)
{
    SymFinder finder{doc, decls};
    auto name_it = DotSeparator{id};
    auto label = name_it.next();
    while (name_it.has_next()) {
        auto struct_symbol = finder.find(label);
        if (struct_symbol.has_value()) {
            if (struct_symbol->type.is_constant())
                finder.set_source(struct_symbol->type.get(0).get(0));
            else if (struct_symbol->type.is(UTAP::Constants::INSTANCE)){
                if(auto opt_process = find_process(doc, label))
                    finder.set_source(&static_cast<UTAP::template_t&>(*opt_process));
            } else
                finder.set_source(struct_symbol->type.get(0));
        } else {
            return std::nullopt;
        }

        label = name_it.next();
    }

    return finder.find(label);
}

std::optional<Sym> find_sym(UTAP::Document& doc, const Identifier& id)
{
    UTAP::declarations_t& decls = navigate_xpath(doc, id.xpath, id.offset);
    return find_sym(doc, decls, id.identifier);
}

std::optional<UtapEntity> find_declaration(UTAP::Document& doc, UTAP::declarations_t& decls,
                                           std::string_view identifier)
{
    if (std::optional<Sym> sym = find_sym(doc, decls, identifier)) {
        if (sym->symbol != UTAP::symbol_t{})
            return std::make_optional(sym->symbol);
        else
            return std::make_optional(sym->type);
    }

    return std::nullopt;
}

std::optional<TextLocation> find_goto_result(UTAP::Document& doc, const Identifier& params)
{
    if (std::optional<Sym> symbol = find_sym(doc, params))
        return std::make_optional(TextLocation{doc, symbol->position});
    else
        return std::nullopt;
}

nlohmann::json find(SystemRepository& doc_repo, const Identifier& params)
{
    UTAP::Document& doc = doc_repo.get_document();

    auto result_opt = find_goto_result(doc, params);

    // TODO use exceptions instead
    if (result_opt.has_value() &&
        !(*result_opt->path == "/nta/system" && params.xpath.find("/template[") != std::string::npos))
        return Serializer<TextLocation>::serialize(*result_opt);
    else
        return FAIL_RESPONSE;
}

void DeclarationsModule::configure(Server& server)
{
    server.add_command<Identifier>("goto_decl", [this](const Identifier& params) { return find(doc, params); });
}