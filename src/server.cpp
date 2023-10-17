#include <uls/server.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <ranges>

namespace views = std::ranges::views;
using namespace std::chrono_literals;
using json = nlohmann::json;

nlohmann::json OK_RESPONSE = {{"res", "OK"}};
nlohmann::json FAIL_RESPONSE = {{"res", "FAIL"}};

void Server::start()
{
    is_running = true;
    io.out << "json" << std::endl;

    json message;

    while (is_running) {
        try {
            io.in >> message;
            auto name = message["cmd"].get<std::string>();
            auto it = std::find_if(commands.begin(), commands.end(),
                                   [&name](const Command& cmd) { return cmd.name == name; });
            if (it == commands.end())
                throw std::invalid_argument("Unknown command " + name);
            auto response = it->callback(message["args"]);
            send("response/" + it->name, response);
        } catch (nlohmann::json::parse_error& e) {
            send("error", e.what());
            stop();
        } catch (std::exception& e) {
            send("error", e.what());
        }
    }
}

Server& Server::add_close_command(std::string name)
{
    add_simple_command<json>(std::move(name), [this]() {
        stop();
        return OK_RESPONSE;
    });
    return *this;
}

void Server::send(const std::string& message_type, const nlohmann::json& message)
{
    io.out << json{{"type", message_type}, {"msg", message}} << std::endl;
}

void Server::stop() { is_running = false; }

Server& Server::add_module(ServerModule& server_module)
{
    server_module.configure(*this);
    return *this;
}

std::string Deserializer<std::string>::deserialize(const json& message) { return message.get<std::string>(); }

json Serializer<std::string>::serialize(const std::string& str) { return {str}; }