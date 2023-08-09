#pragma once
#include "system.h"
#include "common_data.h"
#include <utap/document.h>
#include <optional>
#include <uls/utap_extension.h>

std::optional<UtapEntity> find_declaration(UTAP::Document& doc, UTAP::declarations_t& decls,
                                           std::string_view identifier);

class DeclarationsModule : public ServerModule
{
    SystemRepository& doc;

public:
    DeclarationsModule(SystemRepository& doc): doc{doc} {}

    void configure(Server& server) override;
};