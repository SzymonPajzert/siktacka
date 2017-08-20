//
// Created by svp on 07.08.17.
//

#ifndef SIKTACKA_UTIL_HPP
#define SIKTACKA_UTIL_HPP

#include <iostream>
#include <endian.h>
#include <map>
#include "types.hpp"


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

/** Merge values of maybe according to the rules of alternative typeclass
 *
 * @tparam T type of the maybe function
 * @param first first argument of or
 * @param second second argument of or
 * @return first argument that exists
 */
template <typename T>
maybe<T> or_maybe(maybe<T> first, maybe<T> second) {
    if(first) return first;
    else {
        if(second) return second;
        else return nullptr;
    }
}

template<typename T>
maybe<T> parse_int(const std::string & int_string) {
    bool all_digit = true;
    for(char digit : int_string) {
        all_digit = all_digit && (isdigit(digit) != 0);
    }

    if(all_digit) {
        return std::make_shared<T>(std::stoi(int_string));
    } else {
        return nullptr;
    }
}

constexpr void * unit = nullptr;
template <typename T>
constexpr maybe<T> nothing = nullptr;

template<typename T>
T id(T x) {
    return x;
}

template <typename T, typename A, typename Func>
A get_maybe(maybe<T> value, A def_value, Func fun = id) {
    if(value) {
        return fun(*value.get());
    } else {
        return def_value;
    }
}

/** Perform map on the maybe value
 *
 * @tparam T
 * @param value
 * @param fun
 */
template <typename T, typename A, typename Func>
maybe<A> map_maybe(maybe<T> value, Func fun) {
    if (value) {
        return std::make_shared<A>(fun(*value.get()));
    } else {
        return nullptr;
    }
}

/** Perform map on the maybe value but without returning value
 *
 * @tparam T
 * @param value
 * @param fun
 */
template <typename T, typename Func>
void map_maybe_(maybe<T> value, Func fun) {
    auto lambda = [fun](T extracted) -> void * {
        fun(extracted);
        return unit;
    };

    (void) get_maybe(value, unit, lambda);
}

/** Perform map on the maybe value
 *
 * @tparam T
 * @param value
 * @param fun
 */
template <typename T, typename A, typename Func>
maybe<A> bind_maybe(maybe<T> value, Func fun) {
    if(value) {
        return fun(*value.get());
    } else {
        return nullptr;
    }
}

template<typename T>
T host_to_net(T value) {
    constexpr size_t size = sizeof(T);
    switch(size){
        case 1: return value;
        case 2: return static_cast<T>(htobe16(value));
        case 4: return static_cast<T>(htobe32(value));
        case 8: return htobe64(value);
        default: throw std::logic_error("Wrong size in host_to_net");
    }
}

template<typename T>
T net_to_host(T value) {
    constexpr size_t size = sizeof(T);
    switch(size){
        case 1: return value;
        case 2: return static_cast<T>(be16toh(value));
        case 4: return static_cast<T>(be32toh(value));
        case 8: return static_cast<T>(be64toh(value));
        default: throw std::logic_error("Wrong size in host_to_net");
    }
}

struct RandomSource {
    using value_t = int64_t;

    explicit RandomSource(value_t seed) : value(seed) {}

    value_t get_next() {
        value_t result = value;
        value = (value * 279470273) % 4294967291;
        return result;
    }

private:
    value_t value;

};

extern std::ostream null;

enum part_t {
    comm,
    serv,
    addr,
    binary,
};

extern std::map<part_t, int> max_log_level;

std::ostream & logs(part_t part, int level);

inline void syserr(const std::string & message) {
    // TODO add errno interpretation
    std::cerr << message << std::endl;
    std::cerr << errno << ": " << strerror(errno) << std::endl;
    exit(1);
}

inline void failure(const std::string & message) {
    std::cerr << message << std::endl;
    exit(1);
}

inline void print_bytes(const void *object, size_t size) {
    auto * const bytes = static_cast<const unsigned char *>(object);
    size_t i;

    printf("[ ");
    for(i = 0; i < size; i++)
    {
        printf("%02x ", bytes[i]);
    }
    printf("]\n");
}



#endif //SIKTACKA_UTIL_HPP
