#include <dbg_pckt/PacketToString.hpp>

namespace dbg_pckt {

std::string float_to_string(float f) {
    auto str = std::to_string(f);
    if (str.find('.') != std::string::npos) {
        str.erase(std::begin(str) + (str.find_last_not_of('0') + 1), std::end(str));

        if (str.back() == '.') {
            str.erase(std::end(str) - 1);
        }
    }

    return str;
}


std::string packet_to_string(pong::packet::ChangeUsername const& change_username) {
    return "ChangeUsername{ username: " + change_username.username + " }";
}

std::string packet_to_string(pong::packet::ChangeUsernameResponse const& username_response) {
    std::string response{ "??? "};
    switch(username_response.result) {
        case pong::packet::ChangeUsernameResponse::Okay: response = "Okay"; break;
        case pong::packet::ChangeUsernameResponse::InvalidCharacters: response = "InvalidCharacters"; break;
        case pong::packet::ChangeUsernameResponse::TooShort: response = "TooShort"; break;
        case pong::packet::ChangeUsernameResponse::TooLong: response = "TooLong"; break;
    }
    return "ChangeUsernameResponse{ result: " + response + " }";
}

std::string packet_to_string(pong::packet::LobbyInfo const& lobby_info) {
    std::string res = "LobbyInfo{ users: [";
    if (!lobby_info.users.empty()) {
        res += " ";

        for(auto it = std::begin(lobby_info.users); it != std::end(lobby_info.users); ++it) {
            auto const& user = *it;
            res += user;
            if (it + 1 != std::end(lobby_info.users)) {
                res += ", ";
            }
        }

        res += " ";
    }

    res += "], rooms: [";

    if (!lobby_info.rooms.empty()) {
        res += " ";

        for(auto it = std::begin(lobby_info.rooms); it != std::end(lobby_info.rooms); ++it) {
            auto const& room = *it;
            res += std::to_string(room);
            if (it + 1 != std::end(lobby_info.rooms)) {
                res += ", ";
            }
        }

        res += " ";
    }
    return res + "] }";
}

std::string packet_to_string(pong::packet::NewUser const& new_user) {
    return "NewUser{ username: " + new_user.username + " }";
}

std::string packet_to_string(pong::packet::OldUser const& old_user) {
    return "OldUser{ username: " + old_user.username + " }";
}

std::string packet_to_string(pong::packet::NewRoom const& new_room) {
    return "NewRoom{ id: " + std::to_string(new_room.id) + " }";
}

std::string packet_to_string(pong::packet::OldRoom const& old_room) {
    return "OldRoom{ id: " + std::to_string(old_room.id) + " }";
}

std::string packet_to_string(pong::packet::EnterRoom const& enter_room) {
    return "EnterRoom{ id: " + std::to_string(enter_room.id) + " }";
}

std::string packet_to_string(pong::packet::CreateRoom const&) {
    return "CreateRoom{}";
}

std::string packet_to_string(pong::packet::EnterRoomResponse const& enter_room_response) {
    std::string response{ "??? "};
    switch(enter_room_response.result) {
        case pong::packet::EnterRoomResponse::Okay: response = "Okay"; break;
        case pong::packet::EnterRoomResponse::Full: response = "Full"; break;
        case pong::packet::EnterRoomResponse::InvalidID: response = "InvalidID"; break;
    }
    return "EnterRoomResponse{ result: " + response + " }";
}

std::string packet_to_string(pong::packet::RoomInfo const& room_info) {
    std::string res = "RoomInfo{ left_player: " + room_info.left_player;
    res += ", right_player: " + room_info.right_player + ", spectators: [";

    if (!room_info.spectators.empty()) {
        res += " ";

        for(auto it = std::begin(room_info.spectators); it != std::end(room_info.spectators); ++it) {
            auto const& spectator = *it;
            res += spectator;
            if (it + 1 != std::end(room_info.spectators)) {
                res += ", ";
            }
        }

        res += " ";
    }
    return res + "] }";

}

std::string packet_to_string(pong::packet::LeaveRoom const&) {
    return "LeaveRoom{}";
}

std::string packet_to_string(pong::packet::GameState const& game_state) {
    auto const& ball = game_state.ball;
    std::string ball_str = "ball: { position: (" + float_to_string(ball.position.x) + ", " + float_to_string(ball.position.y) + "), speed: (";
    ball_str += float_to_string(ball.speed.x) + ", " + float_to_string(ball.speed.y) + ") }";

    auto pad_to_string = [] (pong::Pad const& pad) {
        return "pad: { y_position: " + float_to_string(pad.y) + ", speed: " + float_to_string(pad.speed) + " }";
    };

    auto left_str = "left_" + pad_to_string(game_state.left);
    auto right_str = "right_" + pad_to_string(game_state.right);

    return "GameState{ " + ball_str + ", " + left_str + ", " + right_str + " }";
}

std::string packet_to_string(pong::packet::Input const& input) {
    std::string response{ "??? "};
    switch(input.input) {
        case pong::Input::Idle: response = "Idle"; break;
        case pong::Input::Up: response = "Up"; break;
        case pong::Input::Down: response = "Down"; break;
    }
    return "Input{ input: " + response + " }";
}

std::string packet_to_string(pong::packet::NewPlayer const& packet) {
    std::string side = packet.side == pong::packet::NewPlayer::Side::Left ? "Left" : "Right";
    return "NewPlayer{" + side + ", " + packet.username + "}";
}

std::string packet_to_string(pong::packet::OldPlayer const& packet)  {
    std::string side = packet.side == pong::packet::OldPlayer::Side::Left ? "Left" : "Right";
    return "OldPlayer{ " + side + ", " + packet.username + " }";
}

std::string packet_to_string(pong::packet::BePlayer const& packet) {
    std::string side = packet.side == pong::packet::BePlayer::Side::Left ? "Left" : "Right";
    return "BePlayer{ " + side + " }";
}

std::string packet_to_string(pong::packet::Abandon const&) {
    return "Abandon{}";
}

std::string packet_to_string(pong::packet::EnterQueue const&) {
    return "EnterQueue{}";
}

std::string packet_to_string(pong::packet::LeaveQueue const&) {
    return "LeaveQueue{}";
}


}