//
// Created by svp on 21.08.17.
//

#include <zlib.h>
#include "ServerConnection.hpp"

maybe<std::vector<ServerUpdate> > ServerConnection::read(binary_t data) {
    auto result = just(std::vector<ServerUpdate>{});

    auto reader = binary_reader_t(std::move(data), net);

    auto game_id = reader.read<game_id_t>();

    while(reader.unread_size() > 0) {
        auto len = *reader.read<uint32_t>();
        auto event_no = *reader.read<event_no_t>();
        logs(client, 2) << "Read event of size: " << event_no << " ";

        auto event_type = *reader.read<event_type_t>();

        if(event_type == NEW_GAME) {
            logs(client, 2, false) << "new game" << std::endl;
            auto width = *reader.read<dim_t>();
            auto height = *reader.read<dim_t>();
            std::vector<std::string> names;

            len -= 4 + 1 + 4 + 4;
            // read until crc
            while (len > 0) {
                auto read_player_name = *reader.read_string();
                names.push_back(read_player_name);
                len -= read_player_name.size() + 1;
            }

            set_player_names(names);
            result->push_back(ServerUpdate::new_game(event_no, width, height, names));
        } else if (event_type == PIXEL) {
            logs(client, 2, false) << "pixel" << std::endl;
            auto player_number = *reader.read<int8_t>();
            auto x = *reader.read<dim_t>();
            auto y = *reader.read<dim_t>();

            auto read_player_name = get_player_name(player_number);
            result->push_back(ServerUpdate::pixel(event_no, read_player_name, x, y));
        } else if (event_type ==PLAYER_ELIMINATED) {
            logs(client, 2, false) << "player eliminated" << std::endl;
            auto player_number = *reader.read<int8_t>();
            auto read_player_name = get_player_name(player_number);
            result->push_back(ServerUpdate::player_eliminated(event_no, read_player_name));
            break;
        } else if (event_type == GAME_OVER) {
            logs(client, 2, false) << "game over" << std::endl;
            result->push_back(ServerUpdate::game_over(event_no));
        }

        const auto crc = reader.read<crc_t>();

        /*if(crc != nullptr) {
            const uLong calculated_crc = crc32(0L, data.bytes.get(), static_cast<uInt>(data.length - sizeof(crc_t)));

            if(calculated_crc != *crc) {
                logs(comm, 4) << "Bad crc in a incoming package" << std::endl;
                result = nullptr;
            }
        } else {
            result = nullptr;
        }*/
    }

    return result;

}
