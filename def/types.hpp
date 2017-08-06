#ifndef SIKTACKA_TYPES_HPP
#define SIKTACKA_TYPES_HPP

#include <cstdint>
#include <ctime>
#include <bits/unique_ptr.h>

using dim_t = int32_t;
using port_t = int32_t;
using speed_t = int32_t;
using seed_t = time_t;

template<typename T>
using maybe = std::unique_ptr<T>;

// TODO move to utils
template<typename T>
bool maybe_assign(maybe<T> value, T & target) {
    bool result = false;

    if(value != nullptr) {
        target = *value;
        result = true;
    }

    return result;
}

#endif //SIKTACKA_TYPES_HPP
