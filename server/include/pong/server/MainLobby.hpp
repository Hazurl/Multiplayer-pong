#pragma once

#include <pong/server/Common.hpp>
#include <pong/server/State.hpp>

#include <pong/server/Room.hpp>

namespace pong::server {

struct MainLobbyState : public State<MainLobbyState, user_t> {
    MainLobbyState(std::vector<std::unique_ptr<RoomState>>& rooms) : State({
        // Receive
        { pong::packet::PacketID::CreateRoom, &MainLobbyState::on_create_room }
    }), rooms{ rooms } {}

    std::vector<std::unique_ptr<RoomState>>& rooms;

    Action on_create_room(user_handle_t handle, packet_t packet) {
        // Find an ID without a room
        std::size_t room_id{ 0 };
        for(; room_id < rooms.size(); ++room_id) {
            if (rooms[room_id] == nullptr || rooms[room_id]->is_empty()) {
                break;
            }
        }


        std::cout << "New room #" << room_id << " created\n";


        if (room_id >= rooms.size()) {
            rooms.emplace_back(std::make_unique<RoomState>(*this));


        } else if(rooms[room_id] == nullptr) {
            rooms[room_id] = std::make_unique<RoomState>(*this);


        }


        broadcast_other(handle, pong::packet::NewRoom{
            static_cast<int>(room_id)
        });


        return order_change_state(
            *rooms[room_id],
            handle,
            get_user_data(handle)
        );
    }



    void on_user_enter(user_handle_t handle) {
        std::vector<std::string> usernames;
        usernames.reserve(number_of_user());


        for(user_handle_t h{ 0 }; h < number_of_user(); ++h) {
            if (is_valid(h)) {
                usernames.emplace_back(get_user_data(h));
            }
        }

        std::vector<int> rooms_id;

        for(std::size_t room_id{ 0 }; room_id < rooms.size(); ++room_id) {
            if (rooms[room_id] && !rooms[room_id]->is_empty()) {
                rooms_id.emplace_back(static_cast<int>(room_id));
            }
        }

        send(handle, pong::packet::LobbyInfo{
            std::move(usernames), std::move(rooms_id)
        });

        broadcast_other(handle, pong::packet::NewUser{
            get_user_data(handle)
        });
    }



    void on_user_leave(user_handle_t handle) {
        broadcast_other(handle, pong::packet::OldUser{
            get_user_data(handle)
        });
    }
};

}