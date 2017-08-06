#ifndef SIKTACKA_PARSER_HPP
#define SIKTACKA_PARSER_HPP

#include <getopt.h>
#include <memory>
#include "def/types.hpp"
#include <string>
#include <sstream>


struct server_param {
    dim_t width, height;
    port_t port;
    speed_t speed;
    speed_t turn;
    seed_t seed;

    static const dim_t DEFAULT_WIDTH = 800;
    static const dim_t DEFAULT_HEIGHT = 600;
    static const port_t DEFAULT_PORT = 12345;
    static const speed_t DEFAULT_SPEED = 50;
    static const speed_t DEFAULT_TURN = 6;

    static seed_t DEFAULT_SEED() {
        return time(nullptr);
    }
};

void parse_error(const std::string & message) {
    // TODO
}

template<typename T>
maybe<T> parse_int(const std::string & int_string) {
    bool all_digit = true;
    for(char digit : int_string) {
        all_digit = all_digit && (isdigit(digit) != 0);
    }

    if(all_digit) {
        return std::make_unique<T>(std::stoi(int_string));
    } else {
        return nullptr;
    }
}

// TODO move to unnamed namespace
template<typename T>
void try_parse(T & target, std::string var_name) {
    if(!maybe_assign<T>(parse_int<T>(optarg), target)) {
        std::string message = "Wrong argument to ";
        message.append(var_name);
        parse_error(message);
    }
}

server_param parse_server(int argc, char **argv) {
    dim_t width = server_param::DEFAULT_WIDTH;
    dim_t height = server_param::DEFAULT_HEIGHT;
    port_t port = server_param::DEFAULT_PORT;
    speed_t speed = server_param::DEFAULT_SPEED;
    speed_t turn = server_param::DEFAULT_TURN;
    seed_t seed = server_param::DEFAULT_SEED();

    int c;
    while ((c = getopt (argc, argv, "W:H:p:s:t:r:")) != -1) {
        if (c == '?') {
            std::stringstream message;
            message <<
                    "Option: " <<
                    (char) optopt <<
                    "requires an argument";
            parse_error(message.str());
            break;
        }

        switch (c) {
            case 'W':try_parse(width, "width");break;
            case 'H':try_parse(height, "height");break;
            case 'p':try_parse(port, "port"); break;
            case 's':try_parse(speed, "speed");break;
            case 't':try_parse(turn, "turn");break;
            case 'r':try_parse(seed, "seed");break;
            default:
                parse_error("I don't event know what's going on");
        }
    }

    return server_param { width, height, port, speed, turn, seed };
}




#endif //SIKTACKA_PARSER_HPP
