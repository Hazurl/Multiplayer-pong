#pragma once

#include <cassert>

#include <pong/packet/Client.hpp>
#include <pong/packet/Server.hpp>

namespace pong::packet {

enum class SubState {
    NewUser_Invalid,
    NewUser_Connecting,
    Lobby_New,
    Lobby_RegularUser,
    Lobby_EnteringRoom,
    Lobby_CreatingRoom,
    Room_New,
    Room_Leaving,
    Room_Spectator,
    Room_Queued,
    Room_AcceptingBePlayer,
    Room_NextPlayer,
    Room_Player
};

enum class State {
    NewUser, Lobby, Room
};

constexpr std::string_view to_string_view(SubState state) {
    switch(state) {
        case SubState::NewUser_Invalid: return "NewUser::Invalid";
        case SubState::NewUser_Connecting: return "NewUser::Connecting";
        case SubState::Lobby_New: return "Lobby::New";
        case SubState::Lobby_RegularUser: return "Lobby::RegularUser";
        case SubState::Lobby_EnteringRoom: return "Lobby::EnteringRoom";
        case SubState::Lobby_CreatingRoom: return "Lobby::CreatingRoom";
        case SubState::Room_New: return "Room::New";
        case SubState::Room_Leaving: return "Room::Leaving";
        case SubState::Room_Spectator: return "Room::Spectator";
        case SubState::Room_Queued: return "Room::Queued";
        case SubState::Room_AcceptingBePlayer: return "Room::AcceptingBePlayer";
        case SubState::Room_NextPlayer: return "Room::NextPlayer";
        case SubState::Room_Player: return "Room::Player";
    }

    assert(false);
}

constexpr State get_master_state(SubState state) {
    switch(state) {
        case SubState::NewUser_Invalid: return State::NewUser;
        case SubState::NewUser_Connecting: return State::NewUser;
        case SubState::Lobby_New: return State::Lobby;
        case SubState::Lobby_RegularUser: return State::Lobby;
        case SubState::Lobby_EnteringRoom: return State::Lobby;
        case SubState::Lobby_CreatingRoom: return State::Lobby;
        case SubState::Room_New: return State::Room;
        case SubState::Room_Leaving: return State::Room;
        case SubState::Room_Spectator: return State::Room;
        case SubState::Room_Queued: return State::Room;
        case SubState::Room_AcceptingBePlayer: return State::Room;
        case SubState::Room_NextPlayer: return State::Room;
        case SubState::Room_Player: return State::Room;
    }

    assert(false);
}

template<typename T>
constexpr bool is_packet_ignored_in(SubState state, T const& = T{}) {
    switch(state) {
        
        case SubState::Room_Leaving: 
            return 
                std::is_same_v<T, server::DeniedBePlayer>
            ||  std::is_same_v<T, server::BeNextPlayer>;

        case SubState::Room_Spectator: 
            return 
                std::is_same_v<T, server::DeniedBePlayer>
            ||  std::is_same_v<T, server::BeNextPlayer>
            ||  std::is_same_v<T, client::AcceptBePlayer>
            ||  std::is_same_v<T, client::LeaveQueue>
            ||  std::is_same_v<T, client::Abandon>
            ||  std::is_same_v<T, client::Input>;

        case SubState::Room_Queued: 
            return 
                std::is_same_v<T, server::DeniedBePlayer>
            ||  std::is_same_v<T, client::Abandon>
            ||  std::is_same_v<T, client::Input>;

        case SubState::Room_AcceptingBePlayer: 
            return 
                std::is_same_v<T, client::Abandon>
            ||  std::is_same_v<T, client::Input>;

        default: 
            return false;
    }
}

template<typename T>
constexpr bool is_packet_expected_in(SubState state, T const& = T{}) {
        case SubState::NewUser_Invalid:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, client::ChangeUsername>;

        case SubState::NewUser_Connecting:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::ChangeUsernameResponse>;

        case SubState::Lobby_New:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::LobbyInfo>;

        case SubState::Lobby_RegularUser:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewRoom>
            ||  std::is_same_v<T, server::OldRoom>
            ||  std::is_same_v<T, server::RoomInfo>
            ||  std::is_same_v<T, client::SubscribeRoomInfo>
            ||  std::is_same_v<T, client::EnterRoom>
            ||  std::is_same_v<T, client::CreateRoom>;

        case SubState::Lobby_EnteringRoom:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewRoom>
            ||  std::is_same_v<T, server::OldRoom>
            ||  std::is_same_v<T, server::RoomInfo>
            ||  std::is_same_v<T, client::SubscribeRoomInfo>
            ||  std::is_same_v<T, client::EnterRoomResponse>;

        case SubState::Lobby_CreatingRoom:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewRoom>
            ||  std::is_same_v<T, server::OldRoom>
            ||  std::is_same_v<T, server::RoomInfo>
            ||  std::is_same_v<T, client::SubscribeRoomInfo>
            ||  std::is_same_v<T, client::CreateRoomResponse>;

        case SubState::Room_New:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::RoomInfo>;

        case SubState::Room_Leaving:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewPlayer>
            ||  std::is_same_v<T, server::OldPlayer>
            ||  std::is_same_v<T, server::GameState>
            ||  std::is_same_v<T, server::Score>
            ||  std::is_same_v<T, server::GameOver>
            ||  std::is_same_v<T, server::LeaveRoomResponse>;

        case SubState::Room_Spectator:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewPlayer>
            ||  std::is_same_v<T, server::OldPlayer>
            ||  std::is_same_v<T, server::GameState>
            ||  std::is_same_v<T, server::Score>
            ||  std::is_same_v<T, server::GameOver>
            ||  std::is_same_v<T, client::EnterQueue>
            ||  std::is_same_v<T, client::LeaveRoom>;

        case SubState::Room_Queued:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewPlayer>
            ||  std::is_same_v<T, server::OldPlayer>
            ||  std::is_same_v<T, server::GameState>
            ||  std::is_same_v<T, server::Score>
            ||  std::is_same_v<T, server::GameOver>
            ||  std::is_same_v<T, server::BeNextPlayer>
            ||  std::is_same_v<T, client::LeaveQueue>
            ||  std::is_same_v<T, client::LeaveRoom>;

        case SubState::Room_AcceptingBePlayer:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewPlayer>
            ||  std::is_same_v<T, server::OldPlayer>
            ||  std::is_same_v<T, server::GameState>
            ||  std::is_same_v<T, server::Score>
            ||  std::is_same_v<T, server::GameOver>
            ||  std::is_same_v<T, server::DeniedBePlayer>
            ||  std::is_same_v<T, client::AcceptBePlayer>
            ||  std::is_same_v<T, client::LeaveQueue>
            ||  std::is_same_v<T, client::LeaveRoom>;

        case SubState::Room_NextPlayer:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewPlayer>
            ||  std::is_same_v<T, server::OldPlayer>
            ||  std::is_same_v<T, server::GameState>
            ||  std::is_same_v<T, server::Score>
            ||  std::is_same_v<T, server::GameOver>
            ||  std::is_same_v<T, server::DeniedBePlayer>
            ||  std::is_same_v<T, client::BePlayer>;

        case SubState::Room_Player:
            return
                is_packet_ignored_in<T>(state)
            ||  std::is_same_v<T, server::NewUser>
            ||  std::is_same_v<T, server::OldUser>
            ||  std::is_same_v<T, server::NewPlayer>
            ||  std::is_same_v<T, server::OldPlayer>
            ||  std::is_same_v<T, server::GameState>
            ||  std::is_same_v<T, server::Score>
            ||  std::is_same_v<T, server::GameOver>
            ||  std::is_same_v<T, client::Input>
            ||  std::is_same_v<T, client::Abandon>;

}

}