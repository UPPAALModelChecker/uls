#pragma once
#include "system.h"
#include "common_data.h"
#include <utap/document.h>
#include <optional>
#include <variant>

std::optional<UTAP::symbol_t> find_declaration(UTAP::Document& doc, const Identifier& id);

class DeclarationsModule : public ServerModule {
    SystemRepository& doc;
    
public:
    DeclarationsModule(SystemRepository& doc): doc{doc} {}

    void configure(Server& server) override;
};