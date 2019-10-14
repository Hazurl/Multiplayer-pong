#include <pong/client/state/Login.hpp>

#include <pong/client/Logger.hpp>
#include <pong/client/Visitor.hpp>

#include <sftk/print/Printer.hpp>

#include <cmath>

namespace pong::client::state {

Login::Login(Application& app, sf::Color color) {
    rect.setSize({100, 100});
    rect.setPosition({100, 100});
    rect.setFillColor(color);
}

action::Actions Login::on_window_event(Application& application, WindowEvent const& window_event) {
    return std::visit(Visitor{
        [] (KeyPressed const& key) {
            using sftk::operator<<;
            NOTICE("Pressed #", key.code);
            if (key.code == sf::Keyboard::Space) {
                return action::seq(action::connect("127.0.0.1", 48624));
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
    static float t = 0;
    t += dt;
    rect.move({std::sin(t) * 10, 0});

    return action::Actions{};
}

void Login::notify_gui(gui::Gui<>& gui) const {

}

void Login::update_properties(gui::Gui<> const& gui) {

}

void Login::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(rect, states);
}

}