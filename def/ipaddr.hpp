//
// Created by svp on 06.08.17.
//

#ifndef SIKTACKA_IPADDR_HPP
#define SIKTACKA_IPADDR_HPP

#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include "types.hpp"
#include "util.hpp"

enum proto_t {
    UDP,
    TCP
};

struct IP {
    explicit IP(sockaddr_in sock_address) : address(std::make_shared<sockaddr_in>(sock_address)) {}

    /** Parse IP address from a string assuming it contains only address without port
     *
     * @param ip_string
     * @param port Value of the port to be assigned while creating instance of the address
     * @return
     */
    static maybe<IP> parse(proto_t protocol, const std::string & ip_string, port_t port) {
        return map_maybe<sockaddr_in, IP>(convert_address(protocol, ip_string, port), [](auto converted_address)-> auto {
            return IP(converted_address);
        });
    }

    /** Parse IP address from a string assuming it has port specified
     *
     * @param ip_string String representing IPv4 or IPv6 address with port number
     * @return IP address represented by the string
     */
    static maybe<IP> parse(proto_t protocol, const std::string & ip_string);



    // TODO(maybe) move to IPv6 after test creation
    std::shared_ptr<sockaddr_in> get_sockaddr() const {
        return address;
    }

    /** Checks whether two adresses are the same
     *
     * @param address Address to be compared
     * @return True if they come from the same address
     */
    bool same_socket(const IP & that) {
        auto this_address = this->get_sockaddr();
        auto that_address = that.get_sockaddr();

        auto same_sock = this_address->sin_addr.s_addr == that_address->sin_addr.s_addr;
        auto same_port = this_address->sin_port == that_address->sin_port;

        return same_sock && same_port;
    }

    // Comparison between players is done between their usernames
    bool operator <(const IP& that) {
        auto this_address = this->get_sockaddr();
        auto that_address = that.get_sockaddr();

        if(this_address->sin_addr.s_addr < that_address->sin_addr.s_addr) return true;
        else {
            if(this_address->sin_addr.s_addr ==  that_address->sin_addr.s_addr) return this_address->sin_port < that_address->sin_port;
            else return false;
        }
    }

private:
    static maybe<sockaddr_in> convert_address(proto_t protocol, const std::string & ip_string, port_t port);

    std::shared_ptr<sockaddr_in> address;
};




#endif //SIKTACKA_IPADDR_HPP
