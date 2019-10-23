#pragma once

#include <pong/server/Common.hpp>
#include <pong/server/State.hpp>

#include <pong/server/MainLobby.hpp>

namespace pong::server {

bool is_username_valid(std::string const& username) {
    if (username.size() < 3) {
        std::cout << '"' << username << '"' << " is too short\n";
        return false;
    }

    if (username.size() > 20) {
        std::cout << '"' << username << '"' << " is too long\n";
        return false;
    }

    if (!std::all_of(std::begin(username), std::end(username), [] (auto c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    })) {
        std::cout << '"' << username << '"' << " is not valid\n";
        return false;
    }

    return true;

}



struct NewUserState : public State<NewUserState> {
    NewUserState(MainLobbyState& _main_lobby) : State({


        // Receive
        { id_of(pong::packet::client::ChangeUsername{}), &NewUserState::on_username_changed }


    }), main_lobby{ _main_lobby } {}

    
    
    MainLobbyState& main_lobby;



    Action on_username_changed(user_handle_t handle, packet_t packet) {
        auto username = from_packet<pong::packet::client::ChangeUsername>(packet).username;

        auto response = is_username_valid(username);

        std::cout << "Send ChangeUsernameResponse\n";
        send(handle, pong::packet::server::ChangeUsernameResponse{
            response
        });

        if (!response) {
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