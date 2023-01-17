#include <uls/server.h>
#include <uls/highlight.h>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(){
    auto system_repo = SystemRepository{}; // Rename this system is technically wrong
    auto highlight_module = Highlight{system_repo};

    auto server = Server({std::cin, std::cout});
    server.add_close_command("exit")
        .add_module(system_repo)
        .add_module(highlight_module)
        .start();



    return 0;
}