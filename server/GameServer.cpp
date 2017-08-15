//
// Created by svp on 07.08.17.
//

#include <vector>
#include "GameServer.hpp"

void GameServer::run() {
    logs_0 << "Server run" << std::endl;
    while(play_game()) {}
    logs_0 << "Game stopped" << std::endl;
}

bool GameServer::play_game() {
    if (!start_game()) {
        std::cerr << "Game cannot start - cannot collect players" << std::endl;
        return false;
    }

    bool play_game = true;
    while(play_game) {
        maybe<PlayerPackage> read_command;
        while (read_command = receive_next()) {
            map_maybe_(read_command, [this](PlayerPackage command) -> void {
                auto event_no = command.package.next_expected_event_no;
                auto package = get_from(event_no);
                socket.send(command.owner->get_address(), package);
            });
        }

        // We got timeouted and have to process the next step
        auto calculate_result = calculate_step();
        play_game = !std::get<0>(calculate_result);
        ServerPackage next_action = std::get<1>(calculate_result);
    }

    return true;
}



PlayerPtr GameServer::resolve_player(TimeoutSocket::socket_data data) {
    logs_1 << "resolve_player" << std::endl;

    auto player_address = std::get<0>(data);
    auto player_name = std::get<1>(data).player_name;

    PlayerPtr result = nullptr;
    for(const auto player : connected_users) {
        if (player->get_address().same_socket(player_address) and
            player->player_name == player_name) {

            if (result != nullptr) failure("Indistinguishable users");

            result = player;
        }
    }

    // If the user doesn't exist we have to create new one
    if(result == nullptr) {
        logs_2 << "Creating new user: " << player_name << std::endl;
        result = std::make_shared<Player>(Player { player_name, player_address });
    }

    return result;
}

bool GameServer::start_game() {
    while(connected_users.size() < 2) {
        logs_2 << "In loop for start game" << std::endl;
        // TODO probably process package or move processing to the receive next
        auto package = receive_next();

        if(package != nullptr) {
            logs_2 << "Read client package" << std::endl;
        }
    }

    return false;
}

ServerPackage GameServer::get_from(event_no_t event_no) {
    std::vector<binary_t> interesting_events;

    for(auto it = events.find(event_no); it != events.end(); it++) {
        logs_3 << "Serializing event no: " << it->first << std::endl;
        interesting_events.push_back(it->second);
    }

    std::vector<binary_t> serialized_events {};

    // TODO move to the method namespace since calculate step needs it as well
    auto new_writer = [this]() -> auto {
        binary_writer_t writer {TimeoutSocket::BUFFER_SIZE};
        if(!writer.write(this->game_id)) {
            failure("Failed writing to the buffer");
        }
        return writer;
    };

    auto writer = new_writer();
    for(auto event : interesting_events) {
        if(writer.size_left() < event.length) {
            serialized_events.push_back(writer.save());
            writer = new_writer();
        }

        if(!writer.write_bytes(event)) {
            failure("Failed writing to the buffer");
        }
    }
    serialized_events.push_back(writer.save());

    return serialized_events;
}

maybe<PlayerPackage> GameServer::receive_next() {
    return map_maybe<TimeoutSocket::socket_data, PlayerPackage> (socket.receive(),
                                                                  [this](auto data) -> auto {
        return PlayerPackage{ this->resolve_player(data), std::get<1>(data) };
    });
}

std::tuple<bool, ServerPackage> GameServer::calculate_step() {
    // TODO implement
    binary_writer_t writer { TimeoutSocket::BUFFER_SIZE };
    if(!writer.write("No elo ziomki")) {
        failure("Writer failed.");
    }

    std::vector<binary_t> result;
    result.push_back(writer.save());

    return std::make_tuple<bool, ServerPackage>(true, std::move(result));
}


