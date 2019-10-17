#pragma once

#include <pong/client/state/State.hpp>
#include <pong/client/state/Room/Graphics.hpp>
#include <pong/client/state/Room/Game.hpp>

namespace pong::client::state {

class Room : public State {

public:

    Room(Application app, std::string username);

    action::Actions on_window_event(Application application, WindowEvent const& window_event) override;
    action::Actions on_send(Application application, pong::packet::GamePacket const& game_packet) override;
    action::Actions on_receive(Application application, pong::packet::GamePacket const& game_packet) override;
    action::Actions on_update(Application application, float dt) override;

    action::Actions on_connection(Application application) override;
    action::Actions on_connection_failure(Application application) override;
    action::Actions on_disconnection(Application application) override;

    void notify_gui(gui::Gui<>& gui) const override;
    void update_properties(gui::Gui<> const& gui) override;
    void free_properties(gui::Allocator<> gui) const override;

    void draw(Application app, sf::RenderTarget& target, sf::RenderStates states) const override;

private:

    room::Graphics graphics;
    room::Game game;

    std::string username;
    room::Game::Role role;

};

}