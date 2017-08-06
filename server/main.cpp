#include <iostream>
#include "parse/parser.hpp"

int main(int argc, char **argv) {
    auto params = parse_server(argc, argv);
    std::cout << params.width << std::endl;
    std::cout << params.height << std::endl;
    std::cout << params.port << std::endl;
    std::cout << params.speed << std::endl;
    std::cout << params.turn << std::endl;
    std::cout << params.seed << std::endl;
    return 0;
}