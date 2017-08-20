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

#include "types.hpp"

/** Class implementing package from client
 *
 * Allows reading from binary form to deserialize the data
 *
 */
struct ClientPackage {
    using session_t = uint64_t;

    session_t session_id {};
    turn_dir_t turn_direction {};
    event_no_t next_expected_event_no {};
    std::string player_name;


    /** Read the data and possibly return the client communicate
     *
     * Performs additional conversion from network to host byte order
     *
     * @param data Data to be parsed
     * @return Maybe proper client package
     */
    static maybe<ClientPackage> read(binary_t data);

    bool operator==(const ClientPackage& that) const{
        return
            (this->session_id == that.session_id) and
            (this->turn_direction == that.turn_direction) and
            (this->next_expected_event_no == that.next_expected_event_no) and
            (this->player_name == that.player_name);
    }

};

namespace notstd {
    // TODO(maybe) move
    inline std::string to_string(const ClientPackage& package) {
        std::stringstream result;
        result
                << package.session_id << " "
                << (int) package.turn_direction << " "
                << package.next_expected_event_no << " "
                << "'" << package.player_name << "'" ;

        return result.str();
    }
}

template<>
binary_t serialize<ClientPackage>(const ClientPackage & package);

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
