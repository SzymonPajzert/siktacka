#ifndef SIKTACKA_PARSER_HPP
#define SIKTACKA_PARSER_HPP

#include <getopt.h>
#include <memory>
#include "def/types.hpp"
#include "def/ipaddr.hpp"
#include <string>
#include <sstream>
#include <utility>
#include <iostream>


struct server_param {
    dim_t width, height;
    port_t port;
    speed_t rounds_sec;
    speed_t turning;
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

struct client_param {
    std::string player_name;
    IP server_address;
    IP gui_address;

    static const port_t default_server_port = 12345;
    static const std::string default_gui_address;
    static const port_t default_gui_port = 12346;

    client_param(std::string _player_name, IP _server_address, IP _gui_address)
            : player_name(std::move(_player_name))
            , server_address(std::move(_server_address))
            , gui_address(std::move(_gui_address)) {}

    static WARN_UNUSED bool correct_player_name(const std::string &player_name) {
        bool result = true;

        result = result && player_name.length() <= client_param::max_player_name_len;
        for(auto c : player_name) {
            result =
                    result &&
                    c >= client_param::min_allowed_player_name_char &&
                    c <= client_param::max_allowed_player_name_char;
        }

        return result;
    }

private:
    static constexpr int max_player_name_len = 64;
    static constexpr int min_allowed_player_name_char = 33;
    static constexpr int max_allowed_player_name_char = 126;
};

[[ noreturn ]] void parse_error(const std::string & message);

template<typename T>
void try_parse(T & target, const std::string &var_name);

maybe<server_param> parse_server(int argc, char **argv);

maybe<client_param> parse_client(int argc, char **argv);

#endif //SIKTACKA_PARSER_HPP
