//
// Created by svp on 07.08.17.
//

#ifndef SIKTACKA_GAMESERVER_HPP
#define SIKTACKA_GAMESERVER_HPP

#include <map>
#include <set>
#include <cmath>
#include <def/config.hpp>
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

using disc_position_t = std::pair<size_t, size_t>;
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
    void broadcast(ServerPackage package) const {
        (void)package;
        // TODO implement
    }

    /** Predicate necessary to start game
     *
     * @return True if all connected players able to play pressed arrow key
     * and there at least two of them.
     */
    bool can_start() const;

    bool game_finished() const;



    /*                         GAME MANAGEMENT                             */
    using event_type_t = int8_t;
    using player_id_t = uint8_t;

    player_id_t get_player_id(PlayerPtr player) const {
        return player_ids.at(player);
    }

    static const event_type_t NEW_GAME = 0;
    static const event_type_t PIXEL = 1;
    static const event_type_t PLAYER_ELIMINATED = 2;
    static const event_type_t GAME_OVER = 3;

    void generate_new_game() {
        logs(serv, 3) << "generate: new_game" << std::endl;
        auto event = get_new_event_no();

        binary_writer_t writer { config::BUFFER_SIZE };
        bool success = true;
        success = success and writer.write(host_to_net(event));
        success = success and writer.write(host_to_net(GameServer::NEW_GAME));
        success = success and writer.write(host_to_net(params.width));
        success = success and writer.write(host_to_net(params.height));

        if(success) {
            for(auto player_dir : player_directions) {
                auto player = player_dir.first;
                success = success and writer.write(player->player_name);
                success = success and writer.write('\0');
            }
        }

        if(!success) {
            failure("writing in generate_new_game failed");
        }

        events.push_back(writer.save());
    }

    /** Generate pixel by player and additionally signal its occupancy in the server logic.
     *
     * @param player
     * @param position
     */
    void generate_pixel(const PlayerPtr player, disc_position_t position) {
        logs(serv, 3) << "generate: pixel" << std::endl;
        auto event = get_new_event_no();

        binary_writer_t writer { config::BUFFER_SIZE };
        bool success = true;
        success = success and writer.write(host_to_net(event));
        success = success and writer.write(host_to_net(GameServer::PIXEL));
        success = success and writer.write(host_to_net<uint8_t>(get_player_id(player)));
        success = success and writer.write(host_to_net(position.first));
        success = success and writer.write(host_to_net(position.second));

        if(!success) {
            failure("writing in generate_pixel failed");
        }

        events.push_back(writer.save());
    }

    void generate_player_eliminated(PlayerPtr player) {
        logs(serv, 3) << "generate: player_eliminated" << std::endl;
        auto event = get_new_event_no();

        binary_writer_t writer { config::BUFFER_SIZE };
        bool success = true;
        success = success and writer.write(host_to_net(event));
        success = success and writer.write(host_to_net(GameServer::PLAYER_ELIMINATED));
        success = success and writer.write(host_to_net<uint8_t>(get_player_id(player)));

        if(!success) {
            failure("writing in generate_new_game failed");
        }

        events.push_back(writer.save());
    }

    void game_over();

    bool try_put_pixel(PlayerPtr player, disc_position_t position);

    void move_head(PlayerPtr player);

    std::map<PlayerPtr, player_id_t, PtrComp<Player> > player_ids;
    std::map<PlayerPtr, turn_dir_t, PtrComp<Player> > player_directions;

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
