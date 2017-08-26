//
// Created by svp on 21.08.17.
//

#ifndef SIKTACKA_SERVERCONNECTION_HPP
#define SIKTACKA_SERVERCONNECTION_HPP

#include <utility>
#include <vector>
#include <bits/unique_ptr.h>
#include <def/ipaddr.hpp>
#include <conn/ClientPackage.hpp>
#include <poll.h>
#include <def/config.hpp>
#include "AwaitableSocket.hpp"

struct ServerUpdate {
    event_no_t event_no;
    std::string message;

    bool operator<(ServerUpdate & that) {
        return this->event_no < that.event_no;
    }

    static ServerUpdate new_game(
        event_no_t event_no,
        dim_t width,
        dim_t height,
        std::vector<std::string> names) {

        std::stringstream message;
        message << "NEW_GAME " << width << " " << height;
        for(auto & name : names) {
            message << " " << name;
        }

        return ServerUpdate {event_no, message.str()};
    }

    static ServerUpdate pixel(
        event_no_t event_no,
        const std::string &player_name,
        dim_t x,
        dim_t y
    ) {

        std::stringstream message;
        message << "PIXEL " << x << " " << y << " " << player_name;
        return ServerUpdate {event_no, message.str()};
    }

    static ServerUpdate player_eliminated(
            event_no_t event_no,
            const std::string &player_name
    ) {
        std::stringstream message;
        message << "PLAYER_ELIMINATED " << player_name;
        return ServerUpdate {event_no, message.str()};
    }

    static NO_RETURN  ServerUpdate game_over(event_no_t event_no) {
        failure("GAME ENDED - not implemented");
        // TODO implement
        (void) event_no;
    }
};


class ServerConnection : AwaitableSocket {
public:
    explicit ServerConnection(
            session_t _session_id,
            IP server_address,
            std::string _player_name)
        : session_id(_session_id)
        , sock(create_sock())
        , address(std::move(server_address))
        , player_name(std::move(_player_name))
        , all_player_names() {}

    void send_request(event_no_t last_id, turn_dir_t dir) {
        std::cerr << "Updating for direction: " << (int)dir << "from: " << last_id << std::endl;

        sockaddr_in * server = address.get_sockaddr().get();

        auto package = ClientPackage { session_id, dir, last_id, player_name };

        auto serialized = serialize<ClientPackage>(package);

        int sflags = 0;
        ssize_t sendlen = sendto(
                sock, serialized.bytes.get(), serialized.length, sflags,
                                 (struct sockaddr *) server, (sizeof(sockaddr_in)));

        if (sendlen < 0 or static_cast<size_t >(sendlen) != serialized.length) {
            failure("Failed sending to the server");
        }
    }

    // TODO Returns updates sorted
    std::vector<ServerUpdate> read_updates() {
        std::vector<ServerUpdate> result;
        pollfd socket {};
        socket.revents = POLLIN; // set that value is read first time

        // Read as many as possible available - but first one is assumed to be read
        do {
            binary_writer_t byte_writer{config::BUFFER_SIZE};

            ssize_t len = recv(sock, byte_writer.get(), byte_writer.size_left(), 0);
            logs(client, 2) << "Read from socket data of length: " << len << std::endl;
            if(len < 0) {
                syserr("error on datagram from server connection");
            }
            byte_writer.move(static_cast<size_t>(len));

            auto read_data = read(byte_writer.save());
            if(read_data != nullptr) {
                result.insert(result.end(), read_data->begin(), read_data->end());
            }

            socket.fd = sock;
            socket.events = POLLIN | POLLERR;
            socket.revents = 0;
        } while(poll(&socket, 1, 0) > 0);

        return result;
    }

    // TODO validate
    maybe<std::vector<ServerUpdate> > read(binary_t data);

    void set_player_names(std::vector<std::string> names) {
        all_player_names = names;
    }

    std::string get_player_name(int8_t player_num) const {
        return all_player_names[player_num];
    }

    sock_t get_sock() const override {
        return sock;
    }

private:
    const session_t session_id;
    const int sock;
    IP address;
    std::string player_name;

    std::vector<std::string> all_player_names;

    static int create_sock() {
        const int result_sock = socket(PF_INET, SOCK_DGRAM, 0);
        if (result_sock < 0)
            syserr("socket");
        return result_sock;
    }
};


#endif //SIKTACKA_SERVERCONNECTION_HPP
