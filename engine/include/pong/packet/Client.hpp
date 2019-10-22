#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <variant>
#include <multipong/Game.hpp>
#include <pong/packet/MakePacket.hpp>
#include <pong/packet/Utility.hpp>

namespace pong::packet::client {

MAKE_PACKET(ChangeUsername) {
    static constexpr char const* name = "ChangeUsername";
    std::string username;
};

MAKE_PACKET(CreateRoom) {
    static constexpr char const* name = "CreateRoom";
};

MAKE_PACKET(LeaveRoom) {
    static constexpr char const* name = "LeaveRoom";
};

MAKE_PACKET(EnterQueue) {
    static constexpr char const* name = "EnterQueue";
};

MAKE_PACKET(LeaveQueue) {
    static constexpr char const* name = "LeaveQueue";
};

MAKE_PACKET(Abandon) {
    static constexpr char const* name = "Abandon";
};

MAKE_PACKET(EnterRoom) {
    static constexpr char const* name = "EnterRoom";
    unsigned id;
};

MAKE_PACKET(Input) {
    static constexpr char const* name = "Input";
    ::pong::Input input;
};

MAKE_PACKET(SubscribeRoomInfo) {
    static constexpr char const* name = "SubscribeRoomInfo";
    unsigned range_min;
    unsigned range_max_excluded;
};

MAKE_PACKET(AcceptBePlayer) {
    static constexpr char const* name = "AcceptBePlayer";
};

using Any = std::variant<
    ChangeUsername,
    CreateRoom,
    EnterRoom,
    Input,
    LeaveRoom,
    Abandon,
    EnterQueue, 
    LeaveQueue,
    SubscribeRoomInfo,
    AcceptBePlayer
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