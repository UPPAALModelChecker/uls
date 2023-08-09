#include "server_mock.h"
#include <uls/renaming.h>

#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

std::string MODEL = R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
    <declaration>
const int x = 5;

const int z = x + 4 * x;

int add(int x){
    return x + 25;
}

int[z-5, z] zId;

typedef int[0,z] id;

    </declaration>
	<template>
		<name x="5" y="5">Template</name>
		<declaration>int k = x;</declaration>
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

TEST_CASE("Find usages of x declaraed in global declarations")
{
    auto repo = SystemRepository{};
    auto renaming = RenamingModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("find_usages", {{"identifier", "x"}, {"offset", 11}, {"xpath", "/nta/declaration!"}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(renaming).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == json{{{"start", 11}, {"end", 12}, {"xpath", "/nta/declaration"}},
                                 {{"start", 33}, {"end", 34}, {"xpath", "/nta/declaration"}},
                                 {{"start", 41}, {"end", 42}, {"xpath", "/nta/declaration"}},
                                 {{"start", 8}, {"end", 9}, {"xpath", "/nta/template[1]/declaration"}}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Find usages of x from template")
{
    auto repo = SystemRepository{};
    auto renaming = RenamingModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("find_usages", {{"identifier", "x"}, {"offset", 33}, {"xpath", "/nta/template[1]/declaration"}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(renaming).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == json{{{"start", 11}, {"end", 12}, {"xpath", "/nta/declaration"}},
                                 {{"start", 33}, {"end", 34}, {"xpath", "/nta/declaration"}},
                                 {{"start", 41}, {"end", 42}, {"xpath", "/nta/declaration"}},
                                 {{"start", 8}, {"end", 9}, {"xpath", "/nta/template[1]/declaration"}}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Find usages of x inside function")
{
    auto repo = SystemRepository{};
    auto renaming = RenamingModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("find_usages", {{"identifier", "x"}, {"offset", 72}, {"xpath", "/nta/declaration!"}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(renaming).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == json{{{"start", 57}, {"end", 58}, {"xpath", "/nta/declaration"}},
                                 {{"start", 72}, {"end", 73}, {"xpath", "/nta/declaration"}}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Find usages inside types")
{
    auto repo = SystemRepository{};
    auto renaming = RenamingModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("find_usages", {{"identifier", "z"}, {"offset", 29}, {"xpath", "/nta/declaration!"}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(renaming).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == json{{{"start", 29}, {"end", 30}, {"xpath", "/nta/declaration"}},
                                 {{"start", 87}, {"end", 88}, {"xpath", "/nta/declaration"}},
                                 {{"start", 92}, {"end", 93}, {"xpath", "/nta/declaration"}},
                                 {{"start", 115}, {"end", 116}, {"xpath", "/nta/declaration"}}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

std::string MODEL2 = R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
    <declaration>
const int x = 5;

typedef int[0, x] id_t;

id_t y;
id_t z;
int j[id_t];

    </declaration>
	<template>
		<name x="5" y="5">Template</name>
		<location id="id0" x="-76" y="-68">
			<name x="-86" y="-102">Init</name>
		</location>
		<init ref="id0"/>
	</template>
	<system>
p = Template();

id_t k = y;

system p;
</system>
</nta>)";

TEST_CASE("Type visited multiple times issue")
{
    auto repo = SystemRepository{};
    auto renaming = RenamingModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL2);
    mock.send("find_usages", {{"identifier", "x"}, {"offset", 11}, {"xpath", "/nta/declaration!"}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(renaming).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == json{{{"start", 11}, {"end", 12}, {"xpath", "/nta/declaration"}},
                                 {{"start", 34}, {"end", 35}, {"xpath", "/nta/declaration"}}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Find variable in system declarations")
{
    auto repo = SystemRepository{};
    auto renaming = RenamingModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL2);
    mock.send("find_usages", {{"identifier", "y"}, {"offset", 49}, {"xpath", "/nta/declaration!"}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(renaming).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.receive() == json{{{"start", 49}, {"end", 50}, {"xpath", "/nta/declaration"}},
                                 {{"start", 27}, {"end", 28}, {"xpath", "/nta/system"}}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Find template name")
{
    auto repo = SystemRepository{};
    auto renaming = RenamingModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL2);
    mock.send("find_usages", {{"identifier", "Template"}, {"offset", 7}, {"xpath", "/nta/system!"}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(renaming).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(mock.expect_error());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}
