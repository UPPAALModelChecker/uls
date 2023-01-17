#pragma once
#include <string>
#include <functional>
#include <vector>
#include <nlohmann/json.hpp>
#include <iosfwd>

class Server;

class ServerModule{
public:
    virtual void configure(Server& server) = 0;
};

struct Command{
    std::string name;
    std::function<nlohmann::json(nlohmann::json&)> callback;
};

extern nlohmann::json OK_RESPONSE;

template<typename Data>
struct Deserializer{};
template<typename Data>
struct Serializer{};

// Define shared serializer/deserializer
template<>
struct Deserializer<std::string>{
    static std::string deserialize(const nlohmann::json& message);
};
template<>
struct Serializer<std::string>{
    static nlohmann::json serialize(const std::string& str);
};


template<>
struct Deserializer<const nlohmann::json&>{
    static const nlohmann::json& deserialize(const nlohmann::json& message) { return message; }
};
template<>
struct Serializer<nlohmann::json>{
    static nlohmann::json serialize(nlohmann::json message) { return std::move(message); }
};


template<typename ItemType>
struct Deserializer<std::vector<ItemType>>{
    static std::vector<ItemType> deserialize(const nlohmann::json& message){
        std::vector<ItemType> data;
        for(const auto& item : message)
            data.push_back(Deserializer<ItemType>::deserialize(item));
        return data;
    }
};
template<typename ItemType>
struct Serializer<std::vector<ItemType>>{
    static nlohmann::json serialize(const std::vector<ItemType>& data){
        nlohmann::json json_array;
        for(const auto& item : data)
            json_array.push_back(Serializer<ItemType>::serialize(item));
        return json_array;
    }
};

struct IOStream{
    std::istream& in;
    std::ostream& out;
};

class Server {
    std::vector<Command> commands;
    IOStream io;
    bool is_running{false};

    void send(const std::string& message_type, const nlohmann::json& message);

public:
    Server(IOStream io) : io(std::move(io)) {}
    Server& add_module(ServerModule& server_module);
    template<typename ReturnType>
    Server& add_simple_command(std::string name, std::function<ReturnType()> callback){
        commands.push_back({name, [callback](nlohmann::json&){
            return Serializer<ReturnType>::serialize(callback());
        }});
        return *this;
    }
    template<typename Data, typename Func>
    Server& add_command(std::string name, Func callback){
        commands.push_back({std::move(name), [callback](nlohmann::json& message) -> nlohmann::json {
            auto result = callback(Deserializer<Data>::deserialize(message));
            return Serializer<std::invoke_result_t<Func, Data>>::serialize(std::move(result));
        }});
        return *this;
    }
    Server& add_close_command(std::string name);
    template<typename Data>
    void send_notification(const std::string& type, Data&& element){
        auto message = Serializer<std::remove_reference_t<Data>>::serialize(std::forward<Data>(element));
        send(type, message);
    }
    void start();
    void stop();
};  