#include <pong/client/state/Login/Login.hpp>

#include <pong/client/Logger.hpp>
#include <pong/client/Visitor.hpp>

#include <pong/client/state/MainLobby/MainLobby.hpp>

#include <sftk/print/Printer.hpp>

#include <cmath>

namespace pong::client::state {

Login::Login(Application app) 
:   graphics(app)
,   client_state{ Login::ClientState::Invalid }
{}

action::Actions Login::on_window_event(Application app, WindowEvent const& window_event) {
    return std::visit(Visitor{
        [this] (MouseButtonReleased const& event) {
            if (event.button == sf::Mouse::Button::Left) {
                if (auto button = graphics.on_release_click({ static_cast<float>(event.x), static_cast<float>(event.y) })) {
                    switch(*button) {
                        case login::Graphics::Button::Quit:
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

        [this] (TextEntered const& event) {
            graphics.on_character_entered(event.unicode);
            return action::idle(); 
        },

        [this, &app] (KeyPressed const& event) {
            if (event.code == sf::Keyboard::Left) {
                graphics.cursor_left();
            }

            else if (event.code == sf::Keyboard::Right) {
                graphics.cursor_right();
            }

            else if (event.code == sf::Keyboard::Enter) {
                if (auto valid_username = graphics.validate_username()) {
                    username = std::move(valid_username);
                    if (app.is_connected() || app.is_connecting()) {
                        return action::seq(action::send(pong::packet::client::ChangeUsername{ *username }));
                    } else {
                        return action::seq(action::connect("127.0.0.1", 48624));
                    }
                }
            }

            return action::idle(); 
        },

        [] (auto const&) {
            return action::idle();
        }
    }, window_event);
}

action::Actions Login::on_send(Application app, pong::packet::client::Any const& game_packet) {
    auto const real_client_state = client_state == Login::ClientState::Invalid ? packet::SubState::NewUser_Invalid : packet::SubState::NewUser_Connecting;

    if (!packet::is_packet_expected_in(real_client_state, game_packet)) {
        ERROR("Sending unexpected packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    if (packet::is_packet_ignored_in(real_client_state, game_packet)) {
        WARN("Sending ignored packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    switch(client_state) {
        case Login::ClientState::Invalid:
            return invalid_on_send(app, game_packet);

        case Login::ClientState::Connecting:
            return connecting_on_send(app, game_packet);
    }

    assert(false && "Unknown ClientState");
    throw std::runtime_error("Unknown ClientState");
}

action::Actions Login::on_receive(Application app, pong::packet::server::Any const& game_packet) {
    auto const real_client_state = client_state == Login::ClientState::Invalid ? packet::SubState::NewUser_Invalid : packet::SubState::NewUser_Connecting;

    if (!packet::is_packet_expected_in(real_client_state, game_packet)) {
        ERROR("Receiving unexpected packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    if (packet::is_packet_ignored_in(real_client_state, game_packet)) {
        WARN("Receiving ignored packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    switch(client_state) {
        case Login::ClientState::Invalid:
            return invalid_on_receive(app, game_packet);

        case Login::ClientState::Connecting:
            return connecting_on_receive(app, game_packet);
    }

    assert(false && "Unknown ClientState");
    throw std::runtime_error("Unknown ClientState");
}

action::Actions Login::on_update(Application app, float dt) {
    graphics.update_animations(app, dt);
    return action::idle();
}

action::Actions Login::on_connection(Application) {
    assert(username.has_value());

    auto actions = action::seq(action::send(pong::packet::client::ChangeUsername{ *username }));
    return actions;
}

action::Actions Login::on_connection_failure(Application) {
    ERROR("Connection Failure");
    username = std::nullopt;
    return action::idle();
}

action::Actions Login::on_disconnection(Application) {
    ERROR("Unexpected disconnection");
    username = std::nullopt;
    return action::idle();
}

void Login::notify_gui(gui::Gui<>& gui) const {
    graphics.notify_gui(gui);
}

void Login::update_properties(gui::Gui<> const& gui) {
    graphics.update_properties(gui);
}

void Login::free_properties(gui::Allocator<> gui) const {
    graphics.free_properties(gui);
}

void Login::draw(Application app, sf::RenderTarget& target, sf::RenderStates states) const {
    graphics.draw(app.is_connecting() || client_state == Login::ClientState::Connecting, target, states);
}







action::Actions Login::invalid_on_send(Application, pong::packet::client::Any const& game_packet) {
    if (std::holds_alternative<packet::client::ChangeUsername>(game_packet)) {
        client_state = Login::ClientState::Connecting;
    }

    return action::idle();
}

action::Actions Login::invalid_on_receive(Application, pong::packet::server::Any const&) {
    return action::idle();
}







action::Actions Login::connecting_on_send(Application, pong::packet::client::Any const&) {
    return action::idle();
}

action::Actions Login::connecting_on_receive(Application application, pong::packet::server::Any const& game_packet) {
    if (auto* response = std::get_if<packet::server::ChangeUsernameResponse>(&game_packet)) {
        if (response->valid) {
            return action::seq(action::change_state<MainLobby>(application, std::move(*username)));
        } else {
            client_state = Login::ClientState::Invalid;
            return action::idle();
        }
    }

    return action::idle();
}


}