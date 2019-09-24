#pragma once

#include <pong/server/Common.hpp>
#include <pong/server/State.hpp>

#include <deque>

namespace pong::server {

struct MainLobbyState;

struct RoomState : public State<RoomState, user_t> {
    RoomState(MainLobbyState& _main_lobby) : State({

        // Receive
        { pong::packet::PacketID::Input, &RoomState::on_input },
        { pong::packet::PacketID::Abandon, &RoomState::on_abandon },
        { pong::packet::PacketID::EnterQueue, &RoomState::on_enter_queue },
        { pong::packet::PacketID::LeaveQueue, &RoomState::on_leave_queue },
        { pong::packet::PacketID::LeaveRoom, &RoomState::on_leave_room }


    }), main_lobby{ _main_lobby }, left_player{ invalid_user_handle }, right_player{ invalid_user_handle } {}

    /*
        [CLIENT] Send LeaveRoom, EnterQueue, LeaveQueue
        [SERVER] Send GameState, NewUser, OldUser, NewPlayer, OldPlayer, BePlayer 
    */

    /*
        [CLIENT] Send Input, Abandon
        [SERVER] Send GameState, NewUser, OldUser, NewPlayer, OldPlayer 
    */

    MainLobbyState& main_lobby;
    user_handle_t left_player;
    user_handle_t right_player;

    std::deque<user_handle_t> queue;


    void update_players() {
        if (queue.empty()) return;
        if (left_player == invalid_user_handle) {
            auto handle = queue.back(); queue.pop_back();
            left_player = handle;


            std::cout << "Send NewPlayer\n";
            broadcast_other(left_player, pong::packet::NewPlayer{
                pong::packet::NewPlayer::Side::Left,
                get_user_data(left_player)
            });
            std::cout << "Send BePlayer\n";
            send(left_player, pong::packet::BePlayer{
                pong::packet::BePlayer::Side::Left
            });

            
            if (queue.empty()) return;


        }


        if (right_player == invalid_user_handle) {
            auto handle = queue.back(); queue.pop_back();
            right_player = handle;


            std::cout << "Send NewPlayer\n";
            broadcast_other(right_player, pong::packet::NewPlayer{
                pong::packet::NewPlayer::Side::Right,
                get_user_data(right_player)
            });
            std::cout << "Send BePlayer\n";
            send(right_player, pong::packet::BePlayer{
                pong::packet::BePlayer::Side::Right
            });
        }
    }


    Action on_abandon(user_handle_t handle, packet_t) {
        if (handle == left_player) {
            std::cout << "Send OldPlayer\n";
            broadcast_other(handle, pong::packet::OldPlayer{
                pong::packet::OldPlayer::Side::Left,
                get_user_data(handle)
            });
            std::cout << "Send NewUser\n";
            broadcast_other(handle, pong::packet::NewUser{
                get_user_data(handle)
            });
            left_player = invalid_user_handle;
            update_players();
        }
        else if (handle == right_player) {
            std::cout << "Send OldPlayer\n";
            broadcast_other(handle, pong::packet::OldPlayer{
                pong::packet::OldPlayer::Side::Right,
                get_user_data(handle)
            });
            std::cout << "Send NewUser\n";
            broadcast_other(handle, pong::packet::NewUser{
                get_user_data(handle)
            });
            right_player = invalid_user_handle;
            update_players();
        }
        else {
            std::cerr << "[Warning] Received PacketID::Abandon from a spectator\n";
        }

        return Idle{};
    }


    Action on_enter_queue(user_handle_t handle, packet_t) {
        if (handle == left_player || handle == right_player) {
            std::cerr << "[Warning] Received PacketID::EnterQueue from a player\n";
        } else {
            queue.push_front(handle);
            update_players();
        }

        return Idle{};
    }


    Action on_leave_queue(user_handle_t handle, packet_t) {
        if (handle == left_player || handle == right_player) {
            std::cerr << "[Warning] Received PacketID::LeaveQueue from a player\n";
        } else {
            queue.erase(std::remove(std::begin(queue), std::end(queue), handle), std::end(queue));
        }

        return Idle{};
    }


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


    Action on_leave_room(user_handle_t handle, packet_t) {
        return order_change_state(main_lobby, handle, get_user_data(handle));
    }


    void on_user_enter(user_handle_t handle) {
        std::cout << "Send NewUser\n";
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


        std::cout << "Send RoomInfo\n";
        send(handle, pong::packet::RoomInfo{
            is_valid(left_player) ? get_user_data(left_player) : "",
            is_valid(right_player) ? get_user_data(right_player) : "",
            std::move(spectators)
        });
    }


    void on_user_leave(user_handle_t handle) {
        if (handle == left_player) {
            left_player = invalid_user_handle;
            update_players();
            std::cout << "Send OldPlayer\n";
            broadcast_other(handle, pong::packet::OldPlayer{
                pong::packet::OldPlayer::Side::Left,
                get_user_data(handle)
            });
        }
        else if (handle == right_player) {
            right_player = invalid_user_handle;
            update_players();
            std::cout << "Send OldPlayer\n";
            broadcast_other(handle, pong::packet::OldPlayer{
                pong::packet::OldPlayer::Side::Right,
                get_user_data(handle)
            });
        } else {
            queue.erase(std::remove(std::begin(queue), std::end(queue), handle), std::end(queue));
        }

        std::cout << "Send OldUser\n";
        broadcast_other(handle, pong::packet::OldUser{
            get_user_data(handle)
        });
    }
};

}