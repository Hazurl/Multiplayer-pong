#pragma once

#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>

#include <pong/client/gui/Button.hpp>

namespace pong::client {

struct InRoom : State<InRoom> {
    using base_t = State<InRoom>;

    sf::Font const& font;

    InRoom(socket_ref_t _socket, sf::Font const& font) 
    :   base_t(_socket, {
            { pong::packet::PacketID::GameState, &InRoom::on_game_state },
            { pong::packet::PacketID::RoomInfo,  &InRoom::on_room_info },
            { pong::packet::PacketID::NewUser,   &InRoom::on_new_user },
            { pong::packet::PacketID::OldUser,   &InRoom::on_old_user },
            { pong::packet::PacketID::NewPlayer, &InRoom::on_new_player },
            { pong::packet::PacketID::OldPlayer, &InRoom::on_old_player },
            { pong::packet::PacketID::BePlayer,  &InRoom::on_be_player }
        })
// GameState, NewUser, OldUser, NewPlayer, OldPlayer, BePlayer
    ,   font{ font }
    {
    }

    void update(float dt) override {

    }


    void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
        
    }



    Action on_room_info(packet_t packet) {
        auto room_info = from_packet<pong::packet::RoomInfo>(packet);
        std::cout << "Right player: " << room_info.right_player << '\n';
        std::cout << "Left player: " << room_info.left_player << '\n';
        std::cout << "Spectators: ";
        for(auto sp : room_info.spectators) {
            std::cout << sp << " ";
        }
        std::cout << "\n";
        return Idle{};
    }

    Action on_game_state(packet_t packet) {
        auto gs = from_packet<pong::packet::GameState>(packet);
        return Idle{};
    }

    Action on_new_user(packet_t packet) {
        auto new_user = from_packet<pong::packet::NewUser>(packet).username;
        std::cout << "New user " << new_user << '\n';
        return Idle{};
    }

    Action on_old_user(packet_t packet) {
        auto old_user = from_packet<pong::packet::OldUser>(packet).username;
        std::cout << "Old user " << old_user << '\n';
        return Idle{};
    }

    Action on_new_player(packet_t packet) {
        auto new_player = from_packet<pong::packet::NewPlayer>(packet);
        std::cout << "New player " << new_player.username << " on " << (new_player.side == pong::packet::NewPlayer::Side::Left ? "left" : "right") << " side" << '\n';
        return Idle{};
    }

    Action on_old_player(packet_t packet) {
        auto old_player = from_packet<pong::packet::OldPlayer>(packet);
        std::cout << "Old player " << old_player.username << " on " << (old_player.side == pong::packet::OldPlayer::Side::Left ? "left" : "right") << " side" << '\n';
        return Idle{};
    }

    Action on_be_player(packet_t packet) {
        auto be_player_side = from_packet<pong::packet::BePlayer>(packet);
        std::cout << "BePlayer " << (be_player_side.side == pong::packet::BePlayer::Side::Left ? "left" : "right") << '\n';
        return Idle{};
    }



};

}