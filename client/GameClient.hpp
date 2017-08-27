//
// Created by svp on 21.08.17.
//

#ifndef SIKTACKA_GAMECLIENT_HPP
#define SIKTACKA_GAMECLIENT_HPP


#include <parse/parser.hpp>
#include <utility>
#include <utility>
#include <poll.h>
#include "GUIConnection.hpp"
#include "ServerConnection.hpp"

class GameClient {
public:
    explicit GameClient(client_param _params)
    : session_id(get_session_id())
    , params(std::move(_params))
    , gui_conn(params.gui_address)
    , serv_conn(session_id, params.server_address, params.player_name)
    , next_id(0)
    , direction(0)
    {}

    void run() {
        pollfd sockets[2];
        sockets[0].fd = gui_conn.get_sock();
        sockets[1].fd = serv_conn.get_sock();

        while(true) {
            logs(client, 2) << "Restart client" << std::endl;

            next_id = 0;
            direction = 0;

            while (true) {
                for (auto &socket : sockets) {
                    socket.events = POLLIN | POLLERR;
                    socket.revents = 0;
                }

                logs(client, 4) << "Starting waiting for message" << std::endl;
                int poll_result;
                if ((poll_result = poll(sockets, 2, config::CLIENT_UPDATE_TIMEOUT)) > 0) {
                    // Got GUI update
                    if ((sockets[0].revents & (POLLIN | POLLERR)) != 0) {
                        if ((sockets[0].revents & POLLERR) != 0) {
                            failure("GUI failed");
                        }

                        logs(client, 2) << "Got GUI update" << std::endl;

                        auto new_dir_read = gui_conn.read_direction();

                        map_maybe_(new_dir_read, [this](auto new_dir) -> void {
                            this->direction = new_dir;
                        });
                    }

                    // Got server informations
                    if ((sockets[1].revents & (POLLIN | POLLERR)) != 0) {
                        logs(client, 2) << "Got server informations" << std::endl;

                        auto updates = serv_conn.read_updates();
                        for (const auto &update : updates) {
                            logs(client, 4) << "Processing update: " << update.event_no;

                            if (update.event_no == next_id) {
                                if(update.message == "GAME_OVER") break;

                                logs(client, 4, false) << " correct";
                                next_id++;
                                gui_conn.write_line(update.message);
                            }

                            logs(client, 4, false) << std::endl;
                        }
                    }

                } else if (poll_result == 0) {
                    logs(comm, 4) << "Nothing updated for client" << std::endl;
                } else {
                    failure("Failing reading socket");
                }

                logs(client, 4) << "Updating direction" << std::endl;
                serv_conn.send_request(next_id, direction);
            }
        }
    }

private:
    session_t get_session_id() const {
        return static_cast<session_t>(time(nullptr));
    }

    session_t session_id;
    client_param params;

    GUIConnection gui_conn;
    ServerConnection serv_conn;

    event_no_t next_id;
    turn_dir_t direction;
};


#endif //SIKTACKA_GAMECLIENT_HPP
