#pragma once

#include <pong/client/state/State.hpp>
#include <pong/client/state/Login/Graphics.hpp>
#include <pong/client/state/Login/Network.hpp>

namespace pong::client::state {

class Login : public State {

public:

    Login(Application& app);

    action::Actions on_window_event(Application& application, WindowEvent const& window_event) override;
    action::Actions on_send(Application& application, pong::packet::GamePacket const& game_packet) override;
    action::Actions on_receive(Application& application, pong::packet::GamePacket const& game_packet) override;
    action::Actions on_update(Application& application, float dt) override;

    void notify_gui(gui::Gui<>& gui) const override;
    void update_properties(gui::Gui<> const& gui) override;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:

    login::Graphics graphics;
    login::Network network;

};

}