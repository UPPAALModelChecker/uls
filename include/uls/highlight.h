#pragma once
#include "system.h"

/**
 * The highlight module is used to generate text highlighting hints for the user interface
 */
class Highlight : public ServerModule
{
    SystemRepository& repository;

public:
    Highlight(SystemRepository& repo): repository{repo} {}

    void configure(Server& server) override;
};