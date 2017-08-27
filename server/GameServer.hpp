//
// Created by svp on 07.08.17.
//

#ifndef SIKTACKA_GAMESERVER_HPP
#define SIKTACKA_GAMESERVER_HPP

#include <map>
#include <set>
#include <cmath>
#include <zlib.h>
#include <conn/ClientPackage.hpp>
#include "parse/parser.hpp"
#include "connect.hpp"

/** Represents the user playing the game.
 *
 * Contains information such like username, receive address etc.
 */
class Player {
public:
    std::string player_name;
    session_t session_id;
    IP address;

    // Comparison between players is done between their usernames
    bool operator <(const Player& other) {
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

struct angle_t {
    int angle;

    angle_t WARN_UNUSED turn(turn_dir_t direction, speed_t speed) const {
        int newangle = angle + (direction) * speed;
        newangle += 360;
        newangle %= 360;
        return angle_t { newangle };
    }

    double radian() const {
        return angle * PI / 180.0;
    }

    static constexpr double PI = 3.14159265;
};

using disc_position_t = std::pair<dim_t, dim_t>;
struct position_t {
    double x, y;

    position_t move(const angle_t & angle) {
        double x_diff = - sin(angle.radian());
        double y_diff = - cos(angle.radian());

        return position_t { x + x_diff, y + y_diff };
    }

    bool disc_equal(const position_t & that) {
        return this->to_disc() == that.to_disc();
    }

    disc_position_t to_disc() const {
        return std::make_pair(int(floor(x)), int(floor(y)));
    }
};

class GameServer {
public:
    GameServer(server_param _params)
        : player_ids {}
        , player_directions {}
        , last_activity {}
        , head_directions {}
        , head_positions {}
        , occupied_positions {}
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
    maybe<PlayerPackage> receive_next(bool do_timeout=true);

    /** Cleans the contact book from players with long inactivity time
     *
     */
    void player_cleanup();

    /** Resolve player given their username and address
     *
     * If the player doesn't exist, it creates one.
     * It also updates the direction of the player if they are resolved.
     * Additionally cleans the contact book from players with long inactivity time
     *
     * @param data
     * @return
     */
    PlayerPtr resolve_player(TimeoutSocket::socket_data data);


    /** Calculates next iteration step of the game
     *
     * @return Returns whether the game is still going and server data to be sent to every connected user
     */
    std::tuple<bool, ServerPackage> calculate_step();

    event_no_t get_new_event_no() {
        return static_cast<event_no_t>(events.size());
    }

    /** Get packages requested by the user
     *
     * @param event_no
     * @return
     */
    ServerPackage get_from(event_no_t event_no) const;

    /** Send given package to every currently listening user
     *
     */
    void broadcast(const ServerPackage &package) const {
        logs(comm, 3) << "Broadcasting" << std::endl;

        for (auto & connected : connected_users) {
            logs(comm, 4) << "Sending to: " << connected->player_name << std::endl;
            socket.send(connected->address, package);
        }
    }

    /** Predicate necessary to start game
     *
     * @return True if all connected players able to play pressed arrow key
     * and there at least two of them.
     */
    bool can_start() const;

    bool game_finished() const;

    /*                         GAME MANAGEMENT                             */
    using player_id_t = uint8_t;

    player_id_t get_player_id(const PlayerPtr &player) const {
        return player_ids.at(player);
    }

    /** Encapsulates data in a common event format
     *
     * @param event_data
     * @return Size of the encapsulated data
     */
    uint32_t WARN_UNUSED encaps_write_event(binary_t event_data);

    void generate_new_game();

    /** Generate pixel by player and additionally signal its occupancy in the server logic.
     *
     * @param player
     * @param position
     */
    void generate_pixel(const PlayerPtr & player, disc_position_t position);

    void generate_player_eliminated(PlayerPtr player);

    void game_over();

    bool try_put_pixel(PlayerPtr player, disc_position_t position);

    void move_head(PlayerPtr player);



    /*            ATTRIBUTES            */

    std::map<PlayerPtr, player_id_t, PtrComp<Player> > player_ids;
    std::map<PlayerPtr, turn_dir_t, PtrComp<Player> > player_directions;
    std::map<PlayerPtr, timeout_t, PtrComp<Player> > last_activity;
    std::map<PlayerPtr, angle_t, PtrComp<Player> > head_directions;  //< Directions of the players' heads
    std::map<PlayerPtr, position_t, PtrComp<Player> > head_positions; //< Positions of the players' heads

    std::set<disc_position_t> occupied_positions; //< Positions occupied by the pixels

    // Set of connected users but not necessarily the players
    std::set<PlayerPtr, PtrComp<Player>> connected_users;

    std::vector<binary_t> events;

    server_param params;
    TimeoutSocket socket;
    RandomSource random_source;
    game_id_t game_id;

};


#endif //SIKTACKA_GAMESERVER_HPP
