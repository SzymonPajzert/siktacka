#ifndef SIKTACKA_TYPES_HPP
#define SIKTACKA_TYPES_HPP

#include <cstdint>
#include <ctime>
#include <bits/unique_ptr.h>
#include <memory>
#include <utility>
#include <cstring>
#include <iostream>

#define WARN_UNUSED __attribute__((warn_unused_result))
#define NO_RETURN __attribute__((noreturn))

enum dir_t {
    left = -1,
    forward = 0,
    right = 1,
};

using turn_dir_t = int8_t;
using event_no_t = uint32_t;

using dim_t = int32_t;
using port_t = uint16_t;
using speed_t = int32_t;
using seed_t = time_t;
using game_id_t = uint32_t;
using timeout_t = int;

using sock_t = int;

using len_t = uint32_t;
using crc_t = uint32_t;

using session_t = uint64_t;

template<typename T>
using maybe = std::shared_ptr<T>;

struct binary_reader_t;

using byte_t = unsigned char;

using event_type_t = int8_t;
static constexpr event_type_t NEW_GAME = 0;
static constexpr event_type_t PIXEL = 1;
static constexpr event_type_t PLAYER_ELIMINATED = 2;
static constexpr event_type_t GAME_OVER = 3;

struct config {
    static constexpr size_t UDP_SIZE = 512;
    static constexpr size_t EVENT_SIZE = UDP_SIZE - sizeof(game_id_t);
    static constexpr size_t EVENT_SPECIFIC_SIZE = EVENT_SIZE - sizeof(len_t) - sizeof(crc_t);

    static constexpr int CLIENT_UPDATE_TIMEOUT = 20;

    static constexpr timeout_t INACTIVITY_KICK = 120000;
};

#endif //SIKTACKA_TYPES_HPP
