#pragma once

#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>
#include <pong/client/InMainLobby.hpp>

namespace pong::client {

struct LoginIn : State<LoginIn> {
    using base_t = State<LoginIn>;

    sf::Font const& font;

    LoginIn(socket_ref_t _socket, std::string username, sf::Font const& font) 
    :   base_t(_socket, {
            { pong::packet::PacketID::UsernameResponse, &LoginIn::on_username_response }
        })
    ,   font{ font }
    {

        send(pong::packet::ChangeUsername{ std::move(username) });

    }

    Action on_username_response(packet_t packet) {
        auto res = from_packet<pong::packet::UsernameResponse>(packet).result;
        std::cout << "UsernameResponse: " << (int)res << '\n';
        if (res == pong::packet::UsernameResponse::Okay) {
            return change_state<InMainLobby>(font);
        }

        return Abord{};
    }

};

}