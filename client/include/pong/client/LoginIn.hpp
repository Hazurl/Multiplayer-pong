#pragma once

#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>
#include <pong/client/InMainLobby.hpp>

namespace pong::client {

struct LoginIn : State<LoginIn> {
    using base_t = State<LoginIn>;

    using typename base_t::receiver_t;
    using typename base_t::receiver_map_t;
    using typename base_t::abord_connection_t;

    LoginIn(socket_ref_t _socket, std::string username) : base_t(_socket, {
        { pong::packet::PacketID::UsernameResponse, &LoginIn::on_username_response }
    }) {

        send(pong::packet::ChangeUsername{ std::move(username) });

    }

    Action on_username_response(packet_t packet) {
        auto res = from_packet<pong::packet::UsernameResponse>(packet).result;
        std::cout << "UsernameResponse: " << (int)res << '\n';
        if (res == pong::packet::UsernameResponse::Okay) {
            return change_state<InMainLobby>();
        }

        return Abord{};
    }

};

}