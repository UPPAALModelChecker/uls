#pragma once
#include "system.h"

class AutocompleteModule : public ServerModule{
    SystemRepository& doc_repo;

public:
    AutocompleteModule(SystemRepository& repo) : doc_repo{repo} {}
    void configure(Server& server) override;
};