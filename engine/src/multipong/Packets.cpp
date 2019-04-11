#include <multipong/Packets.hpp>

namespace pong::packet {


template<typename T>
struct ToEnum {
    static_assert(std::is_enum_v<T>, "`to_enum` only works for enums");
    T& t;
};

template<typename T>
ToEnum<T> to_enum(T& t) {
    return ToEnum<T>{ t };
}

template<typename T>
sf::Packet& operator >> (sf::Packet& p, ToEnum<T>&& e) {
    std::underlying_type_t<T> value;
    p >> value;
    e.t = static_cast<T>(value);
    return p;
}





template<typename T>
std::enable_if_t<std::is_enum_v<T>, 
std::underlying_type_t<T>> from_enum(T t) {
    return static_cast<std::underlying_type_t<T>>(t);
}





sf::Packet& operator >> (sf::Packet& p, PacketID& id) {
    return p >> to_enum(id);
}

sf::Packet& operator << (sf::Packet& p, PacketID const& id) {
    return p << from_enum(id);
}




sf::Packet& operator >> (sf::Packet& p, ChangeUsername& username) {
    return p >> username.username;
}

sf::Packet& operator << (sf::Packet& p, ChangeUsername const& username) {
    return p << PacketID::ChangeUsername << username.username;
}



sf::Packet& operator >> (sf::Packet& p, UsernameResponse& response) {
    return p >> to_enum(response.result);
}

sf::Packet& operator << (sf::Packet& p, UsernameResponse const& response) {
    return p << PacketID::UsernameResponse << from_enum(response.result);
}




sf::Packet& operator >> (sf::Packet& p, NewUser& user) {
    return p >> user.username;
}
sf::Packet& operator << (sf::Packet& p, NewUser const& user) {
    return p << PacketID::NewUser << user.username;
}




sf::Packet& operator >> (sf::Packet& p, OldUser& user) {
    return p >> user.username;
}
sf::Packet& operator << (sf::Packet& p, OldUser const& user) {
    return p << PacketID::OldUser << user.username;
}





sf::Packet& operator >> (sf::Packet& p, NewRoom& room) {
    return p >> room.id;
}
sf::Packet& operator << (sf::Packet& p, NewRoom const& room) {
    return p << PacketID::NewRoom << room.id;
}




sf::Packet& operator >> (sf::Packet& p, OldRoom& room) {
    return p >> room.id;
}
sf::Packet& operator << (sf::Packet& p, OldRoom const& room) {
    return p << PacketID::OldRoom << room.id;
}




sf::Packet& operator >> (sf::Packet& p, EnterRoom& room) {
    return p >> room.id;
}

sf::Packet& operator << (sf::Packet& p, EnterRoom const& room) {
    return p << PacketID::EnterRoom << room.id;
}



sf::Packet& operator >> (sf::Packet& p, CreateRoom&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, CreateRoom const&) {
    return p << PacketID::CreateRoom;
}




sf::Packet& operator >> (sf::Packet& p, EnterRoomResponse& response) {
    return p >> to_enum(response.result);
}

sf::Packet& operator << (sf::Packet& p, EnterRoomResponse const& response) {
    return p << PacketID::EnterRoomResponse << from_enum(response.result);
}




sf::Packet& operator >> (sf::Packet& p, RoomInfo& info) {
    return p >> info.left_player >> info.right_player >> info.spectators;
}
sf::Packet& operator << (sf::Packet& p, RoomInfo const& info) {
    return p << PacketID::RoomInfo << info.left_player << info.right_player << info.spectators;
}




sf::Packet& operator >> (sf::Packet& p, LeaveRoom&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, LeaveRoom const&) {
    return p << PacketID::LeaveRoom;
}



sf::Packet& operator >> (sf::Packet& p, LobbyInfo& response) {
    return p >> response.users >> response.rooms;
}

sf::Packet& operator << (sf::Packet& p, LobbyInfo const& response) {
    return p << PacketID::LobbyInfo << response.users << response.rooms;
}




sf::Packet& operator >> (sf::Packet& p, GameState& state) {
    return p >> state.ball >> state.left >> state.right;
}

sf::Packet& operator << (sf::Packet& p, GameState const& state) {
    return p << PacketID::GameState << state.ball << state.left << state.right;
}




sf::Packet& operator >> (sf::Packet& p, Input& input) {
    return p >> to_enum(input.input);
}

sf::Packet& operator << (sf::Packet& p, Input const& input) {
    return p << PacketID::Input << from_enum(input.input);
}  

}