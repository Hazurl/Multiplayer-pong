#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <variant>
#include <multipong/Game.hpp>
#include <pong/packet/MakePacket.hpp>
#include <pong/packet/Utility.hpp>

namespace pong::packet::server {

MAKE_PACKET(ChangeUsernameResponse) {
    static constexpr char const* name = "ChangeUsernameResponse";
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
    enum Result : char {
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

MAKE_PACKET(CreateRoomResponse) {
    static constexpr char const* name = "CreateRoomResponse";
    enum class Reason : char {
        Okay,
        Unknown
    };

    Reason reason;
};

MAKE_PACKET(LeaveRoomResponse) {
    static constexpr char const* name = "LeaveRoomResponse";
    enum class Reason : char {
        Okay,
        Unknown
    };

    Reason reason;
};

MAKE_PACKET(GameOver) {
    static constexpr char const* name = "GameOver";
    enum class Result : char {
        LeftWin,
        RightWin,
        LeftAbandon,
        RightAbandon
    };

    Result result;
};

MAKE_PACKET(BeNextPlayer) {
    static constexpr char const* name = "BeNextPlayer";
};

MAKE_PACKET(DeniedBePlayer) {
    static constexpr char const* name = "DeniedBePlayer";
};

using Any = std::variant<
    ChangeUsernameResponse,
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
    CreateRoomResponse,
    GameOver,
    BeNextPlayer,
    DeniedBePlayer,
    LeaveRoomResponse
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
