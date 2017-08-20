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
        auto package2 = *(ClientPackage::read(serialized).get());
        auto serialized2 = serialize(package2);


        std::cout << "Testing player with name '" << package.player_name << "': "
            << instance.test(package == package2) << " "
            << instance.test(serialized == serialized2)
            << std::endl;
    }

    return instance;
}

#endif //SIKTACKA_SERIALIZATION_HPP
