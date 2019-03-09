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
        [Client] Send EnterRoom
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
    LeaveRoomResponse = 4,
    Input = 7,
    GameState = 8,
    NewUser = 9,
    OldUser = 10,
};

sf::Packet& operator >> (sf::Packet& p, PacketID& id);
sf::Packet& operator << (sf::Packet& p, PacketID const& id);

struct ChangeUsername {
    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, ChangeUsername& username);
sf::Packet& operator << (sf::Packet& p, ChangeUsername const& username);

struct UsernameResponse {
    enum Result {
        Okay = 0,
        InvalidCharacters = 1,
        TooShort = 2,
        TooLong = 3
    };

    Result result;
    std::vector<std::string> users;
    std::vector<int> rooms;
};

sf::Packet& operator >> (sf::Packet& p, UsernameResponse& username);
sf::Packet& operator << (sf::Packet& p, UsernameResponse const& username);

struct NewUser {
    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, NewUser& username);
sf::Packet& operator << (sf::Packet& p, NewUser const& username);

struct OldUser {
    std::string username;
};

sf::Packet& operator >> (sf::Packet& p, OldUser& username);
sf::Packet& operator << (sf::Packet& p, OldUser const& username);

struct EnterRoom {
    int id;
};

sf::Packet& operator >> (sf::Packet& p, EnterRoom& username);
sf::Packet& operator << (sf::Packet& p, EnterRoom const& username);

struct EnterRoomResponse {

    static int const ERROR_ID = 0;

    int id;
    std::string left_player, right_player;
    std::vector<std::string> spectators;
};

sf::Packet& operator >> (sf::Packet& p, EnterRoomResponse& username);
sf::Packet& operator << (sf::Packet& p, EnterRoomResponse const& username);

struct LeaveRoom {
    int id;
};

sf::Packet& operator >> (sf::Packet& p, LeaveRoom& username);
sf::Packet& operator << (sf::Packet& p, LeaveRoom const& username);

struct LeaveRoomResponse {

    static int const ERROR_ID = 0;

    int id;
    std::vector<std::string> users;
    std::vector<int> rooms;
};

sf::Packet& operator >> (sf::Packet& p, LeaveRoomResponse& username);
sf::Packet& operator << (sf::Packet& p, LeaveRoomResponse const& username);

struct GameState {
    Ball ball;
    Pad left;
    Pad right;
};

sf::Packet& operator >> (sf::Packet& p, GameState& username);
sf::Packet& operator << (sf::Packet& p, GameState const& username);

struct Input {
    ::pong::Input input;
};

sf::Packet& operator >> (sf::Packet& p, Input& username);
sf::Packet& operator << (sf::Packet& p, Input const& username);

}

}