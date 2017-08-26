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

        bool keep_going = true;
        while(keep_going) {
            bool direction_changed = false;

            for (auto &socket : sockets) {
                socket.events = POLLIN | POLLERR;
                socket.revents = 0;
            }

            logs(client, 1) << "Starting waiting for message" << std::endl;
            if (poll(sockets, 2, -1) > 0) {
                // Got GUI update
                if((sockets[0].revents & (POLLIN | POLLERR)) != 0) {
                    if((sockets[0].revents & POLLERR) != 0) {
                        failure("GUI failed");
                    }

                    logs(client, 2) << "Got GUI update" << std::endl;

                    auto new_dir = gui_conn.read_direction();

                    if(new_dir != nullptr && *new_dir.get() != direction) {
                        direction = *new_dir.get();
                        direction_changed = true;
                    }
                }

                // Got server informations
                if((sockets[1].revents & (POLLIN | POLLERR)) != 0) {
                    logs(client, 2) << "Got server informations" << std::endl;

                    auto updates = serv_conn.read_updates();
                    for(const auto & update : updates) {
                        logs(client, 3) << "Processing update: " << update.event_no;

                        if(update.event_no == next_id) {
                            logs(client, 3, false) << " correct";
                            next_id++;
                            gui_conn.write_line(update.message);
                        }

                        logs(client, 3, false) << std::endl;
                    }
                }

            } else {
                failure("Failing reading socket");
            }

            if(direction_changed) {
                logs(client, 2) << "Updating direction" << std::endl;
                serv_conn.send_request(next_id, direction);
            }
        }
    }

private:
    session_t get_session_id() const {
        return time(nullptr);
    }

    session_t session_id;
    client_param params;

    GUIConnection gui_conn;
    ServerConnection serv_conn;

    event_no_t next_id;
    turn_dir_t direction;
};


#endif //SIKTACKA_GAMECLIENT_HPP
