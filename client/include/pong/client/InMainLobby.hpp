#pragma once

#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>
#include <pong/client/InRoom.hpp>

#include <pong/client/gui/Button.hpp>

namespace pong::client {

struct InMainLobby : State<InMainLobby> {
    using base_t = State<InMainLobby>;

    gui::Button button;
    std::vector<gui::Button> join_room_buttons;
    sf::Font const& font;

    InMainLobby(socket_ref_t _socket, sf::Font const& font) 
    :   base_t(_socket, {
            { pong::packet::PacketID::LobbyInfo, &InMainLobby::on_lobby_info },
            { pong::packet::PacketID::NewRoom, &InMainLobby::on_new_room },
            { pong::packet::PacketID::NewUser, &InMainLobby::on_new_user },
            { pong::packet::PacketID::OldUser, &InMainLobby::on_old_user },
            { pong::packet::PacketID::EnterRoomResponse, &InMainLobby::on_enter_room_response }
        })
    ,   button([this] () { on_button_click(); }, font, "Create a new room", 20)
    ,   font{ font }
    {
        button.setPosition(200, 10);
    }

    sftk::PropagateEvent on_mouse_button_pressed(sf::Window&, sf::Event::MouseButtonEvent const& b) override {
        return button.on_mouse_button_pressed(b) 
        &&  std::all_of(
                std::begin(join_room_buttons), 
                std::end(join_room_buttons), 
                [b] (auto& but) { 
                    return but.on_mouse_button_pressed(b); 
                });
    }

    sftk::PropagateEvent on_mouse_button_released(sf::Window&, sf::Event::MouseButtonEvent const& b) override {
        return button.on_mouse_button_released(b)
        &&  std::all_of(
                std::begin(join_room_buttons), 
                std::end(join_room_buttons), 
                [b] (auto& but) { 
                    return but.on_mouse_button_released(b); 
                });
    }

    sftk::PropagateEvent on_mouse_moved(sf::Window&, sf::Event::MouseMoveEvent const& b) override {
        return button.on_mouse_moved(b)
        &&  std::all_of(
                std::begin(join_room_buttons), 
                std::end(join_room_buttons), 
                [b] (auto& but) { 
                    return but.on_mouse_moved(b); 
                });
    }


    void update(float dt) override {
        button.update(dt);
        for(auto& b : join_room_buttons) {
            b.update(dt);
        }
    }


    void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
        target.draw(button, states);
        for(auto& b : join_room_buttons) {
            target.draw(b, states);    
        }
    }



    void on_button_click() {
        send_notify([this] () {
            return change_state<InRoom>(font);
        }, pong::packet::CreateRoom{});
    }




    Action on_lobby_info(packet_t packet) {
        auto lobby_info = from_packet<pong::packet::LobbyInfo>(packet);

        std::cout << "Users: ";
        for(auto const& user : lobby_info.users) {
            std::cout << user << " ";
        }
        std::cout << "\nRooms: ";
        for(auto room : lobby_info.rooms) {
            auto& b = join_room_buttons.emplace_back(
                [this, room] () { send(pong::packet::EnterRoom{ room }); }, 
                font, 
                std::string{ "Join room " } + std::to_string(room), 
                20
            );
            b.setPosition(10, (join_room_buttons.size() - 1) * 30 + 10);
            std::cout << room << " ";
        }
        std::cout << "\n";

        return Idle{};
    }


    Action on_new_room(packet_t packet) {
        auto new_room_id = from_packet<pong::packet::NewRoom>(packet).id;
        std::cout << "New room #" << new_room_id << '\n';
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

    Action on_enter_room_response(packet_t packet) {
        auto response = from_packet<pong::packet::EnterRoomResponse>(packet).result;
        if (response == pong::packet::EnterRoomResponse::Okay) {
            return change_state<InRoom>(font);
        } else {
            std::cerr << "Couldn't enter room!\n";
        }

        return Idle{};
    }



};

}