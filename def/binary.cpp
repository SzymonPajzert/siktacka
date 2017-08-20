//
// Created by svp on 16.08.17.
//

#include "binary.hpp"


std::shared_ptr<binary_reader_t> binary_t::reader() {
    return std::make_shared<binary_reader_t>(binary_reader_t(*this));
}

bool binary_writer_t::write_bytes(binary_t data) {
    bool result = size_left() >= data.length;

    if (result) {
        memcpy(get(), data.bytes.get(), data.length);
    }

    return result;
}

binary_t binary_writer_t::save() const {
    logs(binary, 4) << "Saving ptr of size: " << pointer << std::endl;

    // TODO
    // std::shared_ptr<char> ideal_capacity { ((char*) malloc(pointer)) };
    // memcpy(ideal_capacity.get(), bytes.get(), pointer);
    return binary_t { bytes , pointer };
}