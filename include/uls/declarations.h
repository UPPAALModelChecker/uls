#pragma once
#include "system.h"

class DeclarationsModule : public ServerModule {
    SystemRepository& doc;
    
public:
    DeclarationsModule(SystemRepository& doc): doc{doc} {}

    void configure(Server& server) override;
};