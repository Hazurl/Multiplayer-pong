#pragma once

#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>
#include <pong/client/InMainLobby.hpp>

#include <pong/client/gui/Text.hpp>

namespace pong::client {

struct LoginIn : State<LoginIn> {
    using base_t = State<LoginIn>;

    sf::Font const& font;
    gui::Gui<>& gui;
    gui::RectProperties window_properties;

    gui::Text title_txt;
    gui::Text login_txt;
    gui::Text pseudo_txt;
    gui::Text connecting_txt;
    gui::Text by_hazurl_txt;
    gui::Text quit_txt;
    gui::Button quit_button;
    gui::RoundedRectangle pseudo_cursor;
    std::size_t cursor_index;

    bool quit;


    LoginIn(socket_ptr_t _socket, gui::Gui<>& gui, gui::RectProperties window_properties, sf::Font const& font);


    void update_properties(gui::Gui<> const& gui) override;
    void notify_gui(gui::Gui<>& gui) const override;


    Action update(float dt) override;
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;


    sftk::PropagateEvent on_mouse_button_pressed(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_button_released(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_moved(sf::Window&, sf::Event::MouseMoveEvent const& b) override;
    sftk::PropagateEvent on_key_pressed(sf::Window&, sf::Event::KeyEvent const& b) override;
    sftk::PropagateEvent on_text_entered(sf::Window&, sf::Event::TextEvent const& b) override;


    void on_connection() override;


    Action on_username_response(packet_t packet);

};

}