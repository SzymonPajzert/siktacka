//
// Created by svp on 07.08.17.
//

#ifndef SIKTACKA_GAMESERVER_HPP
#define SIKTACKA_GAMESERVER_HPP

#include <map>
#include <set>
#include <cmath>
#include <def/config.hpp>
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
    // TODO check the return size and assert it for constant lengths
    uint32_t WARN_UNUSED encaps_write_event(binary_t event_data) {
        // TODO add this type to types
        auto len = static_cast<uint32_t>(event_data.length + 4 + 4); // len field + crc32;

        binary_writer_t writer { config::BUFFER_SIZE };
        writer.write<uint32_t>(len);
        writer.write_bytes(event_data);

        uLong crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, (const unsigned char*)&len, sizeof(len));
        crc = crc32(crc, event_data.bytes.get(), static_cast<uInt>(event_data.length));

        writer.write<uint32_t>(static_cast<uint32_t>(crc));

        if(writer.is_ok()) {
            events.push_back(writer.save());
        } else {
            failure("Writer failed");
        }

        return len;
    }

    void generate_new_game() {
        logs(serv, 3) << "generate: new_game" << std::endl;
        auto event = get_new_event_no();

        binary_writer_t writer { config::BUFFER_SIZE };

        writer.write(host_to_net(event));
        writer.write(host_to_net(NEW_GAME));
        writer.write(host_to_net(params.width));
        writer.write(host_to_net(params.height));

        if(writer.is_ok()) {
            for(auto player_dir : player_directions) {
                auto player = player_dir.first;
                writer.write(player->player_name);
                writer.write('\0');
            }
        }

        if(!writer.is_ok()) {
            failure("writing in generate_new_game failed");
        }

        // TODO calculate
        auto result = encaps_write_event(writer.save());
        (void) result;
    }

    /** Generate pixel by player and additionally signal its occupancy in the server logic.
     *
     * @param player
     * @param position
     */
    void generate_pixel(const PlayerPtr & player, disc_position_t position) {
        logs(serv, 3) << "generate: pixel" << std::endl;
        event_no_t event = get_new_event_no();

        binary_writer_t writer { config::BUFFER_SIZE };

        writer.write(host_to_net<event_no_t>(event));
        writer.write(host_to_net<event_type_t>(PIXEL));
        writer.write(host_to_net<uint8_t>(get_player_id(player)));
        writer.write(host_to_net(position.first));
        writer.write(host_to_net(position.second));

        if(!writer.is_ok()) {
            failure("writing in generate_pixel failed");
        }

        auto data = writer.save();

        auto saved_size = encaps_write_event(data);
        typeof(saved_size) required_size = 4+4+1+(1+4+4)+4;
        if(saved_size != required_size) {
            logs(errors, 0) << saved_size << " != " << required_size;
            failure("generate_pixel - number of bytes written is wrong ");
        }
    }

    void generate_player_eliminated(PlayerPtr player) {
        logs(serv, 3) << "generate: player_eliminated" << std::endl;
        auto event = get_new_event_no();

        binary_writer_t writer { config::BUFFER_SIZE };
        writer.write(host_to_net(event));
        writer.write(host_to_net(PLAYER_ELIMINATED));
        writer.write(host_to_net<uint8_t>(get_player_id(player)));

        if(!writer.is_ok()) {
            failure("writing in generate_new_game failed");
        }

        if(encaps_write_event(writer.save()) != 4+4+1+(1)+4) {

        }
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
