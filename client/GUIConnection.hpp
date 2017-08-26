//
// Created by svp on 21.08.17.
//

#ifndef SIKTACKA_GUICONNECTION_HPP
#define SIKTACKA_GUICONNECTION_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <def/ipaddr.hpp>
#include "def/types.hpp"
#include "AwaitableSocket.hpp"


class GUIConnection : AwaitableSocket {
public:
    explicit GUIConnection(const IP &gui_address)
        : sock(create_sock(gui_address))
        , exchange(create_exchange(sock)) {}

    GUIConnection(const GUIConnection &) = delete;
    GUIConnection operator=(const GUIConnection &) = delete;
    GUIConnection(const GUIConnection &&) = delete;
    GUIConnection operator=(const GUIConnection &&) = delete;

    maybe<turn_dir_t> read_direction() {
        auto line = read_line();

        if (line == "LEFT_KEY_DOWN") {
            return just<turn_dir_t> (-1);
        } else if (line == "LEFT_KEY_UP" || line == "RIGHT_KEY_UP") {
            return just<turn_dir_t> (0);
        } else if (line == "RIGHT_KEY_DOWN") {
            return just<turn_dir_t> (1);
        } else {
            return nothing<turn_dir_t> ;
        }
    }

    std::string read_line() {
        char buffer[MAX_GUI_COMMAND_SIZE];
        char *res = fgets(buffer, MAX_GUI_COMMAND_SIZE, exchange);
        if (res == nullptr) {
            failure("fgets failed");
        } else {
            size_t len = strlen(buffer);
            if (len == 0 || res[len - 1] != '\n') {
                failure("fgets failed: no \\n terminator");
            } else {
                buffer[len - 1] = 0;
                std::string result {buffer};
                return result;
            }
        }
    }

    void write_line(std::string line) {
        logs(comm, 3) << "Write line: " << line << std::endl;
        if (fprintf(exchange, "%s\n", line.c_str()) < 0) {
            syserr("error sending to exchange");
        }
    }

    int get_sock() const override {
        return sock;
    }

    ~GUIConnection() override {
        (void) close(sock);
    }

    const int sock;
    FILE* const exchange;

    static const size_t MAX_GUI_COMMAND_SIZE = 30;
private:
    static int create_sock(const IP& address) {
        auto addr_result = address.get_sockaddr().get();

        // initialize socket according to getaddrinfo results
        const int result_sock = socket(addr_result->sin_family, SOCK_STREAM, IPPROTO_TCP);
        if (result_sock < 0)
            syserr("socket");

        // connect socket to the server
        if (connect(result_sock, (struct sockaddr *) addr_result, sizeof(sockaddr_in)) < 0)
            syserr("connect");

        return result_sock;
    }

    static FILE* create_exchange(int sock) {
        FILE* const result_exchange = fdopen(sock, "a+");
        if (result_exchange == NULL)
            syserr("fdopen");

        // TODO check if not needed
        setlinebuf(result_exchange);
        // buf_size = 10000;

        return result_exchange;
    }
};


#endif //SIKTACKA_GUICONNECTION_HPP
