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

    bool play_game = true;
    while(play_game) {
        logs(serv, 3) << "In play_game loop" << std::endl;
        maybe<PlayerPackage> read_command = nullptr;
        while ((read_command = receive_next()) != nullptr) {
            logs(serv, 5) << "Received nonempty command";
            map_maybe_(read_command, [this](PlayerPackage command) -> void {
                auto event_no = command.package.next_expected_event_no;
                auto package = get_from(event_no);
                socket.send(command.owner->get_address(), package);
            });
        }

        // We got timeouted and have to process the next step
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

    PlayerPtr result = nullptr;
    for(const auto & player : connected_users) {
        auto same_address = player->get_address().same_socket(player_address);
        auto same_name = player->player_name == player_name;

        logs(serv, 5)
            << "Trying player '" << player->player_name << "' "
            << "Same address(" << same_address << "), name(" << same_name << ")"
            << std::endl;

        if (same_address and same_name) {
            if (result != nullptr) failure("Indistinguishable users");

            result = player;
        }
    }

    // If the user doesn't exist we have to create new one
    if(result == nullptr) {
        logs(serv, 2) << "Creating new user: " << player_name << std::endl;

        // TODO(maybe) consider putting everything into a separate add_player(...) since player instances are game dependent
        result = std::make_shared<Player>(Player { player_name, player_address });

        connected_users.insert(result);
    } else {
        logs(serv, 3) << "Player already exists" << std::endl;
    }

    // If player is successfully resolved, update their direction
    if(result != nullptr) {
        player_directions[result] = std::get<1>(data).turn_direction;
    }


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
            all_pressed = player_directions.at(playerPtr) != 0;
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

        auto package = receive_next();

        if(package != nullptr) {
            logs(serv, 2) << "Read client-stub package" << std::endl;
        }
    }

    return initialize_map();
}


ServerPackage GameServer::get_from(event_no_t event_no) const {
    std::vector<binary_t> interesting_events;

    for(size_t it = event_no; it < events.size(); it++) {
        logs(serv, 3) << "Serializing event no: " << it << std::endl;
        interesting_events.push_back(events[it]);
    }

    std::vector<binary_t> serialized_events {};

    // TODO create big utility adding the binaries together
    auto new_writer = [this]() -> auto {
        binary_writer_t writer { config::BUFFER_SIZE };
        writer.write(this->game_id);

        if(!writer.is_ok()) {
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

        writer.write_bytes(event);
        if(!writer.is_ok()) {
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

// TODO fail if putting head out of bounds
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

bool GameServer::try_put_pixel(PlayerPtr player, disc_position_t position) {
    bool result;

    if(occupied_positions.find(position) == occupied_positions.end()) {
        generate_pixel(player, position);
        result = true;
    } else {
        generate_player_eliminated(player);
        result = false;
    }

    return result;
}

