/*#include <pong/client/net/Listener.hpp>

#include <pong/client/Visitor.hpp>

#include <pong/client/Logger.hpp>

namespace pong::client::net {

StateAction Listener::on_send(pong::packet::GamePacket const& game_packet) {
    return std::visit(Visitor{
        [this] (pong::packet::ChangeUsername const& packet) { return on_send_change_username(packet); },
        [this] (pong::packet::GameState const& packet) { return on_send_game_state(packet); },
        [this] (pong::packet::CreateRoom const& packet) { return on_send_create_room(packet); },
        [this] (pong::packet::EnterRoom const& packet) { return on_send_enter_room(packet); },
        [this] (pong::packet::EnterRoomResponse const& packet) { return on_send_enter_room_response(packet); },
        [this] (pong::packet::Input const& packet) { return on_send_input(packet); },
        [this] (pong::packet::LeaveRoom const& packet) { return on_send_leave_room(packet); },
        [this] (pong::packet::LobbyInfo const& packet) { return on_send_lobby_info(packet); },
        [this] (pong::packet::NewRoom const& packet) { return on_send_new_room(packet); },
        [this] (pong::packet::NewUser const& packet) { return on_send_new_user(packet); },
        [this] (pong::packet::OldRoom const& packet) { return on_send_old_room(packet); },
        [this] (pong::packet::OldUser const& packet) { return on_send_old_user(packet); },
        [this] (pong::packet::RoomInfo const& packet) { return on_send_room_info(packet); },
        [this] (pong::packet::ChangeUsernameResponse const& packet) { return on_send_username_response(packet); },
        [this] (pong::packet::NewPlayer const& packet) { return on_send_new_player(packet); },
        [this] (pong::packet::OldPlayer const& packet) { return on_send_old_player(packet); },
        [this] (pong::packet::BePlayer const& packet) { return on_send_be_player(packet); },
        [this] (pong::packet::Abandon const& packet) { return on_send_abandon(packet); },
        [this] (pong::packet::EnterQueue const& packet) { return on_send_enter_queue(packet); },
        [this] (pong::packet::LeaveQueue const& packet) { return on_send_leave_queue(packet); }
    }, game_packet);
}



StateAction Listener::on_receive(pong::packet::GamePacket const& game_packet) {
    return std::visit(Visitor{
        [this] (pong::packet::ChangeUsername const& packet) { return on_receive_change_username(packet); },
        [this] (pong::packet::GameState const& packet) { return on_receive_game_state(packet); },
        [this] (pong::packet::CreateRoom const& packet) { return on_receive_create_room(packet); },
        [this] (pong::packet::EnterRoom const& packet) { return on_receive_enter_room(packet); },
        [this] (pong::packet::EnterRoomResponse const& packet) { return on_receive_enter_room_response(packet); },
        [this] (pong::packet::Input const& packet) { return on_receive_input(packet); },
        [this] (pong::packet::LeaveRoom const& packet) { return on_receive_leave_room(packet); },
        [this] (pong::packet::LobbyInfo const& packet) { return on_receive_lobby_info(packet); },
        [this] (pong::packet::NewRoom const& packet) { return on_receive_new_room(packet); },
        [this] (pong::packet::NewUser const& packet) { return on_receive_new_user(packet); },
        [this] (pong::packet::OldRoom const& packet) { return on_receive_old_room(packet); },
        [this] (pong::packet::OldUser const& packet) { return on_receive_old_user(packet); },
        [this] (pong::packet::RoomInfo const& packet) { return on_receive_room_info(packet); },
        [this] (pong::packet::ChangeUsernameResponse const& packet) { return on_receive_username_response(packet); },
        [this] (pong::packet::NewPlayer const& packet) { return on_receive_new_player(packet); },
        [this] (pong::packet::OldPlayer const& packet) { return on_receive_old_player(packet); },
        [this] (pong::packet::BePlayer const& packet) { return on_receive_be_player(packet); },
        [this] (pong::packet::Abandon const& packet) { return on_receive_abandon(packet); },
        [this] (pong::packet::EnterQueue const& packet) { return on_receive_enter_queue(packet); },
        [this] (pong::packet::LeaveQueue const& packet) { return on_receive_leave_queue(packet); }
    }, game_packet);
}


StateAction Listener::on_receive(sf::Packet packet) {
    pong::packet::GamePacket game_packet;
    packet >> game_packet;

    NOTICE("Received packet #", game_packet.index());

    return on_receive(game_packet);
}



StateAction Listener::on_send_change_username(pong::packet::ChangeUsername const&) {
    return idle();
}

StateAction Listener::on_send_game_state(pong::packet::GameState const&) {
    return idle();
}

StateAction Listener::on_send_create_room(pong::packet::CreateRoom const&) {
    return idle();
}

StateAction Listener::on_send_enter_room(pong::packet::EnterRoom const&) {
    return idle();
}

StateAction Listener::on_send_enter_room_response(pong::packet::EnterRoomResponse const&) {
    return idle();
}

StateAction Listener::on_send_input(pong::packet::Input const&) {
    return idle();
}

StateAction Listener::on_send_leave_room(pong::packet::LeaveRoom const&) {
    return idle();
}

StateAction Listener::on_send_lobby_info(pong::packet::LobbyInfo const&) {
    return idle();
}

StateAction Listener::on_send_new_room(pong::packet::NewRoom const&) {
    return idle();
}

StateAction Listener::on_send_new_user(pong::packet::NewUser const&) {
    return idle();
}

StateAction Listener::on_send_old_room(pong::packet::OldRoom const&) {
    return idle();
}

StateAction Listener::on_send_old_user(pong::packet::OldUser const&) {
    return idle();
}

StateAction Listener::on_send_room_info(pong::packet::RoomInfo const&) {
    return idle();
}

StateAction Listener::on_send_username_response(pong::packet::ChangeUsernameResponse const&) {
    return idle();
}

StateAction Listener::on_send_new_player(pong::packet::NewPlayer const&) {
    return idle();
}

StateAction Listener::on_send_old_player(pong::packet::OldPlayer const&) {
    return idle();
}

StateAction Listener::on_send_be_player(pong::packet::BePlayer const&) {
    return idle();
}

StateAction Listener::on_send_abandon(pong::packet::Abandon const&) {
    return idle();
}

StateAction Listener::on_send_enter_queue(pong::packet::EnterQueue const&) {
    return idle();
}

StateAction Listener::on_send_leave_queue(pong::packet::LeaveQueue const&) {
    return idle();
}




StateAction Listener::on_receive_change_username(pong::packet::ChangeUsername const&) {
    return idle();
}

StateAction Listener::on_receive_game_state(pong::packet::GameState const&) {
    return idle();
}

StateAction Listener::on_receive_create_room(pong::packet::CreateRoom const&) {
    return idle();
}

StateAction Listener::on_receive_enter_room(pong::packet::EnterRoom const&) {
    return idle();
}

StateAction Listener::on_receive_enter_room_response(pong::packet::EnterRoomResponse const&) {
    return idle();
}

StateAction Listener::on_receive_input(pong::packet::Input const&) {
    return idle();
}

StateAction Listener::on_receive_leave_room(pong::packet::LeaveRoom const&) {
    return idle();
}

StateAction Listener::on_receive_lobby_info(pong::packet::LobbyInfo const&) {
    return idle();
}

StateAction Listener::on_receive_new_room(pong::packet::NewRoom const&) {
    return idle();
}

StateAction Listener::on_receive_new_user(pong::packet::NewUser const&) {
    return idle();
}

StateAction Listener::on_receive_old_room(pong::packet::OldRoom const&) {
    return idle();
}

StateAction Listener::on_receive_old_user(pong::packet::OldUser const&) {
    return idle();
}

StateAction Listener::on_receive_room_info(pong::packet::RoomInfo const&) {
    return idle();
}

StateAction Listener::on_receive_username_response(pong::packet::ChangeUsernameResponse const&) {
    return idle();
}

StateAction Listener::on_receive_new_player(pong::packet::NewPlayer const&) {
    return idle();
}

StateAction Listener::on_receive_old_player(pong::packet::OldPlayer const&) {
    return idle();
}

StateAction Listener::on_receive_be_player(pong::packet::BePlayer const&) {
    return idle();
}

StateAction Listener::on_receive_abandon(pong::packet::Abandon const&) {
    return idle();
}

StateAction Listener::on_receive_enter_queue(pong::packet::EnterQueue const&) {
    return idle();
}

StateAction Listener::on_receive_leave_queue(pong::packet::LeaveQueue const&) {
    return idle();
}


}*/