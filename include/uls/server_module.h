#pragma once

template <typename Data>
struct Deserializer
{};
template <typename Data>
struct Serializer
{};

class Server;

class ServerModule
{
public:
    virtual void configure(Server& server) = 0;
};