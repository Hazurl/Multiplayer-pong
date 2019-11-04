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
,   client_state{ MainLobby::ClientState::New }
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
    auto const real_client_state = [] (auto state) {
        switch(state) {
            case MainLobby::ClientState::New: return packet::SubState::Lobby_New;
            case MainLobby::ClientState::Regular: return packet::SubState::Lobby_RegularUser;
            case MainLobby::ClientState::EnteringRom: return packet::SubState::Lobby_EnteringRoom;
            case MainLobby::ClientState::CreatingRoom: return packet::SubState::Lobby_CreatingRoom;
        }
        assert(false && "Unknown ClientState");
        throw std::runtime_error("Unknown ClientState");
    }(client_state);

    if (!packet::is_packet_expected_in(real_client_state, game_packet)) {
        ERROR("Sending unexpected packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    if (packet::is_packet_ignored_in(real_client_state, game_packet)) {
        WARN("Sending ignored packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    switch(client_state) {
        case MainLobby::ClientState::New:
            return new_on_send(app, game_packet);

        case MainLobby::ClientState::Regular:
            return regular_on_send(app, game_packet);

        case MainLobby::ClientState::EnteringRom:
            return entering_room_on_send(app, game_packet);

        case MainLobby::ClientState::CreatingRoom:
            return creating_room_on_send(app, game_packet);
    }

    assert(false && "Unknown ClientState");
    throw std::runtime_error("Unknown ClientState");
}

action::Actions MainLobby::on_receive(Application app, pong::packet::server::Any const& game_packet) {
    auto const real_client_state = [] (auto state) {
        switch(state) {
            case MainLobby::ClientState::New: return packet::SubState::Lobby_New;
            case MainLobby::ClientState::Regular: return packet::SubState::Lobby_RegularUser;
            case MainLobby::ClientState::EnteringRom: return packet::SubState::Lobby_EnteringRoom;
            case MainLobby::ClientState::CreatingRoom: return packet::SubState::Lobby_CreatingRoom;
        }
        assert(false && "Unknown ClientState");
        throw std::runtime_error("Unknown ClientState");
    }(client_state);

    if (!packet::is_packet_expected_in(real_client_state, game_packet)) {
        ERROR("Receiving unexpected packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    if (packet::is_packet_ignored_in(real_client_state, game_packet)) {
        WARN("Receiving ignored packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    switch(client_state) {
        case MainLobby::ClientState::New:
            return new_on_receive(app, game_packet);

        case MainLobby::ClientState::Regular:
            return regular_on_receive(app, game_packet);

        case MainLobby::ClientState::EnteringRom:
            return entering_room_on_receive(app, game_packet);

        case MainLobby::ClientState::CreatingRoom:
            return creating_room_on_receive(app, game_packet);
    }

    assert(false && "Unknown ClientState");
    throw std::runtime_error("Unknown ClientState");
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





template<typename T>
using require_event_t = std::enable_if_t<std::is_constructible_v<MainLobby::Events, T>, action::Actions>;

template<typename T>
using require_not_event_t = std::enable_if_t<!std::is_constructible_v<MainLobby::Events, T>, action::Actions>;

action::Actions MainLobby::events_on_receive(Application app, MainLobby::Events const& events) {
    return std::visit(Visitor{
        [this, &app] (packet::server::NewUser const& new_user) {
            return action::idle();
        },

        [this, &app] (packet::server::OldUser const& old_user) {
            return action::idle();
        },

        [this, &app] (packet::server::NewRoom const& new_room) {
            return action::idle();
        },

        [this, &app] (packet::server::OldRoom const& old_room) {
            return action::idle();
        },

        [this, &app] (packet::server::RoomInfo const& room_info) {
            return action::idle();
        }
    }, events);
}






action::Actions MainLobby::new_on_send(Application application, pong::packet::client::Any const& game_packet) {
    return action::idle();
}

action::Actions MainLobby::new_on_receive(Application application, pong::packet::server::Any const& game_packet) {

    if (auto* lobby_info = std::get_if<packet::server::LobbyInfo>(&game_packet)) {
        client_state = MainLobby::ClientState::Regular;
    }

    return action::idle();
}






action::Actions MainLobby::regular_on_send(Application app, pong::packet::client::Any const& game_packet) {
    if (std::holds_alternative<packet::client::EnterRoom>(game_packet)) {
        client_state = MainLobby::ClientState::EnteringRom;
    }

    else if (std::holds_alternative<packet::client::CreateRoom>(game_packet)) {
        client_state = MainLobby::ClientState::CreatingRoom;
    }

    else if (auto* subscription = std::get_if<packet::client::SubscribeRoomInfo>(&game_packet)) {
        SUCCESS("Subscribed to [", subscription->range_min, ", ", subscription->range_max_excluded, ")");
    }

    return action::idle();
}

action::Actions MainLobby::regular_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (auto const& event) -> require_event_t<decltype(event)> {
            return events_on_receive(app, event);
        },

        [this, &app] (auto const& non_event) -> require_not_event_t<decltype(non_event)> {
            return action::idle();
        }
    }, game_packet);
}






action::Actions MainLobby::entering_room_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return action::idle();
}

action::Actions MainLobby::entering_room_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::server::EnterRoomResponse const& response) {
            if (response.result == packet::server::EnterRoomResponse::Result::Okay) {
                return action::seq(action::change_state<Room>(app, std::move(username)));
            } else {
                client_state = MainLobby::ClientState::Regular;
            }

            return action::idle();
        },

        [this, &app] (auto const& event) -> require_event_t<decltype(event)> {
            return events_on_receive(app, event);
        },

        [this, &app] (auto const& non_event) -> require_not_event_t<decltype(non_event)> {
            return action::idle();
        }
    }, game_packet);
}






action::Actions MainLobby::creating_room_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return action::idle();
}

action::Actions MainLobby::creating_room_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::server::CreateRoomResponse const& response) {
            if (response.reason == packet::server::CreateRoomResponse::Reason::Okay) {
                return action::seq(action::change_state<Room>(app, std::move(username)));
            } else {
                client_state = MainLobby::ClientState::Regular;
            }

            return action::idle();
        },

        [this, &app] (auto const& event) -> require_event_t<decltype(event)> {
            return events_on_receive(app, event);
        },

        [this, &app] (auto const& non_event) -> require_not_event_t<decltype(non_event)> {
            return action::idle();
        }
    }, game_packet);
}

}