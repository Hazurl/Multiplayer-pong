#pragma once

#include <pong/server/Common.hpp>
#include <pong/server/State.hpp>

#include <pong/server/MainLobby.hpp>

namespace pong::server {

pong::packet::UsernameResponse::Result is_username_valid(std::string const& username) {
    if (username.size() < 3) {
        std::cout << '"' << username << '"' << " is too short\n";
        return pong::packet::UsernameResponse::TooShort;
    }

    if (username.size() > 20) {
        std::cout << '"' << username << '"' << " is too long\n";
        return pong::packet::UsernameResponse::TooLong;
    }

    if (!std::all_of(std::begin(username), std::end(username), [] (auto c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    })) {
        std::cout << '"' << username << '"' << " is not valid\n";
        return pong::packet::UsernameResponse::InvalidCharacters;
    }

    return pong::packet::UsernameResponse::Okay;

}



struct NewUserState : public State<NewUserState> {
    NewUserState(MainLobbyState& _main_lobby) : State({


        // Receive
        { pong::packet::PacketID::ChangeUsername, &NewUserState::on_username_changed }


    }), main_lobby{ _main_lobby } {}

    
    
    MainLobbyState& main_lobby;



    Action on_username_changed(user_handle_t handle, packet_t packet) {
        auto username = from_packet<pong::packet::ChangeUsername>(packet).username;

        auto response = is_username_valid(username);

        send(handle, pong::packet::UsernameResponse{
            response
        });

        if (response != pong::packet::UsernameResponse::Okay) {
            std::cout << "Username " << username << " is not valid\n";
            return Idle{};
        }

        std::cout << username << " is now connected\n";

        return order_change_state(
            main_lobby,
            handle,
            username
        );

    }
};

}