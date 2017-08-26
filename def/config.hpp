//
// Created by svp on 20.08.17.
//

#ifndef SIKTACKA_CONFIG_HPP
#define SIKTACKA_CONFIG_HPP

#include <cstdio>

struct config {
    // TODO remove it and add smaller versions per usage (etc. 508 for event_data)
    static constexpr size_t BUFFER_SIZE = 512;
};

#endif //SIKTACKA_CONFIG_HPP
