#pragma once

#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>

namespace pong::client {

struct InMainLobby : State<InMainLobby> {
    using base_t = State<InMainLobby>;

    using typename base_t::receiver_t;
    using typename base_t::receiver_map_t;
    using typename base_t::abord_connection_t;

    sf::RectangleShape button;

    InMainLobby(socket_ref_t _socket) 
    :   base_t(_socket, {
            { pong::packet::PacketID::LobbyInfo, &InMainLobby::on_lobby_info }
        })
    ,   button({ 100, 40 })
    {
        button.setPosition(100, 100);
        button.setFillColor(sf::Color::White);
    }



    sftk::PropagateEvent on_mouse_button_pressed(sftk::event::MouseButtonPressed const& b) override {
        if (button.getGlobalBounds().contains(static_cast<float>(b.x), static_cast<float>(b.y))) {
            button.setFillColor(sf::Color::Red);
            send(pong::packet::CreateRoom{});
        }

        return false;
    }

    sftk::PropagateEvent on_mouse_button_released(sftk::event::MouseButtonReleased const& b) override {
        button.setFillColor(sf::Color::White);
        return true;
    }

    sftk::PropagateEvent on_mouse_moved(sftk::event::MouseMoved const& b) override {
        if (button.getGlobalBounds().contains(static_cast<float>(b.x), static_cast<float>(b.y))) {
            button.setFillColor(sf::Color::Green);
        } else {
            button.setFillColor(sf::Color::White);
        }
        return true;
    }


    void update(float dt) override {
        
    }


    void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
        target.draw(button, states);
    }




    Action on_lobby_info(packet_t packet) {
        auto lobby_info = from_packet<pong::packet::LobbyInfo>(packet);

        std::cout << "Users: ";
        for(auto const& user : lobby_info.users) {
            std::cout << user << " ";
        }
        std::cout << "\nRooms: ";
        for(auto room : lobby_info.rooms) {
            std::cout << room << " ";
        }
        std::cout << "\n";

        return Idle{};
    }


    Action on_new_room(packet_t packet) {
        auto new_room_id = from_packet<pong::packet::NewRoom>(packet).id;
        std::cout << "New room #" << new_room_id << '\n';
    }


    Action on_new_user(packet_t packet) {
        auto new_user = from_packet<pong::packet::NewUser>(packet).username;
        std::cout << "New user " << new_user << '\n';
    }

    Action on_old_user(packet_t packet) {
        auto old_user = from_packet<pong::packet::OldUser>(packet).username;
        std::cout << "Old user " << old_user << '\n';
    }



};

}