//
// Created by svp on 27.08.17.
//

#include "ipaddr.hpp"

std::shared_ptr <IP> IP::parse(proto_t protocol, const std::string &ip_string) {
    // find the position of address delimiter
    auto delim_pos = ip_string.find_last_of(":");
    maybe<IP> result = nullptr;

    if(delim_pos != std::string::npos && delim_pos + 1 != ip_string.length()) {
        auto port_string = ip_string.substr(delim_pos+1);
        auto address_string = ip_string.substr(0, delim_pos);

        port_t port_number;
        if(maybe_assign(parse_int<port_t>(port_string), port_number)) {
            result = IP::parse(protocol, address_string, port_number);
        }
    }

    return result;
}

maybe<sockaddr_in> IP::convert_address(proto_t protocol, const std::string &ip_string, port_t port) {
    std::shared_ptr<sockaddr_in> address = nullptr;
    struct addrinfo addr_hints {};
    struct addrinfo *addr_result;

    for(sa_family_t ai_family : std::vector<sa_family_t>{AF_INET, AF_INET6}) {
        // 'converting' host/port in string to struct addrinfo
        (void) memset(&addr_hints, 0, sizeof(struct addrinfo));
        addr_hints.ai_family = ai_family;
        addr_hints.ai_socktype = (protocol == UDP ? SOCK_DGRAM : SOCK_STREAM);
        addr_hints.ai_protocol = (protocol == UDP ? IPPROTO_UDP : IPPROTO_TCP);
        addr_hints.ai_flags = 0;
        addr_hints.ai_addrlen = 0;
        addr_hints.ai_addr = NULL;
        addr_hints.ai_canonname = NULL;
        addr_hints.ai_next = NULL;

        if (getaddrinfo(ip_string.c_str(), NULL, &addr_hints, &addr_result) == 0) {
            address = std::make_shared<sockaddr_in>(sockaddr_in {});

            address->sin_family = ai_family; // IPv4
            address->sin_addr.s_addr = ((struct sockaddr_in *) (addr_result->ai_addr))->sin_addr.s_addr; // address IP
            address->sin_port = htons(port);

            freeaddrinfo(addr_result);
        }
    }

    if(address == nullptr) {
        logs(addr, 3) << "getaddrinfo failed" << std::endl;
    }

    return address;
}
