#include <uls/server.h>
#include <uls/autocomplete.h>
#include <iostream>

int main(){
    auto system_repo = SystemRepository{}; // Rename this system is technically wrong
    auto autocomplete_module = AutocompleteModule{system_repo};

    auto server = Server({std::cin, std::cout});
    server.add_close_command("exit")
        .add_module(system_repo)
        .add_module(autocomplete_module)
        .start();

    return 0;
}