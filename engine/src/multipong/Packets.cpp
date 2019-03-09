#include <multipong/Packets.hpp>

namespace pong::packet {



sf::Packet& operator >> (sf::Packet& p, PacketID& id) {
    int i;
    p >> i;
    id = static_cast<PacketID>(i);
    return p;
}

sf::Packet& operator << (sf::Packet& p, PacketID const& id) {
    return p << static_cast<int>(id);
}




sf::Packet& operator >> (sf::Packet& p, ChangeUsername& username) {
    return p >> username.username;
}

sf::Packet& operator << (sf::Packet& p, ChangeUsername const& username) {
    return p << static_cast<int>(PacketID::ChangeUsername) << username.username;
}



sf::Packet& operator >> (sf::Packet& p, UsernameResponse& response) {
    int i;
    p >> i >> response.users >> response.rooms;
    response.result = static_cast<decltype(response.result)>(i);
    return p;
}

sf::Packet& operator << (sf::Packet& p, UsernameResponse const& response) {
    return p << static_cast<int>(PacketID::UsernameResponse) << response.result << response.users << response.rooms;
}




sf::Packet& operator >> (sf::Packet& p, NewUser& user) {
    return p >> user.username;
}
sf::Packet& operator << (sf::Packet& p, NewUser const& user) {
    return p << static_cast<int>(PacketID::NewUser) << user.username;
}




sf::Packet& operator >> (sf::Packet& p, OldUser& user) {
    return p >> user.username;
}
sf::Packet& operator << (sf::Packet& p, OldUser const& user) {
    return p << static_cast<int>(PacketID::OldUser) << user.username;
}




sf::Packet& operator >> (sf::Packet& p, EnterRoom& room) {
    return p >> room.id;
}

sf::Packet& operator << (sf::Packet& p, EnterRoom const& room) {
    return p << static_cast<int>(PacketID::EnterRoom) << room.id;
}



sf::Packet& operator >> (sf::Packet& p, EnterRoomResponse& response) {
    return p >> response.id >> response.left_player >> response.right_player >> response.spectators;
}

sf::Packet& operator << (sf::Packet& p, EnterRoomResponse const& response) {
    return p << static_cast<int>(PacketID::EnterRoomResponse) << response.id << response.left_player << response.right_player << response.spectators;
}




sf::Packet& operator >> (sf::Packet& p, LeaveRoom& room) {
    return p >> room.id;
}

sf::Packet& operator << (sf::Packet& p, LeaveRoom const& room) {
    return p << static_cast<int>(PacketID::LeaveRoom) << room.id;
}




sf::Packet& operator >> (sf::Packet& p, LeaveRoomResponse& response) {
    return p >> response.id >> response.users >> response.rooms;
}

sf::Packet& operator << (sf::Packet& p, LeaveRoomResponse const& response) {
    return p << static_cast<int>(PacketID::LeaveRoomResponse) << response.id << response.users << response.rooms;
}




sf::Packet& operator >> (sf::Packet& p, GameState& state) {
    return p >> state.ball >> state.left >> state.right;
}

sf::Packet& operator << (sf::Packet& p, GameState const& state) {
    return p << static_cast<int>(PacketID::GameState) << state.ball << state.left << state.right;
}




sf::Packet& operator >> (sf::Packet& p, Input& input) {
    int i;
    p >> i;
    input.input = static_cast<::pong::Input>(i);
    return p; 
}

sf::Packet& operator << (sf::Packet& p, Input const& input) {
    return p << static_cast<int>(PacketID::Input) << static_cast<int>(input.input);
}  

}