//
// Created by svp on 17.08.17.
//

#ifndef SIKTACKA_IP_PARSING_HPP
#define SIKTACKA_IP_PARSING_HPP


#include <vector>
#include <algorithm>
#include <iostream>

#include "def/ipaddr.hpp"
#include "Test.hpp"

std::string remove_port(const std::string & ip_string) {
    auto delim_pos = ip_string.find_last_of(':');
    auto address_string = ip_string.substr(0, delim_pos);
    return address_string;
}

Test test_ip_parsing() {
    Test instance;

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
        std::cout << "Parsing: " << address << " " << std::flush;
        auto result = parse_address(address);
        std::cout << instance.test(result != nullptr) << std::endl;
    }

    std::cout << std::endl << "Checking parseable without port" << std::endl;
    for(const auto &address : parseable_without_port) {
        std::cout << "Parsing: " << address << " " << std::flush;
        auto result = IP::parse(address, default_port);
        std::cout << instance.test(result != nullptr) << std::endl;
    }

    return instance;
}



#endif //SIKTACKA_IP_PARSING_HPP
