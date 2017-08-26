//
// Created by svp on 16.08.17.
//

#include "binary.hpp"

void binary_writer_t::write_bytes(binary_t data) {
    ok = size_left() >= data.length;

    if (ok) {
        memcpy(get(), data.bytes.get(), data.length);
    }
}

binary_t binary_writer_t::save() const {
    logs(binary, 4) << "Saving ptr of size: " << pointer << std::endl;

    // TODO
    // std::shared_ptr<char> ideal_capacity { ((char*) malloc(pointer)) };
    // memcpy(ideal_capacity.get(), bytes.get(), pointer);
    return binary_t { bytes , pointer };
}