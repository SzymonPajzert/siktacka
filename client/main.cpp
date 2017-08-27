#include <iostream>
#include <def/binary.hpp>
#include <parse/parser.hpp>
#include "GameClient.hpp"


std::map<part_t, int> max_log_level {
    {comm, 3}, {client, 3}, {addr, 10}
};

int main(int argc, char **argv) {
    auto params = parse_client(argc, argv);

    if(params == nullptr) {
        failure("Parsing failed");
    }

    logs(client, 0) << "Main run" << std::endl;
    GameClient instance {*params};
    logs(client, 0) << "Initialization finished" << std::endl;

    instance.run();

    return 0;
}