//
// Created by svp on 07.08.17.
//

#include <vector>
#include <algorithm>
#include <iostream>

#include "def/ipaddr.hpp"

std::string remove_port(const std::string & ip_string) {
    auto delim_pos = ip_string.find_last_of(':');
    auto address_string = ip_string.substr(0, delim_pos);
    return address_string;
}

namespace {
    int test_run = 0;
    int test_correct = 0;
}

/** Test value and print the description as well as increase internal counters
 *
 * @param value
 */
void test(bool value) {
    test_run++;
    test_correct += int(value);
    std::cout << (value ? "OK" : "WRONG") << std::endl;
}

// TODO consider moving it to the class
/** Print results of run tests
 *
 */
bool print_results() {
    std::cout << std::endl << std::endl << "Results: " << test_correct << "/" << test_run << std::endl;
    return test_correct == test_run;
}

bool test_ip_parsing() {
    using addresses = std::vector<std::string>;

    addresses parseable = {
            "localhost:5000",
            "1.2.3.4:123"
    };

    port_t default_port = 1000;
    addresses parseable_without_port = {
            "google.com",
            "fff::"
    };

    std::cout << "Size of the parseable with port: " << parseable.size() << std::endl;
    std::cout << "Size of the parseable without port: " << parseable_without_port.size() << std::endl;
    std::cout << "Removing ports from parseable and adding to those without port" << std::endl;

    auto to_append_size = parseable_without_port.size();

    auto new_size = parseable.size() + parseable_without_port.size();
    parseable_without_port.resize(new_size);

    auto to_append = parseable_without_port.begin() + to_append_size;
    std::transform(parseable.begin(), parseable.end(), to_append, remove_port);

    std::cout << "Size of the parseable without port: " << parseable_without_port.size() << std::endl;

    // TODO move to separate function
    std::cout << std::endl << "Checking parseable with port" << std::endl;
    for(const auto &address : parseable) {
        auto result = parse_address(address);
        std::cout << "Parsing: " << address << " ";
        test(result != nullptr);
    }

    std::cout << std::endl << "Checking parseable without port" << std::endl;
    for(const auto &address : parseable) {
        auto result = IP::parse(address, default_port);
        std::cout << "Parsing: " << address << " ";
        test(result != nullptr);
    }

    return test_run == test_correct;
}



int main() {
    test_ip_parsing();

    IP::parse("hdfahadshf", 500);

    return (print_results() ? 0 : 1);
}