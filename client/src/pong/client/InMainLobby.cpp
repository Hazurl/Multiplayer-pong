#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>
#include <pong/client/InRoom.hpp>

#include <pong/client/gui/Button.hpp>

#include <pong/client/InMainLobby.hpp>

namespace pong::client {


InMainLobby::InMainLobby(socket_ptr_t _socket, gui::Gui<>& _gui, gui::RectProperties _window_properties, sf::Font const& _font, std::string _username) 
:   InMainLobby::base_t(std::move(_socket), {
        { pong::packet::PacketID::LobbyInfo, &InMainLobby::on_lobby_info },
        { pong::packet::PacketID::NewRoom, &InMainLobby::on_new_room },
        { pong::packet::PacketID::NewUser, &InMainLobby::on_new_user },
        { pong::packet::PacketID::OldUser, &InMainLobby::on_old_user },
        { pong::packet::PacketID::EnterRoomResponse, &InMainLobby::on_enter_room_response }
    })
,   font{ _font }
,   gui{ _gui }
,   window_properties{ _window_properties }
,   button(gui, [this] () { on_button_click(); })
,   username{ std::move(_username) }
{
    button.setSize({200, 40});
    button.set_rounded_radius(2);

    gui.set_constraint<
        [] (float window_left, float window_width, float width) {
            return window_left + (window_width - width) * 0.5f;
        }
    >(button.left(), { window_properties.left(), window_properties.width(),  button.width() });

    gui.set_constraint<
        [] (float window_top, float window_height, float height) {
            return window_top + window_height - height - 20.f;
        }
    >(button.top(), { window_properties.top(), window_properties.height(),  button.height() });
}

sftk::PropagateEvent InMainLobby::on_mouse_button_pressed(sf::Window& w, sf::Event::MouseButtonEvent const& b) {
    return button.on_mouse_button_pressed(w, b) 
    &&  std::all_of(
            std::begin(join_room_buttons), 
            std::end(join_room_buttons), 
            [&w, b] (auto& but) { 
                return but.on_mouse_button_pressed(w, b); 
            });
}

sftk::PropagateEvent InMainLobby::on_mouse_button_released(sf::Window& w, sf::Event::MouseButtonEvent const& b) {
    return button.on_mouse_button_released(w, b)
    &&  std::all_of(
            std::begin(join_room_buttons), 
            std::end(join_room_buttons), 
            [&w, b] (auto& but) { 
                return but.on_mouse_button_released(w, b); 
            });
}

sftk::PropagateEvent InMainLobby::on_mouse_moved(sf::Window& w, sf::Event::MouseMoveEvent const& b) {
    return button.on_mouse_moved(w, b)
    &&  std::all_of(
            std::begin(join_room_buttons), 
            std::end(join_room_buttons), 
            [&w, b] (auto& but) { 
                return but.on_mouse_moved(w, b); 
            });
}


void InMainLobby::update_properties(gui::Gui<> const& _gui) {
    button.update_properties(_gui);
    for(auto& b : join_room_buttons) {
        b.update_properties(_gui);
    }
}

void InMainLobby::notify_gui(gui::Gui<>& _gui) const {
    button.notify_gui(_gui);
    //std::cout << "NOTIFY GUI!\n";
    for(auto& b : join_room_buttons) {
        b.notify_gui(_gui);
    }
}


Action InMainLobby::update(float dt) {
    button.update(dt);
    for(auto& b : join_room_buttons) {
        b.update(dt);
    }

    return Idle{};
}


void InMainLobby::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    target.draw(button, states);
    for(auto& b : join_room_buttons) {
        target.draw(b, states);    
    }
}



void InMainLobby::on_button_click() {
    send_notify([this] () {
        return change_state<InRoom>(gui, window_properties, font, this->username);
    }, pong::packet::CreateRoom{});
}




Action InMainLobby::on_lobby_info(packet_t packet) {
    auto lobby_info = from_packet<pong::packet::LobbyInfo>(packet);

    std::cout << "Users: ";
    for(auto const& user : lobby_info.users) {
        std::cout << user << " ";
    }
    std::cout << "\nRooms: ";
    for(auto room : lobby_info.rooms) {
        auto& b = join_room_buttons.emplace_back(
            gui,
            [this, room] () { send(pong::packet::EnterRoom{ room }); }
        );

        if (join_room_buttons.size() <= 1) {
            gui.set_constraint<[] (float window_top) {
                return window_top + 10;
            }>(b.top(), { window_properties.top() });
        } else {
            auto& last = join_room_buttons[join_room_buttons.size() - 2];
            gui.set_constraint<[] (float prev_top, float prev_height) {
                return prev_top + prev_height + 10;
            }>(b.top(), { last.top(), last.height() });
        }

        gui.set_constraint<[] (float window_left, float window_width, float width) {
            return window_left + window_width - width - 10;
        }>(b.left(), { window_properties.left(), window_properties.width(), b.width() });


        b.setSize({ 150, 30 });
        std::cout << room << " ";
    }
    std::cout << "\n";

    return Idle{};
}


Action InMainLobby::on_new_room(packet_t packet) {
    auto new_room_id = from_packet<pong::packet::NewRoom>(packet).id;
    std::cout << "New room #" << new_room_id << '\n';
    return Idle{};
}


Action InMainLobby::on_new_user(packet_t packet) {
    auto new_user = from_packet<pong::packet::NewUser>(packet).username;
    std::cout << "New user " << new_user << '\n';
    return Idle{};
}

Action InMainLobby::on_old_user(packet_t packet) {
    auto old_user = from_packet<pong::packet::OldUser>(packet).username;
    std::cout << "Old user " << old_user << '\n';
    return Idle{};
}

Action InMainLobby::on_enter_room_response(packet_t packet) {
    auto response = from_packet<pong::packet::EnterRoomResponse>(packet).result;
    if (response == pong::packet::EnterRoomResponse::Okay) {
        std::cout << "Enter room\n";
        return change_state<InRoom>(gui, window_properties, font, this->username);
    } else {
        std::cerr << "Couldn't enter room!\n";
    }

    return Idle{};
}

}