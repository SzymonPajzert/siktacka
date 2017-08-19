//
// Created by svp on 07.08.17.
//

#ifndef SIKTACKA_GAMESERVER_HPP
#define SIKTACKA_GAMESERVER_HPP

#include <map>
#include <set>
#include "parse/parser.hpp"
#include "connect.hpp"

/** Represents the user playing the game.
 *
 * Contains information such like username, receive address etc.
 */
class Player {
public:
    std::string player_name;
    IP address;

    // Comparison between players is done between their usernames
    bool operator <(const Player& other) {
        std::cerr << "Comparing players" << std::endl;
        if(this->player_name < other.player_name) return true;
        else {
            if(this->player_name == other.player_name) return this->address < other.address;
            else return false;
        }
    }

    IP get_address() {
        return address;
    }
};

using PlayerPtr = std::shared_ptr<Player>;

// TOOD move to util
template<typename T>
struct PtrComp {
    bool operator()(std::shared_ptr<T> lhs, std::shared_ptr<T> rhs) const  {
        return *lhs.get() < *rhs.get();
    }
};

struct PlayerPackage {
    PlayerPtr owner;
    ClientPackage package;
};

class GameServer {
public:
    GameServer(server_param _params)
        : player_directions {}
        , connected_users {}
        , events {}
        , params(_params)
          // Divide the number of miliseconds by number of rounds per second
        , socket(1000 / params.rounds_sec, params.port)
        , random_source(params.seed)
        , game_id {} {

    }
    void run();

private:
    /// Plays one instance of the game
    bool play_game();

    /// Tries to start a new game - waits for all players
    bool start_game();

    /// From collection of currently playing players initialize the map
    bool initialize_map();

    /// Works like @class TimeoutSocket but processes the data
    maybe<PlayerPackage> receive_next();

    // TODO take into accont session_id
    /* TODO(szpajz) - Implement and add to docs:
    Additionally cleans the contact book from players with long inactivity time */
    /** Resolve player given their username and address
     *
     * If the player doesn't exist, it creates one.
     * It also updates the direction of the player if they are resolved.
     *
     * @param data
     * @return
     */
    PlayerPtr resolve_player(TimeoutSocket::socket_data data);


    /** Calculates next iteration step of the game
     *
     * TODO
     * @return Returns whether the game is still going and server data to be sent to every connected user
     */
    std::tuple<bool, ServerPackage> calculate_step();

    /** Get packages requested by the user
     *
     * @param event_no
     * @return
     */
    ServerPackage get_from(event_no_t event_no);

    /** Send given package to every currently listening user
     *
     * @return
     */
    bool broadcast(ServerPackage package);

    /** Predicate necessary to start game
     *
     * @return True if all connected players able to play pressed arrow key
     * and there at least two of them.
     */
    bool can_start() const;

    std::map<PlayerPtr, turn_dir_t, PtrComp<Player>> player_directions;

    // Set of connected users but not necessarily the players
    std::set<PlayerPtr, PtrComp<Player>> connected_users;

    std::map<event_no_t, binary_t> events;

    server_param params;
    TimeoutSocket socket;
    RandomSource random_source;
    game_id_t game_id;

};


#endif //SIKTACKA_GAMESERVER_HPP
