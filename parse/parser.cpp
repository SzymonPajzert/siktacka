//
// Created by svp on 07.08.17.
//


#include "parser.hpp"

const std::string client_param::default_gui_address { "localhost" };

void parse_error(const std::string & message) {
    std::cout << message << std::endl;
    exit(1);
}

template<typename T>
void try_parse(T & target, const std::string &var_name) {
    if(!maybe_assign<T>(parse_int<T>(optarg), target)) {
        std::string message = "Wrong argument to ";
        message.append(var_name);
        parse_error(message);
    }
}


maybe<server_param> parse_server(const int argc, char **argv) {
    int processed_args = 0;

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
                    " requires an argument";
            parse_error(message.str());
            break;
        } else {
            // We count the number of processed arguments
            processed_args += 2;
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

    if (argc == 1 + processed_args) {
        return std::make_shared<server_param>(server_param {width, height, port, speed, turn, seed});
    } else {
        std::cerr << "Wrong number of arguments argc:" <<
                  argc << " processed_args: " << processed_args << std::endl;
        parse_error("Parse failure");
    }

    return nullptr;
}

maybe<client_param> parse_client(const int argc, char **argv) {
    if(argc < 3 || argc > 5)  parse_error("Wrong number of arguments");

    std::string player_name { argv[1] };
    if(!client_param::correct_player_name(player_name)) {
        parse_error("Wrong player name");
    }

    maybe<IP> maybe_server_address;
    maybe<IP> maybe_gui_address;

    maybe_server_address = or_maybe(
            IP::parse(UDP, argv[2]),
            IP::parse(UDP, argv[2], client_param::default_server_port));

    if(argc == 4) {
        maybe_gui_address = or_maybe(
                IP::parse(TCP, argv[3]),
                IP::parse(TCP, argv[3], client_param::default_gui_port));
    } else {
        maybe_gui_address = IP::parse(TCP,
                client_param::default_gui_address,
                client_param::default_gui_port);
    }

    if (maybe_server_address and maybe_gui_address) {
        return std::make_shared<client_param>(
                client_param { player_name, *maybe_server_address.get(), *maybe_gui_address.get() });
    }

    if (not maybe_server_address) {
        parse_error("Wrong server address");
    }

    if (not maybe_gui_address) {
        parse_error("Wrong gui address");
    }

    return nullptr;
}