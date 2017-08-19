//
// Created by svp on 07.08.17.
//


#include "ip_parsing.hpp"
#include "serialization.hpp"

std::map<part_t, int> max_log_level {{comm, 1}, {serv, 1}, {addr, 1}, {binary,1}};

int main() {
    auto tests = {
            test_ip_parsing().print_results("IP parsing"),
            test_serialization().print_results("Serialization")
    };

    auto overall = Test::merge_results(tests);

    return (overall.print_results("Overall").ok() ? 0 : 1);
}