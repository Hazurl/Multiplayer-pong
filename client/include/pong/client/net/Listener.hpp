#pragma once

#include <multipong/Packets.hpp>

#include <pong/client/StateAction.hpp>

namespace pong::client::net {

class Listener {
public:

    virtual ~Listener() = default;

    StateAction on_send(pong::packet::GamePacket const& game_packet);
    StateAction on_receive(pong::packet::GamePacket const& game_packet);
    StateAction on_receive(sf::Packet packet);

    virtual StateAction on_send_change_username(pong::packet::ChangeUsername const& packet);
    virtual StateAction on_send_game_state(pong::packet::GameState const& packet);
    virtual StateAction on_send_create_room(pong::packet::CreateRoom const& packet);
    virtual StateAction on_send_enter_room(pong::packet::EnterRoom const& packet);
    virtual StateAction on_send_enter_room_response(pong::packet::EnterRoomResponse const& packet);
    virtual StateAction on_send_input(pong::packet::Input const& packet);
    virtual StateAction on_send_leave_room(pong::packet::LeaveRoom const& packet);
    virtual StateAction on_send_lobby_info(pong::packet::LobbyInfo const& packet);
    virtual StateAction on_send_new_room(pong::packet::NewRoom const& packet);
    virtual StateAction on_send_new_user(pong::packet::NewUser const& packet);
    virtual StateAction on_send_old_room(pong::packet::OldRoom const& packet);
    virtual StateAction on_send_old_user(pong::packet::OldUser const& packet);
    virtual StateAction on_send_room_info(pong::packet::RoomInfo const& packet);
    virtual StateAction on_send_username_response(pong::packet::UsernameResponse const& packet);
    virtual StateAction on_send_new_player(pong::packet::NewPlayer const& packet);
    virtual StateAction on_send_old_player(pong::packet::OldPlayer const& packet);
    virtual StateAction on_send_be_player(pong::packet::BePlayer const& packet);
    virtual StateAction on_send_abandon(pong::packet::Abandon const& packet);
    virtual StateAction on_send_enter_queue(pong::packet::EnterQueue const& packet);
    virtual StateAction on_send_leave_queue(pong::packet::LeaveQueue const& packet);

    virtual StateAction on_receive_change_username(pong::packet::ChangeUsername const& packet);
    virtual StateAction on_receive_game_state(pong::packet::GameState const& packet);
    virtual StateAction on_receive_create_room(pong::packet::CreateRoom const& packet);
    virtual StateAction on_receive_enter_room(pong::packet::EnterRoom const& packet);
    virtual StateAction on_receive_enter_room_response(pong::packet::EnterRoomResponse const& packet);
    virtual StateAction on_receive_input(pong::packet::Input const& packet);
    virtual StateAction on_receive_leave_room(pong::packet::LeaveRoom const& packet);
    virtual StateAction on_receive_lobby_info(pong::packet::LobbyInfo const& packet);
    virtual StateAction on_receive_new_room(pong::packet::NewRoom const& packet);
    virtual StateAction on_receive_new_user(pong::packet::NewUser const& packet);
    virtual StateAction on_receive_old_room(pong::packet::OldRoom const& packet);
    virtual StateAction on_receive_old_user(pong::packet::OldUser const& packet);
    virtual StateAction on_receive_room_info(pong::packet::RoomInfo const& packet);
    virtual StateAction on_receive_username_response(pong::packet::UsernameResponse const& packet);
    virtual StateAction on_receive_new_player(pong::packet::NewPlayer const& packet);
    virtual StateAction on_receive_old_player(pong::packet::OldPlayer const& packet);
    virtual StateAction on_receive_be_player(pong::packet::BePlayer const& packet);
    virtual StateAction on_receive_abandon(pong::packet::Abandon const& packet);
    virtual StateAction on_receive_enter_queue(pong::packet::EnterQueue const& packet);
    virtual StateAction on_receive_leave_queue(pong::packet::LeaveQueue const& packet);


};

}