//
// Created by svp on 07.08.17.
//

#include <vector>
#include "GameServer.hpp"

void GameServer::run() {
    logs(serv, 0) << "Server run" << std::endl;
    while(play_game()) {}
    logs(serv, 0) << "Game stopped" << std::endl;
}

bool GameServer::play_game() {
    logs(serv, 0) << "Play game called" << std::endl;
    if (!start_game()) {
        std::cerr << "Game cannot start - cannot collect players" << std::endl;
        return false;
    }
    broadcast(get_from(0));

    bool play_game = true;
    while(play_game) {
        logs(serv, 3) << "In play_game loop" << std::endl;
        maybe<PlayerPackage> read_command = nullptr;
        while ((read_command = receive_next()) != nullptr) {
            logs(serv, 5) << "Received nonempty command";
            map_maybe_(read_command, [this](PlayerPackage command) -> void {
                auto event_no = command.package.next_expected_event_no;
                logs(comm, 2) << "Responding to client request" << std::endl;
                auto package = get_from(event_no);
                socket.send(command.owner->get_address(), package);
            });
        }

        // We got timeouted and have to process the next step
        socket.reset();

        auto calculate_result = calculate_step();
        play_game = !std::get<0>(calculate_result);
        ServerPackage next_action = std::move(std::get<1>(calculate_result));

        broadcast(next_action);
    }

    return true;
}



PlayerPtr GameServer::resolve_player(TimeoutSocket::socket_data data) {
    logs(serv, 1) << "resolve_player" << std::endl;

    auto player_address = std::get<0>(data);
    auto player_name = std::get<1>(data).player_name;
    auto player_session_id = std::get<1>(data).session_id;

    PlayerPtr result = nullptr;
    for(const auto & player : connected_users) {
        auto same_address = player->get_address().same_socket(player_address);
        auto same_name = player->player_name == player_name;

        logs(serv, 5)
            << "Trying player '" << player->player_name << "' "
            << "Same address(" << same_address << "), name(" << same_name << ")"
            << std::endl;

        if (same_address and same_name) {
            if (result != nullptr and result->session_id == player_session_id) failure("Indistinguishable users");
            if (result->session_id < player_session_id) {
                result->session_id = player_session_id;
            }

            result = player;
        }
    }

    // If the user doesn't exist we have to create new one
    if(result == nullptr) {
        logs(serv, 2) << "Creating new user: " << player_name << std::endl;

        result = std::make_shared<Player>(Player { player_name, player_session_id, player_address });

        connected_users.insert(result);
    } else {
        logs(serv, 3) << "Player already exists" << std::endl;
    }

    // If player is successfully resolved, update their direction
    if(result != nullptr) {
        player_directions[result] = std::get<1>(data).turn_direction;
        last_activity[result] = get_cur_time();
    }

    player_cleanup();

    return result;
}

bool GameServer::can_start() const {
    logs(serv, 6) << "Called can_start";

    bool all_pressed = true;
    size_t player_count = 0;

    for(const auto & playerPtr : connected_users) {
        if(!playerPtr->player_name.empty()) {
            player_count++;
            std::cerr << player_directions.size() << std::endl;
            all_pressed = all_pressed and player_directions.find(playerPtr) != player_directions.end();
            logs(serv, 7) << " " << all_pressed;
            if(!all_pressed) break;
        }
    }

    logs(serv, 6) << std::endl;

    bool result = (all_pressed && player_count >= 2);
    logs(serv, 4) << "Called can_start(" << result << ")" << std::endl;

    return result;
}

bool GameServer::game_finished() const {
    return player_directions.size() == 1 and head_directions.size() == 1 and head_positions.size() == 1;
}

bool GameServer::start_game() {
    while(!can_start()) {
        logs(serv, 2) << "In loop for start game" << std::endl;

        auto package = receive_next(false);

        if(package != nullptr) {
            logs(serv, 2) << "Read client package" << std::endl;
        }
    }

    return initialize_map();
}


ServerPackage GameServer::get_from(event_no_t event_no) const {
    std::vector<binary_t> interesting_events;

    for(size_t it = event_no; it < events.size(); it++) {
        logs(serv, 4) << "Serializing event no: " << it << std::endl;

        interesting_events.push_back(events[it]);
    }

    std::vector<binary_t> serialized_events {};

    auto new_writer = [this]() -> auto {
        binary_writer_t writer { config::UDP_SIZE };
        writer.write(this->game_id);

        if(!writer.is_ok()) {
            failure("Failed writing to the buffer");
        }
        return writer;
    };

    auto writer = new_writer();
    for(auto event : interesting_events) {
        logs(serv, 4) << "Event serialized" << std::endl;

        if(writer.size_left() < event.length) {
            logs(serv, 3) << "Full package, creating new" << std::endl;
            serialized_events.push_back(writer.save());
            writer = new_writer();
        }

        writer.write_bytes(event);
        if(!writer.is_ok()) {
            failure("Failed writing to the buffer");
        }
    }
    logs(serv, 3) << "Finished writing package" << std::endl;
    serialized_events.push_back(writer.save());

    return serialized_events;
}

maybe<PlayerPackage> GameServer::receive_next(bool do_timeout) {
    return map_maybe<TimeoutSocket::socket_data, PlayerPackage> (socket.receive(do_timeout),
                                                                  [this](auto data) -> auto {
        return PlayerPackage{ this->resolve_player(data), std::get<1>(data) };
    });
}

std::tuple<bool, ServerPackage> GameServer::calculate_step() {
    logs(serv, 2) << "Calculate next step - len(player_directions) == " << player_directions.size() << std::endl;
    auto new_action_id = get_new_event_no();

    for(auto player_dir : player_directions) {
        auto player = player_dir.first;
        auto direction = player_dir.second;

        head_directions[player].turn(direction, params.turning);

        move_head(player);
    }

    auto new_actions = get_from(new_action_id);
    return std::make_tuple<bool, ServerPackage>(game_finished(), std::move(new_actions));
}

void GameServer::move_head(PlayerPtr player) {
    logs(serv, 3) << "Move head" << std::endl;
    auto position = head_positions[player];
    auto new_position = position.move(head_directions[player]);

    bool player_playing = true;
    if(not new_position.disc_equal(position)) {
        player_playing = try_put_pixel(player, new_position.to_disc());
    }

    if(player_playing) {
        head_positions[player] = new_position;
    }
}

bool GameServer::initialize_map() {
    logs(serv, 1) << "Initializing map" << std::endl;

    game_id = static_cast<game_id_t>(random_source.get_next());

    generate_new_game();

    player_id_t current_id = 0;

    for(auto player_dir : player_directions) {
        auto player = player_dir.first;

        // Initialize player id
        player_ids[player] = current_id++;

        auto x = random_source.get_next() % params.width + 0.5;
        auto y = random_source.get_next() % params.height + 0.5;
        if(try_put_pixel(player, std::make_pair(x, y))) {
            head_directions[player] = angle_t { static_cast<int>(random_source.get_next() % 360) };
        }
    }

    return true;
}

template<typename T>
bool in(const T & x, const T & first, const T & second) {
    return x >= first and x <= second;
}

bool GameServer::try_put_pixel(PlayerPtr player, disc_position_t position) {
    bool result =
        in(position.first, 0, params.width - 1) and
        in(position.second, 0, params.height - 1) and
        (occupied_positions.find(position) == occupied_positions.end());

    if(result) {
        generate_pixel(player, position);
    } else {
        generate_player_eliminated(player);
    }

    return result;
}

len_t WARN_UNUSED GameServer::encaps_write_event(binary_t event_data) {
    auto len = static_cast<len_t>(event_data.length + 4 + 4); // take into account len + crc32;

    binary_writer_t writer { config::EVENT_SIZE };
    writer.write<uint32_t>(len);
    writer.write_bytes(event_data);

    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const unsigned char*)&len, sizeof(len));
    crc = crc32(crc, event_data.bytes.get(), static_cast<uInt>(event_data.length));

    writer.write<crc_t>(static_cast<crc_t>(crc));

    if(writer.is_ok()) {
        events.push_back(writer.save());
    } else {
        failure("Writer failed");
    }

    return len;
}

void check_size(len_t saved_size, len_t required_size, const std::string &description) {
    if(saved_size != required_size) {
        logs(errors, 0) << saved_size << " != " << required_size;
        failure(description + " - number of bytes written is wrong ");
    }
}

void GameServer::generate_new_game() {
    logs(serv, 3) << "generate: new_game" << std::endl;
    auto event = get_new_event_no();

    binary_writer_t writer { config::EVENT_SPECIFIC_SIZE };

    writer.write(host_to_net(event));
    writer.write(host_to_net(NEW_GAME));
    writer.write(host_to_net(params.width));
    writer.write(host_to_net(params.height));

    if(writer.is_ok()) {
        for(auto player_dir : player_directions) {
            auto player = player_dir.first;
            writer.write(player->player_name);
            writer.write('\0');
        }
    }

    if(!writer.is_ok()) {
        failure("writing in generate_new_game failed");
    }

    len_t required_size = 4+4+1+(2 * sizeof(dim_t)) + 4;
    for(auto player_dir : player_directions) {
        auto player = player_dir.first;
        required_size += 1 + player->player_name.size();
    }

    check_size(encaps_write_event(writer.save()), required_size, "generate_new_game");
}

void GameServer::generate_pixel(const PlayerPtr &player, disc_position_t position) {
    logs(serv, 3) << "generate: pixel" << std::endl;
    event_no_t event = get_new_event_no();

    binary_writer_t writer { config::EVENT_SPECIFIC_SIZE };

    writer.write(host_to_net<event_no_t>(event));
    writer.write(host_to_net<event_type_t>(PIXEL));
    writer.write(host_to_net<uint8_t>(get_player_id(player)));
    writer.write(host_to_net(position.first));
    writer.write(host_to_net(position.second));

    if(!writer.is_ok()) {
        failure("writing in generate_pixel failed");
    }

    check_size(encaps_write_event(writer.save()), 4+4+1+(1+4+4)+4, "generate_pixel");
}

void GameServer::generate_player_eliminated(PlayerPtr player) {
    logs(serv, 3) << "generate: player_eliminated" << std::endl;
    auto event = get_new_event_no();

    binary_writer_t writer { config::EVENT_SPECIFIC_SIZE };
    writer.write(host_to_net(event));
    writer.write(host_to_net(PLAYER_ELIMINATED));
    writer.write(host_to_net<uint8_t>(get_player_id(player)));

    if(!writer.is_ok()) {
        failure("writing in generate_new_game failed");
    }

    check_size(encaps_write_event(writer.save()), 4+4+1+(1)+4, "generate_player_eliminated");
}

void GameServer::player_cleanup() {
    auto cur_time = get_cur_time();

    for(const auto & playerDir : player_directions) {
        if(last_activity.at(playerDir.first) < cur_time - config::INACTIVITY_KICK) {
            logs(serv, 3) << "Removing player " << playerDir.first->player_name << std::endl;
            player_directions.erase(playerDir.first);
        }
    }
}
