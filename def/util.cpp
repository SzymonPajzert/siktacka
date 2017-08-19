//
// Created by svp on 12.08.17.
//

#include "util.hpp"

std::ostream null {0};

std::ostream & logs(part_t part, int level) {
    if (level <= max_log_level.at(part)) {
        for (int i = 0; i < level; i++) {
            std::cerr << " ";
        }
        return std::cerr;
    } else {
        return null;
    }
}



