#include "server_mock.h"
#include <uls/highlight.h>
#include <uls/system.h>

#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

std::string MODEL =  R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
	<template>
		<name x="5" y="5">Template</name>
		<declaration>typedef int y;typedef int x;</declaration>
		<location id="id0" x="-76" y="-68">
			<name x="-86" y="-102">Init</name>
		</location>
		<init ref="id0"/>
	</template>
	<system>
p = Template();
system p;
</system>
</nta>)";

using json = nlohmann::json;

TEST_CASE("typedef in template declaration"){
    auto repo = SystemRepository{};
    auto highlight = Highlight{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit")
        .add_module(repo)
        .add_module(highlight)
        .start();
    
    REQUIRE(mock.handshake());
    auto result = mock.receive();
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.output_empty());
    
    
    REQUIRE(result.size() == 7);

    CHECK(result.at(0)["id"] == "y");
    CHECK(result.at(0)["type"] == "KEYWORD3");
    CHECK(result.at(0)["start"] == 12);
    CHECK(result.at(0)["end"] == std::numeric_limits<int32_t>::max());

    CHECK(result.at(1)["id"] == "x");
    CHECK(result.at(1)["type"] == "KEYWORD3");
    CHECK(result.at(1)["start"] == 26);
    CHECK(result.at(1)["end"] == std::numeric_limits<int32_t>::max());
}