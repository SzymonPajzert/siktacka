//
// Created by svp on 16.08.17.
//

#ifndef SIKTACKA_BINARY_HPP
#define SIKTACKA_BINARY_HPP

#include <cstdint>
#include <ctime>
#include <bits/unique_ptr.h>
#include <memory>
#include <utility>
#include <cstring>
#include <iostream>

#include "util.hpp"

// todo move to binary
struct binary_t {
    std::shared_ptr<byte_t> bytes;
    const size_t length;

    std::shared_ptr<binary_reader_t> reader();

    bool operator==(const binary_t& that) {
        return this->length == that.length and (memcmp(this->bytes.get(), that.bytes.get(), this->length) == 0);
    }
};

struct binary_reader_t {
    binary_t data;
    size_t counter;

    explicit binary_reader_t(binary_t _data) : data(std::move(_data)), counter(0) {};

    template <typename T>
    maybe<T> read() {
        constexpr auto size = sizeof(T);
        if(counter + size <= data.length) {
            T value;
            char * source = data.bytes.get() + counter;
            memcpy(&value, source, size);
            counter += size;

            value = net_to_host<T>(value);

            return std::make_shared<T>(value);
        } else {
            // TODO allow logging here
            std::cerr << "Tried to read too much data" << std::endl;
            return nullptr;
        }
    }

    maybe<std::string> read_string() {
        maybe<char> may_c;
        std::string result_string {};

        while((may_c = this->read<char>()) != nullptr && *may_c.get() != 0) {
            result_string.push_back(*may_c);
        }

        maybe<std::string> result = nullptr;
        // reading not failed, we can return result
        if(may_c != nullptr) {
            result = std::make_shared<std::string>(std::string(result_string));
        }

        return result;
    }
};



struct binary_writer_t {
    explicit binary_writer_t(size_t _max_size)
            : max_size(_max_size)
            , pointer(0)
            , bytes((char*) malloc(max_size))
    {}

    size_t size_left() {
        return max_size - pointer;
    }

    void* get() {
        return bytes.get() + pointer;
    }

    bool move(size_t size) {
        return (pointer += size) <= max_size;
    }

    binary_t save() const;

    // TODO store result in internal state, assume that writing never fails
    template<typename T>
    bool WARN_UNUSED write(T value) {
        constexpr auto size = sizeof(T);
        bool can_write = size_left() >= size;

        if(can_write) {
            memcpy(this->get(), &value, size);
            pointer += size;
        }

        return can_write;
    }

    bool WARN_UNUSED write(const std::string & string) {
        bool result = true;
        for(auto c : string) {
            result = write<char>(c);
            if(!result) break;
        }
        // Check how strings should end
        result = result && write<char>('\0');

        return result;
    }

    /** Writes binary data to the buffer
     *
     * @param data Data to be written
     * @return True if data fit into the memory
     */
    bool WARN_UNUSED write_bytes(binary_t data);

    size_t max_size;
    size_t pointer;
    std::shared_ptr<byte_t> bytes;
};

template <typename T>
binary_t serialize(const T & value);

#endif //SIKTACKA_BINARY_HPP
