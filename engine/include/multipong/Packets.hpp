#pragma once

#include <SFML/Network.hpp>

#include <multipong/Game.hpp>

#include <string>
#include <variant>
#include <vector>

namespace pong {

enum class State {

    /*
        [CLIENT] Not connected to the server
    */
    Offline,

    /*
        [CLIENT] Waiting for the socket to get connected to the server
    */
    Connecting,

    /*
        [CLIENT] Send ChangeUsername
        [SERVER] Send UsernameResponse, LobbyInfo
    */
    InvalidUser,

    /*
        [Client] Send EnterRoom, CreateRoom
        [SERVER] Send EnterRoomResponse, RoomInfo, NewUser, OldUser
    */
    ValidUser,

    /*
        [CLIENT] Send LeaveRoom, EnterQueue, LeaveQueue
        [SERVER] Send GameState, NewUser, OldUser, NewPlayer, OldPlayer, BePlayer 
    */
    Spectator,

    /*
        [CLIENT] Send Input, Abandon
        [SERVER] Send GameState, NewUser, OldUser, NewPlayer, OldPlayer 
    */
    Player,

};


namespace packet {

template<typename T>
sf::Packet& operator << (sf::Packet& p, std::vector<T> const& v) {
    p << static_cast<int>(v.size());
    for(auto const& s : v) {
        p << s;
    }

    return p;
}

template<typename T>
sf::Packet& operator >> (sf::Packet& p, std::vector<T>& v) {
    int size{ 0 };
    p >> size;

    v.reserve(size);
    for(; size > 0; --size) {
        p >> v.emplace_back();
    }

    return p;
}

enum class PacketID {
    ChangeUsername = 0,
    UsernameResponse = 1,
    EnterRoom = 2,
    EnterRoomResponse = 3,
    LeaveRoom = 4,
    Input = 7,
    GameState = 8,
    NewUser = 9,
    OldUser = 10,
    CreateRoom = 11,
    NewRoom = 12,
    OldRoom = 13,
    LobbyInfo = 14,
    RoomInfo = 15,
    NewPlayer = 16,
    OldPlayer = 17,
    BePlayer = 18,
    Abandon = 19,
    EnterQueue = 20, 
    LeaveQueue = 21
};

sf::Packet& operator >> (sf::Packet& p, PacketID& id);
sf::Packet& operator << (sf::Packet& p, PacketID const& id);

struct ChangeUsername {
    static constexpr PacketID packet_id = PacketID::ChangeUsername;

    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, ChangeUsername& username);
sf::Packet& operator << (sf::Packet& p, ChangeUsername const& username);
bool operator == (ChangeUsername const& lhs, ChangeUsername const& rhs);

struct UsernameResponse {
    static constexpr PacketID packet_id = PacketID::UsernameResponse;

    enum Result {
        Okay = 0,
        InvalidCharacters = 1,
        TooShort = 2,
        TooLong = 3
    };

    Result result;
};

sf::Packet& operator >> (sf::Packet& p, UsernameResponse& username);
sf::Packet& operator << (sf::Packet& p, UsernameResponse const& username);
bool operator == (UsernameResponse const& lhs, UsernameResponse const& rhs);

struct LobbyInfo {
    static constexpr PacketID packet_id = PacketID::LobbyInfo;

    std::vector<std::string> users;
    std::vector<int> rooms;
};

sf::Packet& operator >> (sf::Packet& p, LobbyInfo& lobby);
sf::Packet& operator << (sf::Packet& p, LobbyInfo const& lobby);
bool operator == (LobbyInfo const& lhs, LobbyInfo const& rhs);

struct NewUser {
    static constexpr PacketID packet_id = PacketID::NewUser;

    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, NewUser& username);
sf::Packet& operator << (sf::Packet& p, NewUser const& username);
bool operator == (NewUser const& lhs, NewUser const& rhs);

struct OldUser {
    static constexpr PacketID packet_id = PacketID::OldUser;

    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, OldUser& username);
sf::Packet& operator << (sf::Packet& p, OldUser const& username);
bool operator == (OldUser const& lhs, OldUser const& rhs);

struct NewRoom {
    static constexpr PacketID packet_id = PacketID::NewRoom;

    int id;
};

sf::Packet& operator >> (sf::Packet& p, NewRoom& username);
sf::Packet& operator << (sf::Packet& p, NewRoom const& username);
bool operator == (NewRoom const& lhs, NewRoom const& rhs);

struct OldRoom {
    static constexpr PacketID packet_id = PacketID::OldRoom;

    int id;
};

sf::Packet& operator >> (sf::Packet& p, OldRoom& username);
sf::Packet& operator << (sf::Packet& p, OldRoom const& username);
bool operator == (OldRoom const& lhs, OldRoom const& rhs);

struct EnterRoom {
    static constexpr PacketID packet_id = PacketID::EnterRoom;

    int id;
};

sf::Packet& operator >> (sf::Packet& p, EnterRoom& username);
sf::Packet& operator << (sf::Packet& p, EnterRoom const& username);
bool operator == (EnterRoom const& lhs, EnterRoom const& rhs);

struct CreateRoom {
    static constexpr PacketID packet_id = PacketID::CreateRoom;
};

sf::Packet& operator >> (sf::Packet& p, CreateRoom& username);
sf::Packet& operator << (sf::Packet& p, CreateRoom const& username);
bool operator == (CreateRoom const& lhs, CreateRoom const& rhs);

struct EnterRoomResponse {
    static constexpr PacketID packet_id = PacketID::EnterRoomResponse;

    enum Result {
        Okay = 0,
        Full = 1,
        InvalidID = 2,
    };

    Result result;
};

sf::Packet& operator >> (sf::Packet& p, EnterRoomResponse& username);
sf::Packet& operator << (sf::Packet& p, EnterRoomResponse const& username);
bool operator == (EnterRoomResponse const& lhs, EnterRoomResponse const& rhs);

struct RoomInfo {
    static constexpr PacketID packet_id = PacketID::RoomInfo;

    std::string left_player, right_player;
    std::vector<std::string> spectators;
};

sf::Packet& operator >> (sf::Packet& p, RoomInfo& username);
sf::Packet& operator << (sf::Packet& p, RoomInfo const& username);
bool operator == (RoomInfo const& lhs, RoomInfo const& rhs);

struct LeaveRoom {
    static constexpr PacketID packet_id = PacketID::LeaveRoom;
};

sf::Packet& operator >> (sf::Packet& p, LeaveRoom& username);
sf::Packet& operator << (sf::Packet& p, LeaveRoom const& username);
bool operator == (LeaveRoom const& lhs, LeaveRoom const& rhs);


struct GameState {
    static constexpr PacketID packet_id = PacketID::GameState;

    Ball ball;
    Pad left;
    Pad right;
};

sf::Packet& operator >> (sf::Packet& p, GameState& username);
sf::Packet& operator << (sf::Packet& p, GameState const& username);
bool operator == (GameState const& lhs, GameState const& rhs);

struct Input {
    static constexpr PacketID packet_id = PacketID::Input;

    ::pong::Input input;
};

sf::Packet& operator >> (sf::Packet& p, Input& username);
sf::Packet& operator << (sf::Packet& p, Input const& username);
bool operator == (Input const& lhs, Input const& rhs);

struct EnterQueue {
    static constexpr PacketID packet_id = PacketID::EnterQueue;
};

sf::Packet& operator >> (sf::Packet& p, EnterQueue& username);
sf::Packet& operator << (sf::Packet& p, EnterQueue const& username);
bool operator == (EnterQueue const& lhs, EnterQueue const& rhs);

struct LeaveQueue {
    static constexpr PacketID packet_id = PacketID::LeaveQueue;
};

sf::Packet& operator >> (sf::Packet& p, LeaveQueue& username);
sf::Packet& operator << (sf::Packet& p, LeaveQueue const& username);
bool operator == (LeaveQueue const& lhs, LeaveQueue const& rhs);

struct Abandon {
    static constexpr PacketID packet_id = PacketID::Abandon;
};

sf::Packet& operator >> (sf::Packet& p, Abandon& username);
sf::Packet& operator << (sf::Packet& p, Abandon const& username);
bool operator == (Abandon const& lhs, Abandon const& rhs);

struct BePlayer {
    static constexpr PacketID packet_id = PacketID::BePlayer;

    enum class Side {
        Left, Right
    };

    Side side;
};

sf::Packet& operator >> (sf::Packet& p, BePlayer& username);
sf::Packet& operator << (sf::Packet& p, BePlayer const& username);
bool operator == (BePlayer const& lhs, BePlayer const& rhs);

struct OldPlayer {
    static constexpr PacketID packet_id = PacketID::OldPlayer;

    enum class Side {
        Left, Right
    };

    Side side;
    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, OldPlayer& username);
sf::Packet& operator << (sf::Packet& p, OldPlayer const& username);
bool operator == (OldPlayer const& lhs, OldPlayer const& rhs);

struct NewPlayer {
    static constexpr PacketID packet_id = PacketID::NewPlayer;

    enum class Side {
        Left, Right
    };

    Side side;
    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, NewPlayer& username);
sf::Packet& operator << (sf::Packet& p, NewPlayer const& username);
bool operator == (NewPlayer const& lhs, NewPlayer const& rhs);

using GamePacket = std::variant<
    ChangeUsername,
    GameState,
    CreateRoom,
    EnterRoom,
    EnterRoomResponse,
    Input,
    LeaveRoom,
    LobbyInfo,
    NewRoom,
    NewUser,
    OldRoom,
    OldUser,
    RoomInfo,
    UsernameResponse,
    NewPlayer,
    OldPlayer,
    BePlayer,
    Abandon,
    EnterQueue, 
    LeaveQueue
>;

sf::Packet& operator >> (sf::Packet& p, GamePacket& game_packet);
sf::Packet& operator << (sf::Packet& p, GamePacket const& game_packet);


}

}