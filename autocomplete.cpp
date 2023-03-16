#include <uls/autocomplete.h>
#include <uls/server.h>
#include <uls/common_data.h>
#include <uls/declarations.h>
#include "utap_extension.h"
#include <sstream>

void AutocompleteModule::configure(Server& server){
    server.add_command<Identifier>("autocomplete", [this](const Identifier& id) -> std::vector<std::string> {
        auto items = std::vector<std::string>{};
        UTAP::Document& doc = doc_repo.get_document();

        UTAP::declarations_t& decls = navigate_xpath(doc, id.xpath, id.offset);

        auto offset = id.identifier.find_last_of('.');
        if(offset != std::string::npos){
            Identifier newId = id;
            newId.identifier = newId.identifier.substr(0, offset);
            std::optional<UTAP::symbol_t> sym_opt = find_declaration(doc, id);
            if(sym_opt.has_value()){
                UTAP::type_t struct_type = sym_opt->get_type().get(0);
                for(int i = 0; i < struct_type.size(); i++){
                    items.push_back(newId.identifier + "." + struct_type.get_label(i));
                }
            }else{
                items.push_back("Found nothing");
            }
        }else{
            DeclarationsWalker{doc, false}.visit_symbols(decls, [&](UTAP::symbol_t& symbol, const TextRange& range){
                if(symbol.get_frame() != decls.frame || range.begOffset < id.offset){
                    items.push_back(symbol.get_name());
                }
            });
        }


        return items;
    });
}