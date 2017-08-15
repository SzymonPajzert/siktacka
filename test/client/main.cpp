//
// Created by svp on 13.08.17.
//

#include <unistd.h>

#include "parse/parser.hpp"
#include "server/connect.hpp"

enum dir_t {
    left = -1,
    forward = 0,
    right = 1
};

std::shared_ptr<client_param> params;

void send_update(sock_t sock, dir_t dir) {
    sockaddr_in * server = params->server_address.get_sockaddr().get();

    auto package = ClientPackage { 0, dir, 0, std::string {"player"} };
    auto serialized = serialize<ClientPackage>(package);

    int sflags = 0;
    ssize_t sendlen = sendto(sock, serialized.bytes.get(), serialized.length, sflags,
           (struct sockaddr *) server, (sizeof(sockaddr_in)));

    if (sendlen < 0 or static_cast<size_t >(sendlen) != serialized.length) {
        failure("Failed sending to the server");
    }
}

int main(int argc, char* argv[]) {
    params = parse_client(argc, argv);

    const sock_t sock = socket(PF_INET, SOCK_DGRAM, 0);


    bool go_on = true;
    while(go_on) {
        std::cout << "Direction: ";
        dir_t dir;
        char c;
        std::cin >> c;
        switch(c) {
            case 'l': dir = left; break;
            case 'r': dir = right; break;
            case 'u': dir = forward; break;
            case 's': go_on = false; break;
            default: break;
        }

        send_update(sock, dir);
    }

}



