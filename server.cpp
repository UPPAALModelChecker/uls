#include <uls/server.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;
using json = nlohmann::json;

nlohmann::json OK_RESPONSE = {{"res", "OK"}};
nlohmann::json FAIL_RESPONSE = {{"res", "FAIL"}};

const Command& get_command(const std::vector<Command>& commands, const std::string& command_name){
    for(const auto& command : commands){
        if(command.name == command_name)
            return command;
    }

    std::cerr << "Unknown command" << std::endl;
    throw std::exception();
}

void Server::start(){
    is_running = true;
    io.out << "json" << std::endl;

    json message;
    while(is_running){
        io.in >> message;
        
        const auto& command = get_command(commands, message["cmd"].get<std::string>());
        auto response = command.callback(message["args"]);
        send("response/" + command.name, response);
    }
}


Server& Server::add_close_command(std::string name){
    add_simple_command<json>(std::move(name), [this](){ stop(); return OK_RESPONSE; });
    return *this;
}

void Server::send(const std::string& message_type, const nlohmann::json& message){
    // std::cerr << json{{"type", message_type}, {"msg", message}} << std::endl;
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