#include "server_mock.h"
#include <uls/declarations.h>
#include <uls/system.h>

#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

const std::string MODEL = R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
    <declaration>
    int x = 5;
    
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

TEST_CASE("goto x declaration")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("goto_decl", {{"xpath", "/nta/template[1]"}, {"identifier", "x"}, {"offset", 0}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    auto response = mock.receive();
    CHECK(response["xpath"] == "/nta/declaration");
    CHECK(response["start"] == 9);
    CHECK(response["end"] == 10);
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("goto y declaration")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("goto_decl", {{"xpath", "/nta/template[1]"}, {"identifier", "y"}, {"offset", 0}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    auto response = mock.receive();
    CHECK(response["xpath"] == "/nta/template[1]/declaration");
    CHECK(response["start"] == 4);
    CHECK(response["end"] == 5);
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("goto z declaration from template")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("goto_decl", {{"xpath", "/nta/template[1]"}, {"identifier", "z"}, {"offset", 0}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == FAIL_RESPONSE);
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("goto z declaration from system")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("goto_decl", {{"xpath", "/nta/system!"}, {"identifier", "z"}, {"offset", 0}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    auto response = mock.receive();
    CHECK(response["xpath"] == "/nta/system");
    CHECK(response["start"] == 5);
    CHECK(response["end"] == 6);
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Goto non existing declaration")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("goto_decl", {{"xpath", "/nta/template[1]"}, {"identifier", "test"}, {"offset", 0}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == FAIL_RESPONSE);
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Goto template")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("goto_decl", {{"xpath", "/nta/system!"}, {"identifier", "Template"}, {"offset", 0}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == json{{"xpath", "/nta/template[1]"}, {"start", 0}, {"end", 1}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Goto template variable")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("goto_decl", {{"xpath", "/nta/system!"}, {"identifier", "p"}, {"offset", 0}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == json{{"xpath", "/nta/system"}, {"start", 12}, {"end", 27}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Goto function scopes")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("goto_decl", {{"xpath", "/nta/template[1]"}, {"identifier", "x"}, {"offset", 62}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == json{{"xpath", "/nta/template[1]/declaration"}, {"start", 55}, {"end", 56}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

const std::string MODEL2 = R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
    <declaration>
typedef struct{
    int x_loc;
    int y_loc;
} point;

point p = { 3, 5 };
p.x_loc = 5;
    </declaration>
	<template>
		<name x="5" y="5">Template</name>
		<declaration></declaration>
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

TEST_CASE("Goto struct")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL2);
    mock.send("goto_decl", {{"xpath", "/nta/declaration!"}, {"identifier", "p"}, {"offset", 66}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == json{{"xpath", "/nta/declaration"}, {"start", 63}, {"end", 64}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Goto struct members")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL2);
    mock.send("goto_decl", {{"xpath", "/nta/declaration!"}, {"identifier", "p.x_loc"}, {"offset", 66}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == json{{"xpath", "/nta/declaration"}, {"start", 25}, {"end", 30}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

const std::string MODEL3 = R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
    <declaration>
typedef struct{
    int x_loc;
    int y_loc;
} point;

const point p = { 3, 5 };
p.x_loc = 5;

void func(){
    
}
    </declaration>
	<template>
		<name x="5" y="5">Template</name>
		<declaration>clock someClock;</declaration>
		<location id="id0" x="-76" y="-68">
			<name x="-86" y="-102">Init</name>
		</location>
		<init ref="id0"/>
	</template>
	<system>
Process = Template();
system Process;
</system>
</nta>)";

TEST_CASE("Goto const struct members")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL3);
    mock.send("goto_decl", {{"xpath", "/nta/declaration!"}, {"identifier", "p.x_loc"}, {"offset", 66}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == json{{"xpath", "/nta/declaration"}, {"start", 25}, {"end", 30}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Verifier template identifier")
{
    auto repo = SystemRepository{};
    auto decls = DeclarationsModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL3);
    mock.send("goto_decl", {{"xpath", "/nta/"}, {"identifier", "Process.someClock"}, {"offset", 13}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(decls).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    REQUIRE(mock.receive() == json{{"xpath", "/nta/template[1]/declaration"}, {"start", 6}, {"end", 15}});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}