//
// Created by svp on 08.08.17.
//

#include <parse/parser.hpp>
#include <netinet/in.h>
#include <poll.h>
#include <def/config.hpp>
#include "connect.hpp"

int TimeoutSocket::get_sock(port_t port) {
    int return_sock;
    sockaddr_in server_address {};

    // TODO check which
    return_sock = socket(AF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    if (return_sock < 0)
        syserr("Socket creation failed");

    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port); // port specification

    // bind the socket to a concrete address
    if (bind(return_sock, (struct sockaddr *) &server_address,
             (socklen_t) sizeof(server_address)) < 0)
        syserr("bind");

    return return_sock;
}

maybe<std::tuple<IP, ClientPackage> > TimeoutSocket::receive() const {
    logs(comm, 4) << "TimeoutSocket::receive()" << std::endl;

    pollfd client[1];
    client[0].events = POLLIN;
    client[0].fd = sock;

    sockaddr_in client_address{};

    binary_writer_t byte_writer{config::BUFFER_SIZE};

    ssize_t ret;


    auto rcva_len = (socklen_t) sizeof(client_address);
    const int flags = 0;

    // TODO make timeout update
    ret = poll(client, 1, this->timeout);

    if (ret < 0) {
        syserr("Poll failed");
    }

    if ((client[0].revents & (POLLIN | POLLERR)) != 0) {
        client[0].events = POLLIN;
        client[0].fd = sock;
        client[0].revents = 0;

        size_t buffer_size = byte_writer.size_left();
        ssize_t len = recvfrom(this->sock, byte_writer.get(), buffer_size, flags,
                       (struct sockaddr *) &client_address, &rcva_len);

        if (len < 0)
            syserr("error on datagram from client-stub socket");
        else {
            if (!byte_writer.move(static_cast<size_t>(len))) {
                syserr("trying to write too much");
            }
            logs(comm, 3) << "Read from socket: " << len << " bytes" << std::endl;
        }
    }

    logs(comm, 4) << "Finished exchange" << std::endl;

    if(byte_writer.get_pointer() > 0) {
        auto maybe_client_package = ClientPackage::read(byte_writer.save());
        if(maybe_client_package != nullptr) {
            logs(comm, 2) << "Non trival client-stub package read." << std::endl;
        }
        return map_maybe<ClientPackage, socket_data>
                (maybe_client_package,
                 [client_address](auto client_package) -> auto {
                     logs(comm, 3) << "Client package:(" << notstd::to_string(client_package) << ")" << std::endl;
                     return std::make_tuple(IP(client_address), client_package);
                 });
    } else {
        logs(comm, 4) << "Nothing read from socket" << std::endl;
    }

    return nullptr;
}

bool TimeoutSocket::send(IP address, ServerPackage packages) {

    for (const auto package : packages) {
        int sflags = 0;

        std::weak_ptr<sockaddr_in> client_address = address.get_sockaddr();
        auto snda_len = (socklen_t) sizeof(client_address);
        ssize_t snd_len = sendto(sock, package.bytes.get(), package.length, sflags,
                                 (struct sockaddr *) &client_address, snda_len);

        if (snd_len < 0) {
            syserr("TimeoutSocket::send: error on sending datagram to client-stub socket");
        }
        if(static_cast<size_t>(snd_len) != package.length) {
            syserr("TimeoutSocket::send: error on sending datagram to client-stub socket");
        }
    }


    return false;
}
