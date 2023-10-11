#include "server_mock.h"
#include <uls/autocomplete.h>
#include <vector>
#include <ranges>
#include <iterator>

#include <iostream>
#include <stdexcept>
#include <concepts>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

namespace views = std::ranges::views;
using json = nlohmann::json;

std::string get_name(const json& item) { return item["name"]; }

bool is_unique(std::string_view item)
{
    static std::vector<std::string_view> filtered_names{
        "DBL_MAX",   "DBL_MIN",  "FLT_MAX",   "FLT_MIN",  "INT16_MAX",  "INT16_MIN", "INT32_MAX", "INT32_MIN",
        "INT8_MAX",  "INT8_MIN", "M_1_PI",    "M_2_PI",   "M_2_SQRTPI", "M_E",       "M_LN10",    "M_LN2",
        "M_LOG10E",  "M_LOG2E",  "M_PI",      "M_PI_2",   "M_PI_4",     "M_SQRT1_2", "M_SQRT2",   "UINT16_MAX",
        "UINT8_MAX", "bool",     "broadcast", "chan",     "clock",      "const",     "double",    "exists",
        "false",     "forall",   "int",       "int16_t",  "int32_t",    "int8_t",    "meta",      "return",
        "struct",    "true",     "typedef",   "uint16_t", "uint8_t",    "urgent",    "void", "abs","fabs",
        "fmod","fma","fmax","fmin","exp","exp2","expm1","ln","log","log10","log2","log1p","pow","sqrt",
        "cbrt","hypot","sin","cos","tan","asin","acos","atan","atan2","sinh","cosh","tanh","asinh","acosh",
        "atanh","erf","erfc","tgamma","lgamma","ceil","floor","trunc","round","fint","ldexp","ilogb","logb",
        "nextafter","copysign","signbit","random","random_normal","random_poisson","random_arcsine","random_beta",
        "random_gamma","tri","random_weibull"};
    return std::ranges::find(filtered_names, item) == filtered_names.end();
}

/** Transforms a json response into a json array of names */
json name_view(const auto& suggestions)
{
    json result;
    std::ranges::copy(suggestions | views::transform(get_name), std::back_inserter(result));
    return result;
}

/** Filters names to not include default names */
json unique_name_view(const auto& suggestions)
{
    json result;
    for (const json& suggestion : suggestions) {
        std::string name = suggestion["name"];
        if (is_unique(name))
            result.emplace_back(std::move(name));
    }
    return result;
}

const json& find(const json& array, std::string_view name)
{
    auto found = std::ranges::find_if(array, [&](const json& suggestion) { return suggestion["name"] == name; });
    if (found != array.end())
        return *found;

    throw std::logic_error{"Couldnt find element"};
}

const std::string MODEL = R"(<?xml version="1.0" encoding="utf-8"?>
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
    
int x = 20;
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
        <location id="id1" x="-86" y="-68">
		</location>
		<init ref="id0"/>
	</template>
	<system>
int z = 5;
p = Template();
system p;
</system>
</nta>)";

TEST_CASE("Autocomplete test default items")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("autocomplete", {{"xpath", "/nta/declaration!"}, {"identifier", "it"}, {"offset", 0}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    // All built-in typedefs are included, the intresting items are (item1, item2, item3)
    auto results = mock.receive();
    CHECK(name_view(results) ==
          json{"int","double","clock","chan","bool","broadcast","const","urgent","void","meta","true","false","forall",
          "exists","return","typedef","struct","abs","fabs","fmod","fma","fmax","fmin","exp","exp2","expm1","ln","log",
          "log10","log2","log1p","pow","sqrt","cbrt","hypot","sin","cos","tan","asin","acos","atan","atan2","sinh","cosh",
          "tanh","asinh","acosh","atanh","erf","erfc","tgamma","lgamma","ceil","floor","trunc","round","fint","ldexp",
          "ilogb","logb","nextafter","copysign","signbit","random","random_normal","random_poisson","random_arcsine",
          "random_beta","random_gamma","tri","random_weibull","INT8_MIN","INT8_MAX","UINT8_MAX","INT16_MIN","INT16_MAX",
          "UINT16_MAX","INT32_MIN","INT32_MAX","int8_t","uint8_t","int16_t","uint16_t","int32_t","FLT_MIN","FLT_MAX",
          "DBL_MIN","DBL_MAX","M_PI","M_PI_2","M_PI_4","M_E","M_LOG2E","M_LOG10E","M_LN2","M_LN10","M_1_PI","M_2_PI",
          "M_2_SQRTPI","M_SQRT2","M_SQRT1_2"});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Autocomplete struct type")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("autocomplete", {{"xpath", "/nta/declaration!"}, {"identifier", "p_a."}, {"offset", 77}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    auto results = mock.receive();
    CHECK(name_view(results) == json{"p_a.x", "p_a.y", "p_a.z"});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Autocomplete struct type with full member")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("autocomplete", {{"xpath", "/nta/declaration!"}, {"identifier", "p_a.z"}, {"offset", 77}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    auto results = mock.receive();
    CHECK(name_view(results) == json{"p_a.x", "p_a.y", "p_a.z"});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Autocomplete nested struct type")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("autocomplete", {{"xpath", "/nta/declaration!"}, {"identifier", "e.pos."}, {"offset", 147}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    auto results = mock.receive();
    CHECK(unique_name_view(results) == json{
                                           "e.pos.x",
                                           "e.pos.y",
                                           "e.pos.z",
                                       });
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Autocomplete template sub members")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};  // Use an empty set of builtin items

    auto mock = MockIO{};
    mock.send("upload", MODEL);
    mock.send("autocomplete", {{"xpath", "/nta/queries!"}, {"identifier", "p."}, {"offset", 2}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    auto results = mock.receive();
    CHECK(unique_name_view(results) == json{"p.y", "p.f", "p.Init"});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

const std::string MODEL2 = R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
    <declaration>
int numbers[20];
clock clocks[20];
chan channels[20];

    </declaration>
	<template>
		<name x="5" y="5">Template</name>
		<declaration></declaration>
		<location id="id0" x="-76" y="-68">
			<name x="-86" y="-102">Init</name>
		</location>
        <location id="id1" x="-86" y="-68">
		</location>
		<init ref="id0"/>
	</template>
	<system>
p = Template();
system p;
</system>
</nta>)";

TEST_CASE("Autocomplete template sub members")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};  // Use an empty set of builtin items

    auto mock = MockIO{};
    mock.send("upload", MODEL2);
    mock.send("autocomplete", {{"xpath", "/nta/queries!"}, {"identifier", "num"}, {"offset", 2}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    auto results = mock.receive();
    CHECK(find(results, "numbers")["type"] == "variable");
    CHECK(find(results, "clocks")["type"] == "variable");
    CHECK(find(results, "channels")["type"] == "channel");
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

const std::string MODEL3 = R"(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE nta PUBLIC '-//Uppaal Team//DTD Flat System 1.5//EN' 'http://www.it.uu.se/research/group/darts/uppaal/flat-1_5.dtd'>
<nta>
    <declaration>
typedef struct {
    int x;
    int y;
    int z;
} point;

point p_a;

void func(){
    int x
}

    </declaration>
	<template>
		<name x="5" y="5">Template</name>
        <parameter>int xax</parameter>
		<declaration></declaration>
		<location id="id0" x="-76" y="-68">
			<name x="-86" y="-102">Init</name>
		</location>
        <location id="id1" x="-86" y="-68">
		</location>
		<init ref="id0"/>
	</template>
	<system>
p = Template();
system p;
</system>
</nta>)";

TEST_CASE("Autocomplete template sub members")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};

    auto mock = MockIO{};
    mock.send("upload", MODEL3);
    mock.send("autocomplete", {{"xpath", "/nta/declaration!"}, {"identifier", "x"}, {"offset", 94}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(unique_name_view(mock.receive()) == json{"point", "p_a", "func"});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Autocomplete template parameter no variables")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};  // Use an empty set of builtin items

    auto mock = MockIO{};
    mock.send("upload", MODEL3);
    mock.send("autocomplete", {{"xpath", "/nta/template[1]/parameter!"}, {"identifier", "xax"}, {"offset", 7}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(unique_name_view(mock.receive()) == json{"point"});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Autocomplete template dont see locations")
{
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};  // Use an empty set of builtin items

    auto mock = MockIO{};
    mock.send("upload", MODEL3);
    mock.send("autocomplete", {{"xpath", "/nta/template[1]/declaration!"}, {"identifier", ""}, {"offset", 1}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(unique_name_view(mock.receive()) == json{"xax", "point", "p_a", "func"});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}

TEST_CASE("Autocomplete fails when identifier matches template name"){
    auto repo = SystemRepository{};
    auto autocomplete = AutocompleteModule{repo};  // Use an empty set of builtin items

    auto mock = MockIO{};
    mock.send("upload", MODEL3);
    mock.send("autocomplete", {{"xpath", "/nta/queries!"}, {"identifier", "Template."}, {"offset", 1}});
    mock.send_cmd("exit");

    auto server = Server{mock};
    server.add_close_command("exit").add_module(repo).add_module(autocomplete).start();

    REQUIRE(mock.handshake());
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK(unique_name_view(mock.receive()) == json{});
    REQUIRE(mock.receive() == OK_RESPONSE);
    CHECK_EOF(mock);
}