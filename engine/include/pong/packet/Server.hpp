#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <variant>
#include <multipong/Game.hpp>
#include <pong/packet/MakePacket.hpp>
#include <pong/packet/Utility.hpp>

namespace pong::packet::server {

MAKE_PACKET(UsernameResponse) {
    static constexpr char const* name = "UsernameResponse";
    bool valid;
};

MAKE_PACKET(LobbyInfo) {
    static constexpr char const* name = "LobbyInfo";
    std::vector<std::string> users;
    std::vector<int> rooms;
};

MAKE_PACKET(NewUser) {
    static constexpr char const* name = "NewUser";
    std::string username;
};

MAKE_PACKET(OldUser) {
    static constexpr char const* name = "OldUser";
    std::string username;
};

MAKE_PACKET(NewRoom) {
    static constexpr char const* name = "NewRoom";
    unsigned id;
};

MAKE_PACKET(OldRoom) {
    static constexpr char const* name = "OldRoom";
    unsigned id;
};

MAKE_PACKET(EnterRoomResponse) {
    static constexpr char const* name = "EnterRoomResponse";
    enum Result {
        Okay = 0,
        Full = 1,
        InvalidID = 2,
    };

    Result result;
};

MAKE_PACKET(RoomInfo) {
    static constexpr char const* name = "RoomInfo";
    std::string left_player, right_player;
    std::vector<std::string> spectators;
};

MAKE_PACKET(FetchRoomError) {
    static constexpr char const* name = "FetchRoomError";
    unsigned id;
};

MAKE_PACKET(GameState) {
    static constexpr char const* name = "GameState";
    pong::Ball ball;
    pong::Pad left;
    pong::Pad right;
};

MAKE_PACKET(Score) {
    static constexpr char const* name = "Score";
    unsigned left;
    unsigned right;
};

MAKE_PACKET(BePlayer) {
    static constexpr char const* name = "BePlayer";
    pong::Side side;
};

MAKE_PACKET(NewPlayer) {
    static constexpr char const* name = "NewPlayer";
    pong::Side side;
    std::string username;
};

MAKE_PACKET(OldPlayer) {
    static constexpr char const* name = "OldPlayer";
    pong::Side side;
    std::string username;
};

using Any = std::variant<
    UsernameResponse,
    LobbyInfo,
    NewUser,
    OldUser,
    NewRoom,
    OldRoom,
    EnterRoomResponse,
    RoomInfo,
    GameState,
    BePlayer,
    NewPlayer,
    OldPlayer,
    Score,
    FetchRoomError
>;

sf::Packet& operator >> (sf::Packet& p, Any& packet);
sf::Packet& operator << (sf::Packet& p, Any const& packet);
std::ostream& operator <<(std::ostream& os, Any const& packet);
std::string to_string(Any const& packet);

template<typename T>
constexpr id_t id_of() {
    return details::index_of_v<Any, T>;
}

template<typename T>
constexpr id_t id_of(T const&) {
    return id_of<T>();
}

}
