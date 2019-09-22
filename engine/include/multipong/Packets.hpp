#pragma once

#include <SFML/Network.hpp>

#include <multipong/Game.hpp>

#include <string>
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
        [SERVER] Send UsernameResponse
    */
    InvalidUser,

    /*
        [Client] Send EnterRoom, CreateRoom
        [SERVER] Send EnterRoomResponse, NewUser, OldUser
    */
    ValidUser,

    /*
        [CLIENT] Send LeaveRoom, EnterQueue, LeaveQueue
        [SERVER] Send GameState, NewSpectator, NewPlayer, LeaveRoomResponse, EnterQueueResponse, LeaveQueueResponse
    */
    Spectator,

    /*
        [CLIENT] Send Input
        [SERVER] Send GameState, NewSpectator, NewPlayer 
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
    LeaveRoom = 3,
    Input = 7,
    GameState = 8,
    NewUser = 9,
    OldUser = 10,
    CreateRoom = 11,
    NewRoom = 12,
    OldRoom = 13,
    LobbyInfo = 14,
    RoomInfo = 15,
};

sf::Packet& operator >> (sf::Packet& p, PacketID& id);
sf::Packet& operator << (sf::Packet& p, PacketID const& id);

struct ChangeUsername {
    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, ChangeUsername& username);
sf::Packet& operator << (sf::Packet& p, ChangeUsername const& username);
bool operator == (ChangeUsername const& lhs, ChangeUsername const& rhs);

struct UsernameResponse {
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
    std::vector<std::string> users;
    std::vector<int> rooms;
};

sf::Packet& operator >> (sf::Packet& p, LobbyInfo& lobby);
sf::Packet& operator << (sf::Packet& p, LobbyInfo const& lobby);
bool operator == (LobbyInfo const& lhs, LobbyInfo const& rhs);

struct NewUser {
    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, NewUser& username);
sf::Packet& operator << (sf::Packet& p, NewUser const& username);
bool operator == (NewUser const& lhs, NewUser const& rhs);

struct OldUser {
    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, OldUser& username);
sf::Packet& operator << (sf::Packet& p, OldUser const& username);
bool operator == (OldUser const& lhs, OldUser const& rhs);

struct NewRoom {
    int id;
};

sf::Packet& operator >> (sf::Packet& p, NewRoom& username);
sf::Packet& operator << (sf::Packet& p, NewRoom const& username);
bool operator == (NewRoom const& lhs, NewRoom const& rhs);

struct OldRoom {
    int id;
};

sf::Packet& operator >> (sf::Packet& p, OldRoom& username);
sf::Packet& operator << (sf::Packet& p, OldRoom const& username);
bool operator == (OldRoom const& lhs, OldRoom const& rhs);

struct EnterRoom {
    int id;
};

sf::Packet& operator >> (sf::Packet& p, EnterRoom& username);
sf::Packet& operator << (sf::Packet& p, EnterRoom const& username);
bool operator == (EnterRoom const& lhs, EnterRoom const& rhs);

struct CreateRoom {};

sf::Packet& operator >> (sf::Packet& p, CreateRoom& username);
sf::Packet& operator << (sf::Packet& p, CreateRoom const& username);
bool operator == (CreateRoom const& lhs, CreateRoom const& rhs);

struct EnterRoomResponse {
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
    std::string left_player, right_player;
    std::vector<std::string> spectators;
};

sf::Packet& operator >> (sf::Packet& p, RoomInfo& username);
sf::Packet& operator << (sf::Packet& p, RoomInfo const& username);
bool operator == (RoomInfo const& lhs, RoomInfo const& rhs);

struct LeaveRoom {};

sf::Packet& operator >> (sf::Packet& p, LeaveRoom& username);
sf::Packet& operator << (sf::Packet& p, LeaveRoom const& username);
bool operator == (LeaveRoom const& lhs, LeaveRoom const& rhs);


struct GameState {
    Ball ball;
    Pad left;
    Pad right;
};

sf::Packet& operator >> (sf::Packet& p, GameState& username);
sf::Packet& operator << (sf::Packet& p, GameState const& username);
bool operator == (GameState const& lhs, GameState const& rhs);

struct Input {
    ::pong::Input input;
};

sf::Packet& operator >> (sf::Packet& p, Input& username);
sf::Packet& operator << (sf::Packet& p, Input const& username);
bool operator == (Input const& lhs, Input const& rhs);

}

}