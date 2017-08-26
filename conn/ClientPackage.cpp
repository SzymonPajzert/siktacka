//
// Created by svp on 26.08.17.
//

#include <def/config.hpp>
#include "ClientPackage.hpp"

template<>
binary_t serialize<ClientPackage>(const ClientPackage &package) {
    binary_writer_t writer{config::BUFFER_SIZE};

    writer.write(host_to_net(package.session_id));
    writer.write(host_to_net(package.turn_direction));
    writer.write(host_to_net(package.next_expected_event_no));
    writer.write(package.player_name);

    if(!writer.is_ok()) {
        failure("Failure while serializing ClientPackage");
    }

    return writer.save();
}
