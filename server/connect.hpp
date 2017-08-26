//
// Created by svp on 07.08.17.
//

#ifndef SIKTACKA_CONNECT_HPP
#define SIKTACKA_CONNECT_HPP

#include <def/types.hpp>
#include <def/ipaddr.hpp>
#include <vector>
#include <def/binary.hpp>
#include <sstream>
#include <conn/ClientPackage.hpp>

/// Data sent from the server
using ServerPackage = std::vector<binary_t>;


/** Allows reading from socket awaiting messages with timeout
 *
 */
class TimeoutSocket {
public:

    TimeoutSocket(timeout_t _timeout, port_t _port) : timeout(_timeout), sock(get_sock(_port)) {};

    using socket_data = std::tuple<IP, ClientPackage>;

    /** Tries to read the message from the input
     *
     * It will keep reading until the first proper communicate from client
     *
     * @return First proper value or nothing if there is timeout
     */
    maybe<socket_data> receive() const;

    /** Queue for sending given user package
     *
     * @param address Recipient of the package
     * @param package
     * @return
     */
    bool send(IP address, ServerPackage packages);

    sock_t get_sock(port_t port);

    timeout_t timeout; //< timeout after which we'll stop waiting for the messages
    const sock_t sock;
};



#endif //SIKTACKA_CONNECT_HPP
