#include <iostream>
#include "parse/parser.hpp"
#include "GameServer.hpp"

void print_params(server_param params) {
    std::cout << params.width << std::endl;
    std::cout << params.height << std::endl;
    std::cout << params.port << std::endl;
    std::cout << params.rounds_sec << std::endl;
    std::cout << params.turning << std::endl;
    std::cout << params.seed << std::endl;
}

int main(int argc, char **argv) {
    auto params = parse_server(argc, argv);
    // print_params(params);

    if(params == nullptr) {
        failure("Parsing failed");
    }

    logs_0 << "Main run" << std::endl;
    auto server = GameServer(*params);
    logs_0 << "Initialization finished" << std::endl;
    server.run();

    return 0;
}