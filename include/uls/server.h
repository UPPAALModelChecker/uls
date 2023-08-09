#pragma once
#include "server_module.h"
#include "serialization.h"

#include <string>
#include <functional>
#include <vector>
#include <iosfwd>

struct Command
{
    std::string name;
    std::function<nlohmann::json(nlohmann::json&)> callback;
};

extern nlohmann::json OK_RESPONSE;
extern nlohmann::json FAIL_RESPONSE;

struct IOStream
{
    std::istream& in;
    std::ostream& out;
};

class Server
{
    std::vector<Command> commands;
    IOStream io;
    bool is_running{false};

    void send(const std::string& message_type, const nlohmann::json& message);

public:
    Server(IOStream io): io(std::move(io)) {}
    Server& add_module(ServerModule& server_module);

    template <typename ReturnType>
    Server& add_simple_command(std::string name, std::function<ReturnType()> callback)
    {
        commands.push_back(
            {name, [callback](nlohmann::json&) { return Serializer<ReturnType>::serialize(callback()); }});
        return *this;
    }

    template <typename Data, typename Func>
    Server& add_command(std::string name, Func callback)
    {
        commands.push_back({std::move(name), [callback](nlohmann::json& message) -> nlohmann::json {
                                auto result = callback(Deserializer<Data>::deserialize(message));
                                return Serializer<std::invoke_result_t<Func, Data>>::serialize(std::move(result));
                            }});
        return *this;
    }

    Server& add_close_command(std::string name);

    template <typename Data>
    void send_notification(const std::string& type, Data&& element)
    {
        auto message = Serializer<std::remove_reference_t<Data>>::serialize(std::forward<Data>(element));
        send(type, message);
    }

    void start();
    void stop();
};