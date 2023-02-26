#pragma once

class Server;

class ServerModule{
public:
    virtual void configure(Server& server) = 0;
};