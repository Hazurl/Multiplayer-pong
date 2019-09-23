#pragma once

#include <pong/server/Common.hpp>
#include <pong/server/State.hpp>

namespace pong::server {

struct MainLobbyState;

struct RoomState : public State<RoomState, user_t> {
    RoomState(MainLobbyState& main_lobby) : State({


        // Receive
        { pong::packet::PacketID::Input, &RoomState::on_input },
        { pong::packet::PacketID::LeaveRoom, &RoomState::on_leave_room },


    }), main_lobby{ main_lobby }, left_player{ invalid_user_handle }, right_player{ invalid_user_handle } {}


    MainLobbyState& main_lobby;
    user_handle_t left_player;
    user_handle_t right_player;


    Action on_input(user_handle_t handle, packet_t packet) {
        if (handle == left_player) {
            auto input = from_packet<pong::packet::Input>(packet);
        }
        else if (handle == right_player) {
            auto input = from_packet<pong::packet::Input>(packet);
        }
        else {
            std::cerr << "[Warning] Received PacketID::Input from a spectator\n";
        }

        return Idle{};
    }


    Action on_leave_room(user_handle_t handle, packet_t packet) {
        return order_change_state(main_lobby, handle, get_user_data(handle));
    }


    void on_user_enter(user_handle_t handle) {
        broadcast_other(handle, pong::packet::NewUser{
            get_user_data(handle)
        });


        std::vector<std::string> spectators;
        spectators.reserve(number_of_user());


        for(user_handle_t h{ 0 }; h < number_of_user(); ++h) {
            if (is_valid(h) && h != left_player && h != right_player) {
                spectators.push_back(get_user_data(h));
            }
        }


        send(handle, pong::packet::RoomInfo{
            is_valid(left_player) ? get_user_data(left_player) : "",
            is_valid(right_player) ? get_user_data(right_player) : "",
            std::move(spectators)
        });
    }


    void on_user_leave(user_handle_t handle) {
        if (handle == left_player) {
            left_player = invalid_user_handle;
        }
        else if (handle == right_player) {
            right_player = invalid_user_handle;
        }

        broadcast_other(handle, pong::packet::OldUser{
            get_user_data(handle)
        });
    }
};

}