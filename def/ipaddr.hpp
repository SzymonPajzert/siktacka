//
// Created by svp on 06.08.17.
//

#ifndef SIKTACKA_IPADDR_HPP
#define SIKTACKA_IPADDR_HPP

#include <netinet/in.h>
#include <netdb.h>
#include "types.hpp"
#include "util.hpp"

class IP {
public:
    explicit IP(sockaddr_in sock_address) : address(std::make_shared<sockaddr_in>(sock_address)) {}

    /** Parse IP address from a string assuming it contains only address without port
     *
     * @param ip_string
     * @param port Value of the port to be assigned while creating instance of the address
     * @return
     */
    static maybe<IP> parse(const std::string & ip_string, port_t port) {
        return map_maybe<sockaddr_in, IP>(convert_address(ip_string, port), [](auto converted_address)-> auto {
            return IP(converted_address);
        });
    }

    // TODO move
    static maybe<sockaddr_in> convert_address(const std::string & ip_string, port_t port) {
        std::shared_ptr<sockaddr_in> address;
        struct addrinfo addr_hints {};
        struct addrinfo *addr_result;

        // 'converting' host/port in string to struct addrinfo
        (void) memset(&addr_hints, 0, sizeof(struct addrinfo));
        addr_hints.ai_family = AF_INET; // IPv4
        addr_hints.ai_socktype = SOCK_DGRAM;
        addr_hints.ai_protocol = IPPROTO_UDP;
        addr_hints.ai_flags = 0;
        addr_hints.ai_addrlen = 0;
        addr_hints.ai_addr = NULL;
        addr_hints.ai_canonname = NULL;
        addr_hints.ai_next = NULL;

        if (getaddrinfo(ip_string.c_str(), NULL, &addr_hints, &addr_result) == 0) {
            address = std::make_shared<sockaddr_in>(sockaddr_in {});

            address->sin_family = AF_INET; // IPv4
            address->sin_addr.s_addr = ((struct sockaddr_in *) (addr_result->ai_addr))->sin_addr.s_addr; // address IP
            address->sin_port = htons(port);
        } // port from the command line

        freeaddrinfo(addr_result);

        return address;
    }

    // TODO move to IPv6 after test creation
    std::shared_ptr<sockaddr_in> get_sockaddr() {
        return address;
    }

    /** Checks whether two adresses are the same
     *
     * @param address Address to be compared
     * @return True if they come from the same address
     */
    bool same_socket(const IP & other_address) {
        // TODO implement
        (void) other_address;
        return false;
    }

private:

    std::shared_ptr<sockaddr_in> address;
};


// TODO probably make it static method as well
/** Parse IP address from a string assuming it has port specified
 *
 * @param ip_string String representing IPv4 or IPv6 address with port number
 * @return IP address represented by the string
 */
inline maybe<IP> parse_address(const std::string & ip_string) {
    // find the position of address delimiter
    auto delim_pos = ip_string.find_last_of(":");
    maybe<IP> result = nullptr;

    if(delim_pos != std::string::npos && delim_pos + 1 != ip_string.length()) {
        auto port_string = ip_string.substr(delim_pos+1);
        auto address_string = ip_string.substr(0, delim_pos);

        port_t port_number;
        if(maybe_assign(parse_int<port_t>(port_string), port_number)) {
            result = IP::parse(address_string, port_number);
        }
    }

    return result;
}

#endif //SIKTACKA_IPADDR_HPP
