#include <iostream>
#include <def/binary.hpp>

std::map<part_t, int> max_log_level {{comm, 1}, {serv, 1}, {addr, 1}, {binary,1}};

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}