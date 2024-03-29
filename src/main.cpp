#include <iostream>

#if defined(_WIN32)
#   include <Windows.h>
#endif

#include "command.hpp"

int main(int argc, char* argv[]) {

#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
#endif

    try {
        ingress_drone_explorer::command::execute(argc, argv);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❓ Unknown error occured." << std::endl;
        return 1;
    }
    return 0;
}