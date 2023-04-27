#include <uls/highlight.h>
#include <uls/server.h>
#include <uls/common_data.h>
#include <uls/utap_extension.h>

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <string_view>

struct Keyword{
    std::string_view identifier;
    std::string type;
    TextRange valid_range;
};

template<>
struct Serializer<Keyword>{
    static nlohmann::json serialize(const Keyword& keyword){
        return {{"id", keyword.identifier}, {"type", keyword.type}, {"start", keyword.valid_range.begOffset}, {"end", keyword.valid_range.endOffset}, };
    }
};

std::vector<Keyword> keywords_for_path(UTAP::Document& doc, const std::string& xpath){
    auto& decls = navigate_xpath(doc, xpath);

    auto keywords = std::vector<Keyword>{};
    DeclarationsWalker{doc, true}.visit_symbols(decls, [&](const UTAP::symbol_t& symbol, const TextRange& sym_range){
        if(symbol.get_type().is(UTAP::Constants::TYPEDEF)){
            keywords.push_back({symbol.get_name(), "KEYWORD3", sym_range});
        }
        return false;
    });
    return keywords;
}

void Highlight::configure(Server& server){
    server.add_command<std::string>("keywords", [this](std::string xpath){ return keywords_for_path(repository.get_document(), xpath); });
    
    repository.add_on_document_update([this, &server](UTAP::Document& doc){
        auto keywords = keywords_for_path(doc, repository.get_current_xpath());
        server.send_notification("notif/keywords", keywords);
    });

    repository.add_on_current_node_changed([this, &server](const std::string& xpath){
        auto keywords = keywords_for_path(repository.get_document(), xpath);
        server.send_notification("notif/keywords", keywords);
    });
}