#include <iostream>
#include "parse/parser.hpp"
#include "GameServer.hpp"

std::map<part_t, int> max_log_level {{comm, 0}, {serv, 10}, {addr, 10}, {binary,10}};

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

    logs(serv, 0) << "Main run" << std::endl;
    auto server = GameServer(*params);
    logs(serv, 0) << "Initialization finished" << std::endl;
    server.run();

    return 0;
}