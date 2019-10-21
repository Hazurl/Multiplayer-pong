#pragma once

#include <pong/client/net/PacketQueue.hpp>
#include <pong/client/net/Receiver.hpp>
#include <pong/client/net/Status.hpp>

#include <pong/client/Application.hpp>
#include <pong/client/Action.hpp>
#include <pong/client/Logger.hpp>
#include <pong/client/WindowEvent.hpp>

#include <multipong/Packets.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

namespace pong::client::state {

struct State {

    virtual ~State() = default;

    virtual action::Actions on_window_event(Application application, WindowEvent const& window_event) = 0;
    virtual action::Actions on_send(Application application, pong::packet::client::Any const& game_packet) = 0;
    virtual action::Actions on_receive(Application application, pong::packet::server::Any const& game_packet) = 0;
    virtual action::Actions on_update(Application application, float dt) = 0;

    virtual action::Actions on_connection(Application application) = 0;
    virtual action::Actions on_connection_failure(Application application) = 0;
    virtual action::Actions on_disconnection(Application application) = 0;

    virtual void notify_gui(gui::Gui<>& gui) const = 0;
    virtual void update_properties(gui::Gui<> const& gui) = 0;
    virtual void free_properties(gui::Allocator<> gui) const = 0;

    virtual void draw(Application app, sf::RenderTarget& target, sf::RenderStates states) const = 0;

};

}