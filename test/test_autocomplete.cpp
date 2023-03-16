#include "server_mock.h"
#include <uls/autocomplete.h>

#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

const std::string MODEL =  R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
    <declaration>
typedef struct {
    int x;
    int y;
    int z;
} point;

point p_a;



typedef struct {
    point pos;
    int size;
} Entity;

Entity e;
    

    </declaration>
	<template>
		<name x="5" y="5">Template</name>
		<declaration>int y = x + 2;
        void f(){
            const int x = 32;
        }
        </declaration>
		<location id="id0" x="-76" y="-68">
			<name x="-86" y="-102">Init</name>
		</location>
		<init ref="id0"/>
	</template>
	<system>
int z = 5;
p = Template();
system p;
</system>
</nta>)";

using json = nlohmann::json;

TEST_CASE("Autocomplete struct type"){
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("autocomplete", {{"xpath", "/nta/declaration!"}, {"identifier", "p_a."}, {"offset", 77}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit")
        .add_module(repo)
        .add_module(autocomplete)
        .start();
    
    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == json{{"p_a.x"}, {"p_a.y"}, {"p_a.z"}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.output_empty());
}

TEST_CASE("Autocomplete nested struct type"){
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("autocomplete", {{"xpath", "/nta/declaration!"}, {"identifier", "e.pos."}, {"offset", 147}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit")
        .add_module(repo)
        .add_module(autocomplete)
        .start();
    
    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == json{{"e.pos.x"}, {"e.pos.y"}, {"e.pos.z"}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.output_empty());
}