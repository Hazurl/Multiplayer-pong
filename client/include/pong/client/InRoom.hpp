#pragma once

#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>

#include <pong/client/gui/Button.hpp>
#include <pong/client/gui/Text.hpp>
#include <pong/client/gui/Sprite.hpp>

#include <multipong/Game.hpp>

namespace pong::client {

struct Game {
    enum class Playing {
        Left, Right, Spectator
    };


    static constexpr float ball_radius      { 8 };
    static constexpr float pad_height       { 80 };
    static constexpr float pad_width        { 12 };
    static constexpr float pad_padding      { 40 };
    static constexpr float pad_max_speed    { 200 };
    static constexpr float ball_max_speed   { 100 };
    static constexpr float boundaries_x     { 800 };
    static constexpr float boundaries_y     { 600 };
    static constexpr float ball_boundaries_x{ boundaries_x - ball_radius };
    static constexpr float ball_boundaries_y{ boundaries_y - ball_radius };
    static constexpr float pad_boundary     { boundaries_y - pad_height };

    static const     sf::Vector2f boundaries;
    static const     sf::Vector2f ball_boundaries;

    pong::Ball ball;
    pong::Pad pad_left;
    pong::Pad pad_right;

    Game();
    void update(float dt, Playing playing_state, pong::Input input);
};

struct InRoom : State<InRoom> {
    using base_t = State<InRoom>;


    sf::Font const& font;
    gui::Gui<>& gui;
    gui::RectProperties window_properties;

    Game game;

    sf::RectangleShape left_pad;
    sf::RectangleShape right_pad;
    sf::RectangleShape ball;

    Game::Playing playing_state;

    bool is_pressing_down;
    bool is_pressing_up;

    std::string left_player;
    std::string right_player;
    unsigned int spectators_count;

    std::string username;

    gui::Text versus_txt;
    gui::Text left_versus_txt;
    gui::Text right_versus_txt;

    gui::Text score_dash_txt;
    gui::Text left_score_txt;
    gui::Text right_score_txt;

    gui::Text spectator_count_txt;
    sf::Texture spectator_texture;
    gui::Sprite spectator_spr;

    gui::Text quit_txt;
    gui::Button quit_button;

    gui::Text join_queue_txt;
    gui::Button join_queue_button;

    gui::Text leave_queue_txt;
    gui::Button leave_queue_button;

    gui::Text abandon_txt;
    gui::Button abandon_button;

    bool return_to_main_lobby;
    bool is_in_queue;
    bool want_to_abandon;


    InRoom(socket_ptr_t _socket, gui::Gui<>& gui, gui::RectProperties window_properties, sf::Font const& font, std::string username);


    void update_properties(gui::Gui<> const& gui) override;
    void notify_gui(gui::Gui<>& gui) const override;


    Action update(float dt) override;


    void update_graphics();
    void reset_game();
    pong::Input get_input() const;

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;


    sftk::PropagateEvent on_mouse_button_pressed(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_button_released(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_moved(sf::Window&, sf::Event::MouseMoveEvent const& b) override;
    sftk::PropagateEvent on_key_pressed(sf::Window&, sf::Event::KeyEvent const& b) override;
    sftk::PropagateEvent on_key_released(sf::Window&, sf::Event::KeyEvent const& b) override;


    Action on_room_info(packet_t packet);
    Action on_game_state(packet_t packet);
    Action on_new_user(packet_t packet);
    Action on_old_user(packet_t packet);
    Action on_new_player(packet_t packet);
    Action on_old_player(packet_t packet);
    Action on_be_player(packet_t packet);


};

}