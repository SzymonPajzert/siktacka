//
// Created by svp on 17.08.17.
//

#ifndef SIKTACKA_SERIALIZATION_HPP
#define SIKTACKA_SERIALIZATION_HPP

#include "def/types.hpp"
#include "server/connect.hpp"
#include "Test.hpp"


Test test_serialization() {
    Test instance;

    std::vector<ClientPackage> packages = {
        ClientPackage { 0, 0, 0, std::string {""} },
        ClientPackage { 1, 1, 2, std::string {"withoutspace"} },
        ClientPackage { 10, -1, 200, std::string {"player"} }
    };

    for (const auto &package : packages) {
        auto serialized = serialize<ClientPackage>(package);
        auto package2ptr = ClientPackage::read(serialized);

        auto not_null = package2ptr != nullptr;
        std::cout << "Testing deserialized package is not null " << instance.test(not_null) << std::endl;
        if(not_null) {
            auto package2 = *package2ptr;
            auto serialized2 = serialize(package2);

            std::cout << "Testing player with name '" << package.player_name << "': "
                      << instance.test(package == package2) << " "
                      << instance.test(serialized == serialized2)
                      << std::endl;
        } else {
            std::cout << "Failed for package: " << notstd::to_string(package) << std::endl;
        }
    }

    return instance;
}

#endif //SIKTACKA_SERIALIZATION_HPP
