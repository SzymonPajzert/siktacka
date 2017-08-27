//
// Created by svp on 12.08.17.
//

#include "util.hpp"

std::ostream null {0};

std::ostream & logs(part_t part, int level, bool indent) {
    auto max_level = get_or(max_log_level, part, -1);
    if (level <= max_level) {
        if(indent) {
            for (int i = 0; i < level; i++) {
                std::cerr << " ";
            }
        }
        return std::cerr;
    } else {
        return null;
    }
}



