#include <pong/client/state/Room/Room.hpp>

#include <pong/client/Logger.hpp>
#include <pong/client/Visitor.hpp>

#include <sftk/print/Printer.hpp>

#include <cmath>

namespace pong::client::state {

Room::Room(Application app, std::string _username) 
:   graphics(app)
,   username{ std::move(_username) }
,   role{ room::Game::Role::Spec }
{}

action::Actions Room::on_window_event(Application, WindowEvent const& window_event) {
    return std::visit(Visitor{
        [this] (MouseButtonReleased const& event) {
            if (event.button == sf::Mouse::Button::Left) {
                if (auto button = graphics.on_release_click({ static_cast<float>(event.x), static_cast<float>(event.y) })) {
                    switch(*button) {
                        case room::Graphics::Button::Quit:
                            NOTICE("Pressed Quit button");
                            return action::seq(action::quit());

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

        [] (auto const&) {
            return action::idle();
        }
    }, window_event);
}

action::Actions Room::on_send(Application, pong::packet::GamePacket const& game_packet) {
    return action::idle();
}

action::Actions Room::on_receive(Application, pong::packet::GamePacket const& game_packet) {
    return std::visit(Visitor{
        [] (auto const&) { return action::idle(); }
    }, game_packet);
}

action::Actions Room::on_update(Application app, float dt) {
    game.update(dt, role, pong::Input::Idle);
    graphics.update_game(app, game);

    graphics.update_animations(app, dt);

    return action::idle();
}

action::Actions Room::on_connection(Application) {
    return action::idle();
}

action::Actions Room::on_connection_failure(Application) {
    return action::idle();
}

action::Actions Room::on_disconnection(Application) {
    return action::idle();
}

void Room::notify_gui(gui::Gui<>& gui) const {
    graphics.notify_gui(gui);
}

void Room::update_properties(gui::Gui<> const& gui) {
    graphics.update_properties(gui);
}

void Room::free_properties(gui::Allocator<> gui) const {
    graphics.free_properties(gui);
}

void Room::draw(Application app, sf::RenderTarget& target, sf::RenderStates states) const {
    graphics.draw(target, states);
}

}