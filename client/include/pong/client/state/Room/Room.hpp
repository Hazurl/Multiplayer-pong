#pragma once

#include <pong/client/state/State.hpp>
#include <pong/client/state/Room/Graphics.hpp>
#include <pong/client/state/Room/Game.hpp>

namespace pong::client::state {

class Room : public State {

public:

    Room(Application app, std::string username);

    action::Actions on_window_event(Application application, WindowEvent const& window_event) override;
    action::Actions on_send(Application application, pong::packet::client::Any const& game_packet) override;
    action::Actions on_receive(Application application, pong::packet::server::Any const& game_packet) override;
    action::Actions on_update(Application application, float dt) override;

    action::Actions on_connection(Application application) override;
    action::Actions on_connection_failure(Application application) override;
    action::Actions on_disconnection(Application application) override;

    void notify_gui(gui::Gui<>& gui) const override;
    void update_properties(gui::Gui<> const& gui) override;
    void free_properties(gui::Allocator<> gui) const override;

    void draw(Application app, sf::RenderTarget& target, sf::RenderStates states) const override;

private:

    void add_to_spectator_count(int additional);
    void update_left_player(std::string username);
    void update_right_player(std::string username);
    void change_role(room::Game::Role role);
    action::Actions on_input(bool up, bool down);
    action::Actions on_button(room::Graphics::Button button);

    room::Graphics graphics;
    room::Game game;

    std::string username;
    room::Game::Role role;
    unsigned spectator_count;
    bool left_player_present;
    bool right_player_present;

    bool up_pressed;
    bool down_pressed;

    unsigned left_score;
    unsigned right_score;

};

}