#include <pong/packet/Server.hpp>
#include <pong/packet/Utility.hpp>

namespace pong::packet::server {

/*
    UsernameResponse

    Result result
*/

sf::Packet& operator >> (sf::Packet& p, UsernameResponse& packet) {
    return p >> details::by<sf::Uint8>(packet.valid);
}

sf::Packet& operator << (sf::Packet& p, UsernameResponse const& packet) {
    return p << id_of(packet) << details::by<sf::Uint8>(packet.valid);
}

bool operator == (UsernameResponse const& lhs, UsernameResponse const& rhs) {
    return lhs.valid == rhs.valid;
}

std::ostream& operator <<(std::ostream& os, UsernameResponse const& packet) {
    return os << to_string(packet);
}

std::string to_string(UsernameResponse const& packet) {
    char const* result_str = packet.valid ? "Valid" : "Invalid";
    return std::string{ packet.name } + "::" + result_str;
}





/*
    LobbyInfo

    std::vector<std::string> users
    std::vector<int> rooms
*/

sf::Packet& operator >> (sf::Packet& p, LobbyInfo& packet) {
    using details::operator>>;
    return p >> packet.users >> packet.rooms;
}

sf::Packet& operator << (sf::Packet& p, LobbyInfo const& packet) {
    using details::operator<<;
    return p << id_of(packet) << packet.users << packet.rooms;
}

bool operator == (LobbyInfo const& lhs, LobbyInfo const& rhs) {
    return lhs.users == rhs.users && lhs.rooms == rhs.rooms;
}

std::ostream& operator <<(std::ostream& os, LobbyInfo const& packet) {
    return os << to_string(packet);
}

std::string to_string(LobbyInfo const& packet) {
    auto str = std::string{ packet.name } + "{[";
    bool first = true;
    for(auto const& user : packet.users) {
        if (!first) {
            str += ", ";            
        }
        first = false;
        
        str += user;
    }

    str += "], [";

    first = true;
    for(auto const& room : packet.rooms) {
        if (!first) {
            str += ", ";            
        }
        first = false;
        
        str += std::to_string(room);
    }

    str += "]}";
    return str;
}





/*
    NewUser

    std::string username
*/

sf::Packet& operator >> (sf::Packet& p, NewUser& packet) {
    return p >> packet.username;
}

sf::Packet& operator << (sf::Packet& p, NewUser const& packet) {
    return p << id_of(packet) << packet.username;
}

bool operator == (NewUser const& lhs, NewUser const& rhs) {
    return lhs.username == rhs.username;
}

std::ostream& operator <<(std::ostream& os, NewUser const& packet) {
    return os << to_string(packet);
}

std::string to_string(NewUser const& packet) {
    return std::string{ packet.name } + "{" + packet.username + "}";
}





/*
    OldUser

    std::string username
*/

sf::Packet& operator >> (sf::Packet& p, OldUser& packet) {
    return p >> packet.username;
}

sf::Packet& operator << (sf::Packet& p, OldUser const& packet) {
    return p << id_of(packet) << packet.username;
}

bool operator == (OldUser const& lhs, OldUser const& rhs) {
    return lhs.username == rhs.username;
}

std::ostream& operator <<(std::ostream& os, OldUser const& packet) {
    return os << to_string(packet);
}

std::string to_string(OldUser const& packet) {
    return std::string{ packet.name } + "{" + packet.username + "}";
}





/*
    NewRoom

    unsigned id
*/

sf::Packet& operator >> (sf::Packet& p, NewRoom& packet) {
    return p >> details::by<sf::Uint32>(packet.id);
}

sf::Packet& operator << (sf::Packet& p, NewRoom const& packet) {
    return p << id_of(packet) << details::by<sf::Uint32>(packet.id);
}

bool operator == (NewRoom const& lhs, NewRoom const& rhs) {
    return lhs.id == rhs.id;
}

std::ostream& operator <<(std::ostream& os, NewRoom const& packet) {
    return os << to_string(packet);
}

std::string to_string(NewRoom const& packet) {
    return std::string{ packet.name } + "#" + std::to_string(packet.id);
}





/*
    OldRoom

    unsigned id
*/

sf::Packet& operator >> (sf::Packet& p, OldRoom& packet) {
    return p >> details::by<sf::Uint32>(packet.id);
}

sf::Packet& operator << (sf::Packet& p, OldRoom const& packet) {
    return p << id_of(packet) << details::by<sf::Uint32>(packet.id);
}

bool operator == (OldRoom const& lhs, OldRoom const& rhs) {
    return lhs.id == rhs.id;
}

std::ostream& operator <<(std::ostream& os, OldRoom const& packet) {
    return os << to_string(packet);
}

std::string to_string(OldRoom const& packet) {
    return std::string{ packet.name } + "#" + std::to_string(packet.id);
}





/*
    EnterRoomResponse

    Result result
*/

sf::Packet& operator >> (sf::Packet& p, EnterRoomResponse& packet) {
    return p >> details::by<sf::Uint32>(packet.result);
}

sf::Packet& operator << (sf::Packet& p, EnterRoomResponse const& packet) {
    return p << id_of(packet) << details::by<sf::Uint32>(packet.result);
}

bool operator == (EnterRoomResponse const& lhs, EnterRoomResponse const& rhs) {
    return lhs.result == rhs.result;
}

std::ostream& operator <<(std::ostream& os, EnterRoomResponse const& packet) {
    return os << to_string(packet);
}

std::string to_string(EnterRoomResponse const& packet) {
    char const* result_str = 
        packet.result == EnterRoomResponse::Okay ?
            "Okay"
        :   packet.result == EnterRoomResponse::Full ?
            "Full"
        :   "InvalidID";
    return std::string{ packet.name } + "::" + result_str;
}





/*
    RoomInfo

    std::string left_player
    std::string right_player
    std::vector<std::string> spectators
*/

sf::Packet& operator >> (sf::Packet& p, RoomInfo& packet) {
    using details::operator>>;
    return p >> packet.left_player >> packet.right_player >> packet.spectators;
}

sf::Packet& operator << (sf::Packet& p, RoomInfo const& packet) {
    using details::operator<<;
    return p << id_of(packet) << packet.left_player << packet.right_player << packet.spectators;
}

bool operator == (RoomInfo const& lhs, RoomInfo const& rhs) {
    return 
        lhs.left_player == rhs.left_player 
    &&  lhs.right_player == rhs.right_player
    &&  lhs.spectators == rhs.spectators;
}

std::ostream& operator <<(std::ostream& os, RoomInfo const& packet) {
    return os << to_string(packet);
}

std::string to_string(RoomInfo const& packet) {
    auto str = std::string{ packet.name } + "{";
    str +=  packet.left_player + " vs " + packet.right_player + ", [";
    bool first = true;
    for(auto const& user : packet.spectators) {
        if (!first) {
            str += ", ";            
        }
        first = false;
        
        str += user;
    }
    str += "]}";
    return str;
}





/*
    GameState

    pong::Ball ball
    pong::Pad left
    pong::Pad right
*/

sf::Packet& operator >> (sf::Packet& p, GameState& packet) {
    using details::operator>>;
    return p >> packet.ball >> packet.left >> packet.right;
}

sf::Packet& operator << (sf::Packet& p, GameState const& packet) {
    using details::operator<<;
    return p << id_of(packet) << packet.ball << packet.left << packet.right;
}

bool operator == (GameState const& lhs, GameState const& rhs) {
    return 
        lhs.ball == rhs.ball 
    &&  lhs.left == rhs.left
    &&  lhs.right == rhs.right;
}

std::ostream& operator <<(std::ostream& os, GameState const& packet) {
    return os << to_string(packet);
}

std::string to_string(GameState const& packet) {
    auto const vector_to_string = [] (sf::Vector2f const& v) {
        return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
    };

    auto const pad_to_string = [] (pong::Pad const& pad) {
        return "pad{y:" + std::to_string(pad.y) + ", speed:" + std::to_string(pad.speed) + "}";
    };

    auto str = std::string{ packet.name } + "{";
    str += "ball{position:" + vector_to_string(packet.ball.position);
    str += ", speed:" + vector_to_string(packet.ball.speed) + "}, ";
    str += pad_to_string(packet.left) + ", ";
    str += pad_to_string(packet.right) + "}";
    return str;
}





/*
    BePlayer

    pong::Side side
*/

sf::Packet& operator >> (sf::Packet& p, BePlayer& packet) {
    using details::operator>>;
    return p >> details::by<sf::Uint32>(packet.side);
}

sf::Packet& operator << (sf::Packet& p, BePlayer const& packet) {
    using details::operator<<;
    return p << id_of(packet) << details::by<sf::Uint32>(packet.side);
}

bool operator == (BePlayer const& lhs, BePlayer const& rhs) {
    return lhs.side == rhs.side;
}

std::ostream& operator <<(std::ostream& os, BePlayer const& packet) {
    return os << to_string(packet);
}

std::string to_string(BePlayer const& packet) {
    char const* side_str = packet.side == pong::Side::Left ?
            "Left"
        :   "Right";
    return std::string{ packet.name } + "::" + side_str;
}





/*
    NewPlayer

    pong::Side side
    std::string username
*/

sf::Packet& operator >> (sf::Packet& p, NewPlayer& packet) {
    using details::operator>>;
    return p >> details::by<sf::Uint32>(packet.side) >> packet.username;
}

sf::Packet& operator << (sf::Packet& p, NewPlayer const& packet) {
    using details::operator<<;
    return p << id_of(packet) << details::by<sf::Uint32>(packet.side) << packet.username;
}

bool operator == (NewPlayer const& lhs, NewPlayer const& rhs) {
    return lhs.side == rhs.side && lhs.username == rhs.username;
}

std::ostream& operator <<(std::ostream& os, NewPlayer const& packet) {
    return os << to_string(packet);
}

std::string to_string(NewPlayer const& packet) {
    char const* side_str = packet.side == pong::Side::Left ?
            "Left"
        :   "Right";
    return std::string{ packet.name } + "{" + packet.username + ", " + side_str + "}";
}





/*
    OldPlayer

    pong::Side side
    std::string username
*/

sf::Packet& operator >> (sf::Packet& p, OldPlayer& packet) {
    using details::operator>>;
    return p >> details::by<sf::Uint32>(packet.side) >> packet.username;
}

sf::Packet& operator << (sf::Packet& p, OldPlayer const& packet) {
    using details::operator<<;
    return p << id_of(packet) << details::by<sf::Uint32>(packet.side) << packet.username;
}

bool operator == (OldPlayer const& lhs, OldPlayer const& rhs) {
    return lhs.side == rhs.side && lhs.username == rhs.username;
}

std::ostream& operator <<(std::ostream& os, OldPlayer const& packet) {
    return os << to_string(packet);
}

std::string to_string(OldPlayer const& packet) {
    char const* side_str = packet.side == pong::Side::Left ?
            "Left"
        :   "Right";
    return std::string{ packet.name } + "{" + packet.username + ", " + side_str + "}";
}





/*
    CreateRoomResponse

    Reason reason
*/

sf::Packet& operator >> (sf::Packet& p, CreateRoomResponse& packet) {
    return p >> details::by<sf::Uint8>(packet.reason);
}

sf::Packet& operator << (sf::Packet& p, CreateRoomResponse const& packet) {
    return p << id_of(packet) << details::by<sf::Uint8>(packet.reason);
}

bool operator == (CreateRoomResponse const& lhs, CreateRoomResponse const& rhs) {
    return lhs.reason == rhs.reason;
}

std::ostream& operator <<(std::ostream& os, CreateRoomResponse const& packet) {
    return os << to_string(packet);
}

std::string to_string(CreateRoomResponse const& packet) {
    char const* reason_str = packet.reason == CreateRoomResponse::Reason::Okay ?
            "Okay"
        :   "Unknown";
    return std::string{ packet.name } + "::" + reason_str;
}





/*
    GameOver

    Result result
*/

sf::Packet& operator >> (sf::Packet& p, GameOver& packet) {
    return p >> details::by<sf::Uint8>(packet.result);
}

sf::Packet& operator << (sf::Packet& p, GameOver const& packet) {
    return p << id_of(packet) << details::by<sf::Uint8>(packet.result);
}

bool operator == (GameOver const& lhs, GameOver const& rhs) {
    return lhs.result == rhs.result;
}

std::ostream& operator <<(std::ostream& os, GameOver const& packet) {
    return os << to_string(packet);
}

std::string to_string(GameOver const& packet) {
    char const* result_str = 
        packet.result == GameOver::Result::LeftWin ?
            "Okay"
        :   packet.result == GameOver::Result::RightWin ?
            "RightWin"
        :   packet.result == GameOver::Result::LeftAbandon ?
            "LeftAbandon"
        :   "RightAbandon";
    return std::string{ packet.name } + "::" + result_str;
}





/*
    BeNextPlayer
*/

sf::Packet& operator >> (sf::Packet& p, BeNextPlayer& packet) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, BeNextPlayer const& packet) {
    return p << id_of(packet);
}

bool operator == (BeNextPlayer const& lhs, BeNextPlayer const& rhs) {
    return true;
}

std::ostream& operator <<(std::ostream& os, BeNextPlayer const& packet) {
    return os << to_string(packet);
}

std::string to_string(BeNextPlayer const& packet) {
    return std::string{ packet.name };
}





/*
    DeniedBePlayer
*/

sf::Packet& operator >> (sf::Packet& p, BeNextPlayer& packet) {
    return p;
}

sf::Packet& operator << (sf::Packet& p, BeNextPlayer const& packet) {
    return p << id_of(packet);
}

bool operator == (BeNextPlayer const& lhs, BeNextPlayer const& rhs) {
    return true;
}

std::ostream& operator <<(std::ostream& os, BeNextPlayer const& packet) {
    return os << to_string(packet);
}

std::string to_string(BeNextPlayer const& packet) {
    return std::string{ packet.name };
}





/*
    Score

    unsigned left
    unsigned right
*/

sf::Packet& operator >> (sf::Packet& p, Score& packet) {
    return p >> details::by<sf::Uint32>(packet.left) >> details::by<sf::Uint32>(packet.right);
}

sf::Packet& operator << (sf::Packet& p, Score const& packet) {
    return p << id_of(packet) << details::by<sf::Uint32>(packet.left) << details::by<sf::Uint32>(packet.right);
}

bool operator == (Score const& lhs, Score const& rhs) {
    return lhs.left == rhs.left && lhs.right == rhs.right;
}

std::ostream& operator <<(std::ostream& os, Score const& packet) {
    return os << to_string(packet);
}

std::string to_string(Score const& packet) {
    return std::string{ packet.name } + "{" + std::to_string(packet.left) + ":" + std::to_string(packet.right) + "}";
}





/*
    Any

    std::variant<
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
        CreateRoomResponse,
        GameOver,
        BeNextPlayer,
        DeniedBePlayer
    >;
*/

sf::Packet& operator >> (sf::Packet& p, Any& any_packet) {
    unsigned id;
    p >> id;
    switch(id) {
        case id_of<UsernameResponse>(): {
            UsernameResponse packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<LobbyInfo>(): {
            LobbyInfo packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<NewUser>(): {
            NewUser packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<OldUser>(): {
            OldUser packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<NewRoom>(): {
            NewRoom packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<OldRoom>(): {
            OldRoom packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<EnterRoomResponse>(): {
            EnterRoomResponse packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<RoomInfo>(): {
            RoomInfo packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<GameState>(): {
            GameState packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<BePlayer>(): {
            BePlayer packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<NewPlayer>(): {
            NewPlayer packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<OldPlayer>(): {
            OldPlayer packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<Score>(): {
            Score packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<CreateRoomResponse>(): {
            CreateRoomResponse packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<GameOver>(): {
            GameOver packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<BeNextPlayer>(): {
            BeNextPlayer packet;
            p >> packet;
            any_packet = std::move(packet);
            return p;
        }

        case id_of<DeniedBePlayer>(): {
            DeniedBePlayer packet;
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