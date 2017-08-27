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

struct binary_t {
    std::shared_ptr<byte_t> bytes;
    const size_t length;

    bool operator==(const binary_t& that) {
        return this->length == that.length and (memcmp(this->bytes.get(), that.bytes.get(), this->length) == 0);
    }
};

enum data_source_t {
    net, host
};

struct binary_reader_t {
    binary_t data;
    size_t counter;
    data_source_t data_source;

    explicit binary_reader_t(
            binary_t _data,
            data_source_t _data_source,
            bool fail=true)
            : data(std::move(_data))
            , counter(0)
            , data_source(_data_source) {

        // TODO(maybe) implement behaviour fail prone and set to true by default
        (void) fail;
    };

    template <typename T>
    maybe<T> read() {
        constexpr auto size = sizeof(T);
        if(counter + size <= data.length) {
            T value;
            byte_t* source = data.bytes.get() + counter;
            memcpy(&value, source, size);
            counter += size;

            switch (data_source) {
                case net : value = net_to_host<T>(value); break;
                case host: value = host_to_net<T>(value); break;
                default: break;
            }

            return std::make_shared<T>(value);
        } else {
            logs(binary, 2) << "Tried to read too much data" << std::endl;
            return nullptr;
        }
    }

    maybe<std::string> read_string() {
        maybe<byte_t> may_c = nothing<byte_t>;
        std::string result_string {};
        bool read_failed = false;

        // End if message finished or we fail
        while(unread_size() > 0 and // try more only if there's something to read
                // If we fail reading, set read_failed to true
                ((((may_c = read<byte_t>()) != nullptr) or ((read_failed=true), false)) and *may_c.get() != 0)) {
            result_string.push_back(*may_c);
        }

        maybe<std::string> result = nullptr;
        // reading not failed, we can return result
        if(!read_failed) {
            result = std::make_shared<std::string>(std::string(result_string));
        }

        return result;
    }

    size_t unread_size() const {
        return data.length - counter;
    }
};



struct binary_writer_t {
    explicit binary_writer_t(size_t _max_size)
            : max_size(_max_size)
            , pointer(0)
            , bytes((byte_t*) malloc(max_size))
            , ok(true)
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

    template<typename T>
    void write(T value) {
        constexpr auto size = sizeof(T);
        bool can_write = size_left() >= size;

        if(can_write) {
            memcpy(this->get(), &value, size);
            pointer += size;
        }

        ok = ok and can_write;
    }

    void write(const std::string & string, bool append_zero=true) {
        for(auto c : string) {
            write<char>(c);
            if(!is_ok()) break;
        }

        if(append_zero) {
            write<char>('\0');
        }
    }

    /** Writes binary data to the buffer
     *
     * @param data Data to be written
     * @return True if data fit into the memory
     */
    void write_bytes(binary_t data);

    bool is_ok() const {
        return ok;
    }

    size_t get_pointer() const {
        return pointer;
    };

private:
    size_t max_size;
    size_t pointer;
    std::shared_ptr<byte_t> bytes;
    bool ok;
};

template <typename T>
binary_t serialize(const T & value);

#endif //SIKTACKA_BINARY_HPP
