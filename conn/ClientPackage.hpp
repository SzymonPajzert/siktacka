//
// Created by svp on 26.08.17.
//

#ifndef SIKTACKA_CLIENTPACKAGE_HPP
#define SIKTACKA_CLIENTPACKAGE_HPP


#include <def/binary.hpp>
#include <parse/parser.hpp>

/** Class implementing package from client
 *
 * Allows reading from binary form to deserialize the data
 *
 */
struct ClientPackage {
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
    static maybe<ClientPackage> read(binary_t data) {
        auto reader = binary_reader_t(data, net);

        maybe<session_t> session_id = reader.read<session_t>();
        maybe<turn_dir_t> turn_direction = reader.read<turn_dir_t>();
        maybe<event_no_t> next_expected_event_no = reader.read<event_no_t>();

        maybe<std::string> player_name = reader.read_string();
        if (player_name) {
            if (!client_param::correct_player_name(*player_name)) {
                player_name = nullptr;
            }
        }

        maybe<ClientPackage> result = nullptr;
        if (session_id && turn_direction && next_expected_event_no && player_name) {
            logs(comm, 3) << "ClientPackage::read succeeded" << std::endl;
            result = std::make_shared<ClientPackage>(ClientPackage {
                    *session_id,
                    *turn_direction,
                    *next_expected_event_no,
                    *player_name});

            logs(comm, 4) << result->session_id << std::endl;
        }

        if(reader.counter != data.length) {
            logs(comm, 2) << "ClientPackage::read didn't read all the data ["
                          << reader.counter << "/" << data.length <<
                          "] - removing result" << std::endl;
            result = nullptr;
        }

        return result;
    }


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

#endif //SIKTACKA_CLIENTPACKAGE_HPP
