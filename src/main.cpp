#include <iostream>

#include "command.hpp"

int main(int argc, char* argv[]) {
    try {
        ingress_drone_explorer::command::run(argc, argv);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❓ Unknown error occured." << std::endl;
        return 1;
    }
    return 0;
}