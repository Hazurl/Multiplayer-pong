#include <pong/client/state/MainLobby/MainLobby.hpp>
#include <pong/client/state/Room/Room.hpp>

#include <pong/client/Logger.hpp>
#include <pong/client/Visitor.hpp>

#include <sftk/print/Printer.hpp>

#include <cmath>

namespace pong::client::state {

MainLobby::MainLobby(Application app, std::string _username) 
:   graphics(app)
,   username{ std::move(_username) }
{}

action::Actions MainLobby::on_window_event(Application, WindowEvent const& window_event) {
    return std::visit(Visitor{
        [this] (MouseButtonReleased const& event) {
            if (event.button == sf::Mouse::Button::Left) {
                if (auto button = graphics.on_release_click({ static_cast<float>(event.x), static_cast<float>(event.y) })) {
                    switch(*button) {
                        case mainlobby::Graphics::Button::Quit:
                            NOTICE("Pressed Quit button");
                            return action::seq(action::quit());

                        case mainlobby::Graphics::Button::CreateRoom:
                            NOTICE("Pressed Create room button");
                            return action::seq(action::send(pong::packet::client::CreateRoom{}));

                        default: break;
                    }
                }
            }
            return action::idle(); 
        },

        [this] (MouseButtonPressed const& event) {
            if (event.button == sf::Mouse::Button::Left) {
                graphics.on_click({ static_cast<float>(event.x), static_cast<float>(event.y) });
            }
            return action::idle(); 
        },

        [this] (MouseMoved const& event) {
            graphics.on_hover({ static_cast<float>(event.x), static_cast<float>(event.y) });
            return action::idle(); 
        },

        [this] (KeyPressed const& event) {
            if (event.code == sf::Keyboard::Space) {
                return action::seq(action::send(pong::packet::client::EnterRoom{ 0 }));
            }
            return action::idle();
        },

        [] (auto const&) {
            return action::idle();
        }
    }, window_event);
}

action::Actions MainLobby::on_send(Application app, pong::packet::client::Any const& game_packet) {
    return std::visit(Visitor{
        [&] (pong::packet::client::CreateRoom const& /* create_room */) {
            NOTICE("Change state to spectator");
            return action::seq(action::change_state<Room>(app, std::move(username)));
        },

        [] (auto const&) { return action::idle(); }
    }, game_packet);
}

action::Actions MainLobby::on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor{
        [this] (pong::packet::server::LobbyInfo const& /* lobby_info */) {
            NOTICE("Received Lobby info");
            return action::idle();
        },

        [&] (pong::packet::server::EnterRoomResponse const& response) {
            if (response.result == pong::packet::server::EnterRoomResponse::Okay) {
                NOTICE("Change state to spectator");
                return action::seq(action::change_state<Room>(app, std::move(username)));
            } else {
                WARN("EnterRoomResponse #", static_cast<int>(response.result));
                return action::idle();
            }
        },

        [] (auto const&) { return action::idle(); }
    }, game_packet);
}

action::Actions MainLobby::on_update(Application app, float dt) {
    graphics.update_animations(app, dt);
    return action::idle();
}

action::Actions MainLobby::on_connection(Application) {
    return action::idle();
}

action::Actions MainLobby::on_connection_failure(Application) {
    return action::idle();
}

action::Actions MainLobby::on_disconnection(Application) {
    return action::idle();
}

void MainLobby::notify_gui(gui::Gui<>& gui) const {
    graphics.notify_gui(gui);
}

void MainLobby::update_properties(gui::Gui<> const& gui) {
    graphics.update_properties(gui);
}

void MainLobby::free_properties(gui::Allocator<> gui) const {
    graphics.free_properties(gui);
}

void MainLobby::draw(Application app, sf::RenderTarget& target, sf::RenderStates states) const {
    graphics.draw(target, states);
}

}