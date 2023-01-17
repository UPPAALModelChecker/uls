#include "server_mock.h"
#include <uls/server.h>
#include <uls/highlight.h>

#include <iostream>
#include <sstream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using json = nlohmann::json;

TEST_CASE("close command"){
    auto mock = MockIO{};
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit")
        .start();
}

TEST_CASE("simple command"){
    auto mock = MockIO{};
    mock.send_cmd("set_value");
    mock.send_cmd("exit");

    bool value = false;

    auto server = Server{mock};
    server.add_close_command("exit")
        .add_simple_command<json>("set_value", [&](){value = true; return OK_RESPONSE;})
        .start();
    
    CHECK(value);
}

TEST_CASE("Return value"){
    auto mock = MockIO{};
    mock.send_cmd("get");
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit")
        .add_simple_command<json>("get", [](){return OK_RESPONSE;})
        .start();

    REQUIRE(mock.handshake());
    CHECK(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == OK_RESPONSE);
    CHECK(mock.output_empty());
}

struct Person{
    std::string name;
    int age;
};
template<>
struct Deserializer<Person>{
    static Person deserialize(const json& message){
        return {message["name"].get<std::string>(), message["age"].get<int>()};
    }
};

TEST_CASE("Test Deserializer"){
    auto mock = MockIO{};
    Person result;
    mock.send("set", {{"name", "Bob"}, {"age", 32}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit")
        .add_command<Person>("set", [&](Person person){ result = std::move(person); return OK_RESPONSE; })
        .start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.output_empty());
    
    CHECK(result.name == "Bob");
    CHECK(result.age == 32);
}

template<>
struct Serializer<Person>{
    static json serialize(const Person& person){
        return {{"name", person.name}, {"age", person.age}};
    }
};

TEST_CASE("Test Serializer"){
    auto mock = MockIO{};
    mock.send_cmd("get");
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit")
        .add_simple_command<Person>("get", []() -> Person { return {"Fred", 23}; })
        .start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == json{{"name", "Fred"}, {"age", 23}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.output_empty());
}

TEST_CASE("Stop command"){
    auto mock = MockIO{};
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.output_empty());
}