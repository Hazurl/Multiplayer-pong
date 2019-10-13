#pragma once

#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>
#include <pong/client/InRoom.hpp>

#include <pong/client/gui/Button.hpp>

namespace pong::client {

struct InMainLobby : State<InMainLobby> {
    using base_t = State<InMainLobby>;

    sf::Font const& font;
    gui::Gui<>& gui;
    gui::RectProperties window_properties;
    gui::Button button;
    std::vector<gui::Button> join_room_buttons;

    std::string username;

    InMainLobby(socket_ptr_t _socket, gui::Gui<>& gui, gui::RectProperties window_properties, sf::Font const& font, std::string username);


    void update_properties(gui::Gui<> const& gui) override;
    void notify_gui(gui::Gui<>& gui) const override;


    sftk::PropagateEvent on_mouse_button_pressed(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_button_released(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_moved(sf::Window&, sf::Event::MouseMoveEvent const& b) override;


    Action update(float dt) override;

private:
    void on_button_click();

public:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;




    Action on_lobby_info(packet_t packet);
    Action on_new_room(packet_t packet);
    Action on_new_user(packet_t packet);
    Action on_old_user(packet_t packet);
    Action on_enter_room_response(packet_t packet);



};

}