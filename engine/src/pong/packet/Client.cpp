#include <pong/packet/Client.hpp>
#include <pong/packet/Utility.hpp>

namespace pong::packet::client {

/*
    Changeusername

    std::string username
*/

sf::Packet& operator >> (sf::Packet& p, ChangeUsername& packet) {
    return p >> packet.username;
}

sf::Packet& operator << (sf::Packet& p, ChangeUsername const& packet) {
    return p << id_of(packet) << packet.username;
}

bool operator == (ChangeUsername const& lhs, ChangeUsername const& rhs) {
    return lhs.username == rhs.username;
}

std::ostream& operator <<(std::ostream& os, ChangeUsername const& packet) {
    return os << to_string(packet);
}

std::string to_string(ChangeUsername const& packet) {
    return std::string{ packet.name };
}





/*
    CreateRoom
*/

sf::Packet& operator >> (sf::Packet& p, CreateRoom&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, CreateRoom const& packet) {
    return p << id_of(packet);
}

bool operator == (CreateRoom const&, CreateRoom const&) {
    return true;
}

std::ostream& operator <<(std::ostream& os, CreateRoom const& packet) {
    return os << to_string(packet);
}

std::string to_string(CreateRoom const& packet) {
    return std::string{ packet.name };
}





/*
    LeaveRoom
*/

sf::Packet& operator >> (sf::Packet& p, LeaveRoom&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, LeaveRoom const& packet) {
    return p << id_of(packet);
}

bool operator == (LeaveRoom const&, LeaveRoom const&) {
    return true;
}

std::ostream& operator <<(std::ostream& os, LeaveRoom const& packet) {
    return os << to_string(packet);
}

std::string to_string(LeaveRoom const& packet) {
    return std::string{ packet.name };
}





/*
    EnterQueue
*/

sf::Packet& operator >> (sf::Packet& p, EnterQueue&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, EnterQueue const& packet) {
    return p << id_of(packet);
}

bool operator == (EnterQueue const&, EnterQueue const&) {
    return true;
}

std::ostream& operator <<(std::ostream& os, EnterQueue const& packet) {
    return os << to_string(packet);
}

std::string to_string(EnterQueue const& packet) {
    return std::string{ packet.name };
}





/*
    LeaveQueue
*/

sf::Packet& operator >> (sf::Packet& p, LeaveQueue&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, LeaveQueue const& packet) {
    return p << id_of(packet);
}

bool operator == (LeaveQueue const&, LeaveQueue const&) {
    return true;
}

std::ostream& operator <<(std::ostream& os, LeaveQueue const& packet) {
    return os << to_string(packet);
}

std::string to_string(LeaveQueue const& packet) {
    return std::string{ packet.name };
}





/*
    Abandon
*/

sf::Packet& operator >> (sf::Packet& p, Abandon&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, Abandon const& packet) {
    return p << id_of(packet);
}

bool operator == (Abandon const&, Abandon const&) {
    return true;
}

std::ostream& operator <<(std::ostream& os, Abandon const& packet) {
    return os << to_string(packet);
}

std::string to_string(Abandon const& packet) {
    return std::string{ packet.name };
}





/*
    EnterRoom

    unsigned id
*/

sf::Packet& operator >> (sf::Packet& p, EnterRoom& packet) {
    return p >> details::by<sf::Uint32>(packet.id);
}

sf::Packet& operator << (sf::Packet& p, EnterRoom const& packet) {
    return p << id_of(packet) << details::by<sf::Uint32>(packet.id);
}

bool operator == (EnterRoom const& lhs, EnterRoom const& rhs) {
    return lhs.id == rhs.id;
}

std::ostream& operator <<(std::ostream& os, EnterRoom const& packet) {
    return os << to_string(packet);
}

std::string to_string(EnterRoom const& packet) {
    return std::string{ packet.name };
}





/*
    Input

    pong::Input input
*/

sf::Packet& operator >> (sf::Packet& p, Input& packet) {
    return p >> details::by<sf::Uint32>(packet.input);
}

sf::Packet& operator << (sf::Packet& p, Input const& packet) {
    return p << id_of(packet) << details::by<sf::Uint32>(packet.input);
}

bool operator == (Input const& lhs, Input const& rhs) {
    return lhs.input == rhs.input;
}

std::ostream& operator <<(std::ostream& os, Input const& packet) {
    return os << to_string(packet);
}

std::string to_string(Input const& packet) {
    char const* input_str = 
        packet.input == pong::Input::Idle ?
            "Idle"
        :   packet.input == pong::Input::Down ?
            "Down"
        :   "Up";

    return std::string{ packet.name } + input_str;
}





/*
    SubscribeRoomInfo

    unsigned range_min
    unsigned range_max_excluded
*/

sf::Packet& operator >> (sf::Packet& p, SubscribeRoomInfo& packet) {
    return p >> details::by<sf::Uint32>(packet.range_min) >> details::by<sf::Uint32>(packet.range_max_excluded);
}

sf::Packet& operator << (sf::Packet& p, SubscribeRoomInfo const& packet) {
    return p << id_of(packet) << details::by<sf::Uint32>(packet.range_min) << details::by<sf::Uint32>(packet.range_max_excluded);
}

bool operator == (SubscribeRoomInfo const& lhs, SubscribeRoomInfo const& rhs) {
    return lhs.range_min == rhs.range_min && lhs.range_max_excluded == rhs.range_max_excluded;
}

std::ostream& operator <<(std::ostream& os, SubscribeRoomInfo const& packet) {
    return os << to_string(packet);
}

std::string to_string(SubscribeRoomInfo const& packet) {
    return std::string{ packet.name } + "{" + packet.range_min + ".." + packet.range_max_excluded ++ "}";
}





/*
    AcceptBePlayer
*/

sf::Packet& operator >> (sf::Packet& p, AcceptBePlayer&) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, AcceptBePlayer const& packet) {
    return p << id_of(packet);
}

bool operator == (AcceptBePlayer const& lhs, AcceptBePlayer const& rhs) {
    return true;
}

std::ostream& operator <<(std::ostream& os, AcceptBePlayer const& packet) {
    return os << to_string(packet);
}

std::string to_string(AcceptBePlayer const& packet) {
    return std::string{ packet.name };
}





/*
    Any

    std::variant<
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
*/

sf::Packet& operator >> (sf::Packet& p, Any& any_packet) {
    unsigned id;
    p >> id;
    switch(id) {
        case id_of<ChangeUsername>(): {
            ChangeUsername packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<CreateRoom>(): {
            CreateRoom packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<EnterRoom>(): {
            EnterRoom packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<Input>(): {
            Input packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<LeaveRoom>(): {
            LeaveRoom packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<Abandon>(): {
            Abandon packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<EnterQueue>(): {
            EnterQueue packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<LeaveQueue>(): {
            LeaveQueue packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<SubscribeRoomInfo>(): {
            SubscribeRoomInfo packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        case id_of<AcceptBePlayer>(): {
            AcceptBePlayer packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        } 

        default:
            throw std::runtime_error("Bad packet id\n");

    }
}

sf::Packet& operator << (sf::Packet& p, Any const& any_packet) {
    return std::visit([&p] (auto const& packet) -> decltype(auto) { return p << packet; }, any_packet);
}

std::ostream& operator <<(std::ostream& os, Any const& any_packet) {
    return std::visit([&os] (auto const& packet) -> decltype(auto) { return os << packet; }, any_packet);
}

std::string to_string(Any const& any_packet) {
    return std::visit([] (auto const& packet) -> decltype(auto) { return to_string(packet); }, any_packet);
}

}