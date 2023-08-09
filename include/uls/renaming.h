#pragma once
#include "system.h"

class RenamingModule : public ServerModule
{
    SystemRepository& doc;

public:
    void configure(Server& server) override;
    RenamingModule(SystemRepository& doc_repo): doc{doc_repo} {}
};