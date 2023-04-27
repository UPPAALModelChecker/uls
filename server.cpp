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

const Command& get_command(const std::vector<Command>& commands, const std::string& command_name){
    for(const auto& command : commands){
        if(command.name == command_name)
            return command;
    }
    throw std::invalid_argument("Unknown command name " + command_name);
}

void Server::start(){
    is_running = true;
    io.out << "json" << std::endl;

    json message;

    while(is_running){
        try{
            io.in >> message;
            
            const Command& command = get_command(commands, message["cmd"].get<std::string>());
            auto response = command.callback(message["args"]);
            send("response/" + command.name, response);
        }catch(nlohmann::json::parse_error& e){
            send("error", e.what());
            stop();
        }
        catch(std::exception& e){
            send("error", e.what());
        }
    }
}


Server& Server::add_close_command(std::string name){
    add_simple_command<json>(std::move(name), [this](){ stop(); return OK_RESPONSE; });
    return *this;
}

void Server::send(const std::string& message_type, const nlohmann::json& message){
    io.out << json{{"type", message_type}, {"msg", message}} << std::endl;
}

void Server::stop(){
    is_running = false;
}

Server& Server::add_module(ServerModule& server_module){
    server_module.configure(*this);
    return *this;
}

std::string Deserializer<std::string>::deserialize(const json& message){
    return message.get<std::string>();
}

json Serializer<std::string>::serialize(const std::string& str){
    return {str};
}