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

using dim_t = int32_t;
using port_t = uint16_t;
using speed_t = int32_t;
using seed_t = time_t;
using game_id_t = uint32_t;
using timeout_t = int;

using sock_t = int;

template<typename T>
using maybe = std::shared_ptr<T>;

struct binary_reader_t;

using byte_t = char;


#endif //SIKTACKA_TYPES_HPP
