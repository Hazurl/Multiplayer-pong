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

bool operator==(ChangeUsername const& lhs, ChangeUsername const& rhs) {
    return lhs.username == rhs.username;
}



sf::Packet& operator >> (sf::Packet& p, ChangeUsernameResponse& response) {
    return p >> to_enum(response.result);
}

sf::Packet& operator << (sf::Packet& p, ChangeUsernameResponse const& response) {
    return p << PacketID::ChangeUsernameResponse << from_enum(response.result);
}

bool operator==(ChangeUsernameResponse const& lhs, ChangeUsernameResponse const& rhs) {
    return lhs.result == rhs.result;
}



sf::Packet& operator >> (sf::Packet& p, NewUser& user) {
    return p >> user.username;
}
sf::Packet& operator << (sf::Packet& p, NewUser const& user) {
    return p << PacketID::NewUser << user.username;
}

bool operator==(NewUser const& lhs, NewUser const& rhs) {
    return lhs.username == rhs.username;
}




sf::Packet& operator >> (sf::Packet& p, OldUser& user) {
    return p >> user.username;
}
sf::Packet& operator << (sf::Packet& p, OldUser const& user) {
    return p << PacketID::OldUser << user.username;
}

bool operator==(OldUser const& lhs, OldUser const& rhs) {
    return lhs.username == rhs.username;
}





sf::Packet& operator >> (sf::Packet& p, NewRoom& room) {
    return p >> room.id;
}
sf::Packet& operator << (sf::Packet& p, NewRoom const& room) {
    return p << PacketID::NewRoom << room.id;
}

bool operator==(NewRoom const& lhs, NewRoom const& rhs) {
    return lhs.id == rhs.id;
}




sf::Packet& operator >> (sf::Packet& p, OldRoom& room) {
    return p >> room.id;
}
sf::Packet& operator << (sf::Packet& p, OldRoom const& room) {
    return p << PacketID::OldRoom << room.id;
}

bool operator==(OldRoom const& lhs, OldRoom const& rhs) {
    return lhs.id == rhs.id;
}




sf::Packet& operator >> (sf::Packet& p, EnterRoom& room) {
    return p >> room.id;
}

sf::Packet& operator << (sf::Packet& p, EnterRoom const& room) {
    return p << PacketID::EnterRoom << room.id;
}

bool operator==(EnterRoom const& lhs, EnterRoom const& rhs) {
    return lhs.id == rhs.id;
}



sf::Packet& operator >> (sf::Packet& p, CreateRoom&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, CreateRoom const&) {
    return p << PacketID::CreateRoom;
}

bool operator==(CreateRoom const&, CreateRoom const&) {
    return true;
}




sf::Packet& operator >> (sf::Packet& p, EnterRoomResponse& response) {
    return p >> to_enum(response.result);
}

sf::Packet& operator << (sf::Packet& p, EnterRoomResponse const& response) {
    return p << PacketID::EnterRoomResponse << from_enum(response.result);
}

bool operator==(EnterRoomResponse const& lhs, EnterRoomResponse const& rhs) {
    return lhs.result == rhs.result;
}




sf::Packet& operator >> (sf::Packet& p, RoomInfo& info) {
    return p >> info.left_player >> info.right_player >> info.spectators;
}
sf::Packet& operator << (sf::Packet& p, RoomInfo const& info) {
    return p << PacketID::RoomInfo << info.left_player << info.right_player << info.spectators;
}

bool operator==(RoomInfo const& lhs, RoomInfo const& rhs) {
    return lhs.left_player == rhs.left_player && lhs.right_player == rhs.right_player && lhs.spectators == rhs.spectators;
}




sf::Packet& operator >> (sf::Packet& p, LeaveRoom&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, LeaveRoom const&) {
    return p << PacketID::LeaveRoom;
}

bool operator==(LeaveRoom const&, LeaveRoom const& ) {
    return true;
}



sf::Packet& operator >> (sf::Packet& p, LobbyInfo& response) {
    return p >> response.users >> response.rooms;
}

sf::Packet& operator << (sf::Packet& p, LobbyInfo const& response) {
    return p << PacketID::LobbyInfo << response.users << response.rooms;
}

bool operator==(LobbyInfo const& lhs, LobbyInfo const& rhs) {
    return lhs.users == rhs.users && lhs.rooms == rhs.rooms;
}




sf::Packet& operator >> (sf::Packet& p, GameState& state) {
    return p >> state.ball >> state.left >> state.right;
}

sf::Packet& operator << (sf::Packet& p, GameState const& state) {
    return p << PacketID::GameState << state.ball << state.left << state.right;
}

bool operator==(GameState const& lhs, GameState const& rhs) {
    return lhs.ball == rhs.ball && lhs.left == rhs.left && lhs.right == rhs.right;
}




sf::Packet& operator >> (sf::Packet& p, Input& input) {
    sf::Uint8 i;
    p >> i;
    input.input = static_cast<pong::Input>(i);
    return p;
}

sf::Packet& operator << (sf::Packet& p, Input const& input) {
    return p << PacketID::Input << static_cast<sf::Uint8>(input.input);
}  

bool operator==(Input const& lhs, Input const& rhs) {
    return lhs.input == rhs.input;
}




sf::Packet& operator >> (sf::Packet& p, EnterQueue&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, EnterQueue const&) {
    return p << PacketID::EnterQueue;
}  

bool operator==(EnterQueue const&, EnterQueue const&) {
    return true;
}




sf::Packet& operator >> (sf::Packet& p, LeaveQueue&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, LeaveQueue const&) {
    return p << PacketID::LeaveQueue;
}  

bool operator==(LeaveQueue const&, LeaveQueue const&) {
    return true;
}



sf::Packet& operator >> (sf::Packet& p, Abandon&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, Abandon const&) {
    return p << PacketID::Abandon;
}  

bool operator==(Abandon const&, Abandon const&) {
    return true;
}




sf::Packet& operator >> (sf::Packet& p, BePlayer& be_player) {
    return p >> to_enum(be_player.side);
}

sf::Packet& operator << (sf::Packet& p, BePlayer const& be_player) {
    return p << PacketID::BePlayer << from_enum(be_player.side);
}

bool operator == (BePlayer const& lhs, BePlayer const& rhs) {
    return lhs.side == rhs.side;
}



sf::Packet& operator >> (sf::Packet& p, OldPlayer& old_player) {
    return p >> to_enum(old_player.side) >> old_player.username;
}

sf::Packet& operator << (sf::Packet& p, OldPlayer const& old_player) {
    return p << PacketID::OldPlayer << from_enum(old_player.side) << old_player.username;
}

bool operator == (OldPlayer const& lhs, OldPlayer const& rhs) {
    return lhs.side == rhs.side && lhs.username == rhs.username;
}





sf::Packet& operator >> (sf::Packet& p, NewPlayer& new_player) {
    return p >> to_enum(new_player.side) >> new_player.username;
}

sf::Packet& operator << (sf::Packet& p, NewPlayer const& new_player) {
    return p << PacketID::NewPlayer << from_enum(new_player.side) << new_player.username;
}

bool operator == (NewPlayer const& lhs, NewPlayer const& rhs) {
    return lhs.side == rhs.side && lhs.username == rhs.username;
}




sf::Packet& operator >> (sf::Packet& p, GamePacket& game_packet) {
    PacketID id;
    p >> id;
    switch(id) {
#define process(P) \
        case PacketID::P: { \
            P tmp; \
            p >> tmp; \
            game_packet = std::move(tmp); \
            break; \
        }

        process(ChangeUsername)
        process(GameState)
        process(CreateRoom)
        process(EnterRoom)
        process(EnterRoomResponse)
        process(Input)
        process(LeaveRoom)
        process(LobbyInfo)
        process(NewRoom)
        process(NewUser)
        process(OldRoom)
        process(OldUser)
        process(RoomInfo)
        process(ChangeUsernameResponse)
        process(NewPlayer)
        process(OldPlayer)
        process(BePlayer)
        process(Abandon)
        process(EnterQueue) 
        process(LeaveQueue)

#undef process

        default: {
            throw std::runtime_error("Unknown packet with ID #" + std::to_string(static_cast<std::underlying_type_t<PacketID>>(id)));
        }

    }

    return p;
}

sf::Packet& operator << (sf::Packet& p, GamePacket const& game_packet) {
    return std::visit([&p] (auto const& value) -> decltype(auto) {
        return p << value;
    }, game_packet);
}


}