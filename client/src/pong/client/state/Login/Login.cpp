#include <pong/client/state/Login/Login.hpp>

#include <pong/client/Logger.hpp>
#include <pong/client/Visitor.hpp>

#include <sftk/print/Printer.hpp>

#include <cmath>

namespace pong::client::state {

Login::Login(Application& app) 
:   graphics(app)
{}

action::Actions Login::on_window_event(Application& application, WindowEvent const& window_event) {
    return std::visit(Visitor{
        [this] (MouseButtonReleased const& event) {
            if (event.button == sf::Mouse::Button::Left) {
                if (auto button = graphics.on_click(static_cast<float>(event.x), static_cast<float>(event.y))) {
                    switch(*button) {
                        case login::Graphics::Button::Quit:
                            return action::seq(action::quit())

                        default: break;
                    }
                }
            }
            return action::Actions{}; 
        },
        [this] (MouseButtonPressed const& event) {
            if (event.button == sf::Mouse::Button::Left) {
                graphics.on_click(static_cast<float>(event.x), static_cast<float>(event.y));
            }
            return action::Actions{}; 
        },
        [] (auto const&) {
            return action::Actions{};
        }
    }, window_event);
}

action::Actions Login::on_send(Application& application, pong::packet::GamePacket const& game_packet) {
    return action::Actions{};
}

action::Actions Login::on_receive(Application& application, pong::packet::GamePacket const& game_packet) {
    return action::Actions{};
}

action::Actions Login::on_update(Application& application, float dt) {
    graphics.update_animations(dt);
    return action::Actions{};
}

void Login::notify_gui(gui::Gui<>& gui) const {
    graphics.notify_gui(gui);
}

void Login::update_properties(gui::Gui<> const& gui) {
    graphics.update_properties(gui);
}

void Login::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(graphics, states);
}

}