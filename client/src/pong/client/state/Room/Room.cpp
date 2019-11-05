#include <pong/client/state/Room/Room.hpp>

#include <pong/client/state/MainLobby/MainLobby.hpp>

#include <pong/client/Logger.hpp>
#include <pong/client/Visitor.hpp>

#include <sftk/print/Printer.hpp>

#include <cmath>

namespace pong::client::state {


pong::Input get_input_from_keys(bool up, bool down) {

    if (up & !down) {
        return pong::Input::Up;
    } 
    
    if (!up && down) {
        return pong::Input::Down;
    }

    return pong::Input::Idle;
}


Room::Room(Application app, std::string _username) 
:   graphics(app)
,   client_state{ Room::ClientState::New }
,   username{ std::move(_username) }
,   role{ room::Game::Role::Spec }
,   spectator_count{ 0 }
,   left_player_present{ false }
,   right_player_present{ false }
,   up_pressed{ false }
,   down_pressed{ false }
,   left_score{ 0 }
,   right_score{ 0 }
{}

action::Actions Room::on_window_event(Application, WindowEvent const& window_event) {
    return std::visit(Visitor{
        [this] (MouseButtonReleased const& event) {
            if (event.button == sf::Mouse::Button::Left) {
                if (auto button = graphics.on_release_click(role, { static_cast<float>(event.x), static_cast<float>(event.y) })) {
                    return on_button(*button);
                }
            }
            return action::idle(); 
        },

        [this] (MouseButtonPressed const& event) {
            if (event.button == sf::Mouse::Button::Left) {
                graphics.on_click(role, { static_cast<float>(event.x), static_cast<float>(event.y) });
            }
            return action::idle(); 
        },

        [this] (MouseMoved const& event) {
            graphics.on_hover({ static_cast<float>(event.x), static_cast<float>(event.y) });
            return action::idle(); 
        },

        [this] (KeyPressed const& event) {
            if (event.code == sf::Keyboard::Up || event.code == sf::Keyboard::W) {
                return on_input(true, down_pressed);
            }

            if (event.code == sf::Keyboard::Down || event.code == sf::Keyboard::S) {
                return on_input(up_pressed, true);
            }

            return action::idle(); 
        },

        [this] (KeyReleased const& event) {
            if (event.code == sf::Keyboard::Up || event.code == sf::Keyboard::W) {
                return on_input(false, down_pressed);
            }

            if (event.code == sf::Keyboard::Down || event.code == sf::Keyboard::S) {
                return on_input(up_pressed, false);
            }

            if (event.code == sf::Keyboard::Num1) {
                return action::seq(action::send(packet::client::AcceptBePlayer{}));
            }

            if (event.code == sf::Keyboard::Num2) {
                return action::seq(action::send(packet::client::LeaveRoom{}));
            }

            return action::idle(); 
        },

        [] (auto const&) {
            return action::idle();
        }
    }, window_event);
}

action::Actions Room::on_send(Application app, pong::packet::client::Any const& game_packet) {
    auto const real_client_state = get_real_state(client_state);

    if (!packet::is_packet_expected_in(real_client_state, game_packet)) {
        ERROR("Sending unexpected packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    if (packet::is_packet_ignored_in(real_client_state, game_packet)) {
        WARN("Sending ignored packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    switch(client_state) {
        case Room::ClientState::New:
            return new_on_send(app, game_packet);

        case Room::ClientState::Spectator:
            return spectator_on_send(app, game_packet);

        case Room::ClientState::Leaving:
            return leaving_on_send(app, game_packet);

        case Room::ClientState::Player:
            return player_on_send(app, game_packet);

        case Room::ClientState::Queued:
            return queued_on_send(app, game_packet);

        case Room::ClientState::NextPlayer:
            return next_player_on_send(app, game_packet);

        case Room::ClientState::AcceptingBePlayer:
            return accepting_be_player_on_send(app, game_packet);
    }

    assert(false && "Unknown ClientState");
    throw std::runtime_error("Unknown ClientState");
/*
    return std::visit(Visitor{
        [this] (pong::packet::client::Abandon const&) {
            if (role == room::Game::Role::Left) {
                update_left_player("");
            } else

            if (role == room::Game::Role::Right) {
                update_right_player("");
            }
            change_role(room::Game::Role::Spec);
            add_to_spectator_count(1);
            return action::idle(); 
        },

        [this] (pong::packet::client::EnterQueue const&) {
            change_role(room::Game::Role::Waiting);
            return action::idle(); 
        },

        [this] (pong::packet::client::LeaveQueue const&) {
            change_role(room::Game::Role::Spec);
            return action::idle(); 
        },

        [] (auto const&) { 
            return action::idle(); 
        }
    }, game_packet);*/
}

action::Actions Room::on_receive(Application app, pong::packet::server::Any const& game_packet) {
    auto const real_client_state = get_real_state(client_state);

    if (!packet::is_packet_expected_in(real_client_state, game_packet)) {
        ERROR("Receiving unexpected packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    if (packet::is_packet_ignored_in(real_client_state, game_packet)) {
        WARN("Receiving ignored packet ", game_packet, " while in the state ", to_string_view(real_client_state));
        return action::idle();
    }

    NOTICE("Received packet ", game_packet, " while in state ", to_string_view(real_client_state));

    switch(client_state) {
        case Room::ClientState::New:
            return new_on_receive(app, game_packet);

        case Room::ClientState::Spectator:
            return spectator_on_receive(app, game_packet);

        case Room::ClientState::Leaving:
            return leaving_on_receive(app, game_packet);

        case Room::ClientState::Player:
            return player_on_receive(app, game_packet);

        case Room::ClientState::Queued:
            return queued_on_receive(app, game_packet);

        case Room::ClientState::NextPlayer:
            return next_player_on_receive(app, game_packet);

        case Room::ClientState::AcceptingBePlayer:
            return accepting_be_player_on_receive(app, game_packet);
    }

    assert(false && "Unknown ClientState");
    throw std::runtime_error("Unknown ClientState");

/*
    return std::visit(Visitor{
        [this] (pong::packet::server::GameState const& game_state) {
            game.ball = game_state.ball;
            game.left = game_state.left;
            game.right = game_state.right;

            return action::idle(); 
        },

        [this] (pong::packet::server::NewUser const& new_user) {
            NOTICE("New user in the room: ", new_user.username);

            add_to_spectator_count(1);

            return action::idle(); 
        },

        [this] (pong::packet::server::OldUser const& old_user) {
            NOTICE("Old user in the room: ", old_user.username);

            add_to_spectator_count(-1);

            return action::idle(); 
        },

        [this] (pong::packet::server::NewPlayer const& new_player) {
            NOTICE("New player in the room: ", new_player.username);

            add_to_spectator_count(-1);

            if (new_player.side == pong::Side::Left) {
                update_left_player(new_player.username);
            } else {
                update_right_player(new_player.username);
            }

            game = room::Game{};
            
            return action::idle(); 
        },

        [this] (pong::packet::server::OldPlayer const& old_player) {
            NOTICE("Old player in the room: ", old_player.username);

            if (old_player.side == pong::Side::Left) {
                update_left_player("");
            } else {
                update_right_player("");
            }
            
            return action::idle(); 
        },

        [this] (pong::packet::server::BePlayer const& be_player) {
            NOTICE("Be player on side: ", (be_player.side == pong::Side::Left ? "left" : "right"));

            add_to_spectator_count(-1);

            if (be_player.side == pong::Side::Left) {
                update_left_player(username);
                change_role(room::Game::Role::Left);
            }
            else {
                update_right_player(username);
                change_role(room::Game::Role::Right);
            }

            game = room::Game{};
            
            return action::idle(); 
        },

        [this] (pong::packet::server::RoomInfo const& room_info) {
            NOTICE("Received ", room_info);

            add_to_spectator_count(room_info.spectators.size() + 1);
            graphics.highlight_spectator_count();
            
            game = room::Game{};

            update_left_player(std::move(room_info.left_player));
            update_right_player(std::move(room_info.right_player));

            return action::idle(); 
        },

        [this, &app] (pong::packet::server::Score const& score) {
            NOTICE("Received ", to_string(score));

            game = room::Game{};
            left_score = score.left;
            right_score = score.right;

            graphics.update_score(app, left_score, right_score);
            
            return action::idle(); 
        },

        [] (auto const& v) { 
            WARN("Unprocessed packet ", v.name);
            return action::idle(); 
        }
    }, game_packet);*/
}

action::Actions Room::on_update(Application app, float dt) {

    if (left_player_present && right_player_present) {
        auto event = game.update(dt, role, get_input_from_keys(up_pressed, down_pressed));
        graphics.update_game(app, game);

        if (event == pong::CollisionEvent::LeftBoundary) {
            ++left_score;
            graphics.update_score(app, left_score, right_score);
            game = room::Game{};
        }
        else if (event == pong::CollisionEvent::RightBoundary) {
            ++right_score;
            graphics.update_score(app, left_score, right_score);
            game = room::Game{};
        }
    }

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
    graphics.draw(left_player_present, right_player_present, role, target, states);
}

void Room::add_to_spectator_count(int additional) {
    if (additional != 0) {
        NOTICE("Spectators += ", additional);
        spectator_count += additional;
    }
    int is_spectator = 
        client_state == Room::ClientState::Spectator
    ||  client_state == Room::ClientState::AcceptingBePlayer
    ||  client_state == Room::ClientState::Queued
    ||  client_state == Room::ClientState::NextPlayer
    ||  client_state == Room::ClientState::Leaving ? 1 : 0;
    graphics.set_spectator_count(spectator_count + is_spectator);
}

void Room::update_left_player(std::string player_username) {
    left_player_present = !player_username.empty();
    graphics.set_left_player(std::move(player_username));
}

void Room::update_right_player(std::string player_username) {
    right_player_present = !player_username.empty();
    graphics.set_right_player(std::move(player_username));
}

void Room::change_role(room::Game::Role next_role) {
    switch(role) {
        case room::Game::Role::Left: {
            graphics.unhighlight_left_player();
            break;
        }
        case room::Game::Role::Right: {
            graphics.unhighlight_right_player();
            break;
        }
        case room::Game::Role::Waiting:
        case room::Game::Role::Spec: {
            graphics.unhighlight_spectator_count();
            break;
        }
    }

    role = next_role;

    switch(role) {
        case room::Game::Role::Left: {
            graphics.highlight_left_player();
            break;
        }
        case room::Game::Role::Right: {
            graphics.highlight_right_player();
            break;
        }
        case room::Game::Role::Waiting:
        case room::Game::Role::Spec: {
            graphics.highlight_spectator_count();
            break;
        }
    }
}

packet::SubState Room::get_real_state(ClientState state) {
    switch(state) {
        case Room::ClientState::New: return packet::SubState::Room_New;
        case Room::ClientState::Spectator: return packet::SubState::Room_Spectator;
        case Room::ClientState::Leaving: return packet::SubState::Room_Leaving;
        case Room::ClientState::Player: return packet::SubState::Room_Player;
        case Room::ClientState::Queued: return packet::SubState::Room_Queued;
        case Room::ClientState::NextPlayer: return packet::SubState::Room_NextPlayer;
        case Room::ClientState::AcceptingBePlayer: return packet::SubState::Room_AcceptingBePlayer;
    }
    assert(false && "Unknown ClientState");
    throw std::runtime_error("Unknown ClientState");
}

void Room::set_state(Room::ClientState next_state) {
    NOTICE("Client State: ", to_string_view(get_real_state(client_state)), " => ", to_string_view(get_real_state(next_state)));
    client_state = next_state;
    add_to_spectator_count(0);
}

action::Actions Room::on_input(bool up, bool down) {
    auto old_input = get_input_from_keys(up_pressed, down_pressed);

    up_pressed = up;
    down_pressed = down;

    auto new_input = get_input_from_keys(up_pressed, down_pressed);

    if (old_input != new_input) {
        return action::seq(action::send(pong::packet::client::Input{ new_input }));
    }

    return action::idle();
}

action::Actions Room::on_button(room::Graphics::Button button) {
    switch(button) {
        case room::Graphics::Button::Quit: {
            NOTICE("Pressed Quit button");
            return action::seq(action::quit());
        }

        case room::Graphics::Button::Abandon: {
            NOTICE("Pressed Abandon button");
            return action::seq(action::send(pong::packet::client::Abandon{}));
        }

        case room::Graphics::Button::JoinQueue: {
            NOTICE("Pressed Join Queue button");
            return action::seq(action::send(pong::packet::client::EnterQueue{}));
        }

        case room::Graphics::Button::LeaveQueue: {
            NOTICE("Pressed Leave Queue button");
            return action::seq(action::send(pong::packet::client::LeaveQueue{}));
        }


        default: WARN("Unprocessed button #", static_cast<int>(button)); break;
    }

    return action::idle();
}






template<typename T>
using require_event_t = std::enable_if_t<std::is_constructible_v<Room::Events, T>, action::Actions>;

template<typename T>
using require_not_event_t = std::enable_if_t<!std::is_constructible_v<Room::Events, T>, action::Actions>;

action::Actions Room::events_on_receive(Application app, Room::Events const& events) {
    return std::visit(Visitor {
        [this, &app] (packet::server::NewUser const& new_user) {
            NOTICE("New user in the room: ", new_user.username);

            add_to_spectator_count(1);

            return action::idle(); 
        },

        [this, &app] (packet::server::OldUser const& old_user) {
            NOTICE("Old user in the room: ", old_user.username);

            add_to_spectator_count(-1);

            return action::idle(); 
        },

        [this, &app] (packet::server::NewPlayer const& new_player) {
            NOTICE("New player in the room: ", new_player.username);

            add_to_spectator_count(-1);

            if (new_player.side == pong::Side::Left) {
                update_left_player(new_player.username);
            } else {
                update_right_player(new_player.username);
            }

            game = room::Game{};
            
            return action::idle(); 
        },

        [this, &app] (packet::server::OldPlayer const& old_player) {
            NOTICE("Old player in the room: ", old_player.username);

            add_to_spectator_count(1);

            if (old_player.side == pong::Side::Left) {
                update_left_player("");
            } else {
                update_right_player("");
            }
            
            return action::idle(); 
        },

        [this, &app] (packet::server::GameState const& game_state) {
            game.ball = game_state.ball;
            game.left = game_state.left;
            game.right = game_state.right;

            return action::idle(); 
        },

        [this, &app] (packet::server::GameOver const& game_over) {
            update_left_player("");
            update_right_player("");
            return action::idle();
        },

        [this, &app] (packet::server::Score const& score) {
            NOTICE("Received ", to_string(score));

            game = room::Game{};
            left_score = score.left;
            right_score = score.right;

            graphics.update_score(app, left_score, right_score);
            
            return action::idle(); 
        }
    }, events);
}






action::Actions Room::new_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (auto const&) {
            return action::idle();
        }
    }, game_packet);
}

action::Actions Room::new_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::server::RoomInfo const& room_info) {
            NOTICE("Received ", room_info);

            add_to_spectator_count(room_info.spectators.size());
            graphics.highlight_spectator_count();
            
            game = room::Game{};

            update_left_player(std::move(room_info.left_player));
            update_right_player(std::move(room_info.right_player));

            set_state(Room::ClientState::Spectator);
            
            return action::idle(); 
        },

        [this, &app] (auto const&) {
            return action::idle();
        }
    }, game_packet);
}





action::Actions Room::spectator_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::client::EnterQueue const&) {
            set_state(Room::ClientState::Queued);
            change_role(room::Game::Role::Waiting);
            return action::idle();
        },

        [this, &app] (packet::client::LeaveRoom const&) {
            set_state(Room::ClientState::Leaving);
            return action::idle();
        },

        [this, &app] (auto const&) {
            return action::idle();
        }
    }, game_packet);
}

action::Actions Room::spectator_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (auto const& event) -> require_event_t<decltype(event)> {
            return events_on_receive(app, event);
        },

        [this, &app] (auto const& non_event) -> require_not_event_t<decltype(non_event)> {
            return action::idle();
        }
    }, game_packet);
}






action::Actions Room::leaving_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (auto const&) {
            return action::idle();
        }
    }, game_packet);
}

action::Actions Room::leaving_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::server::LeaveRoomResponse const& response) {
            if (response.reason == packet::server::LeaveRoomResponse::Reason::Okay) {
                return action::seq(action::change_state<MainLobby>(app, std::move(username)));
            } else {
                set_state(Room::ClientState::Spectator);
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






action::Actions Room::player_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::client::Abandon const& abandon) {
            return action::idle();
        },

        [this, &app] (packet::client::Input const& input) {
            return action::idle();
        },

        [this, &app] (auto const&) {
            return action::idle();
        }
    }, game_packet);
}

action::Actions Room::player_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::server::GameOver const& game_over) {

            switch(game_over.result) {
                case packet::server::GameOver::Result::LeftAbandon: NOTICE("Left player gave up!"); break;
                case packet::server::GameOver::Result::RightAbandon: NOTICE("Right player gave up!"); break;
                case packet::server::GameOver::Result::LeftWin: NOTICE("Left player won!"); break;
                case packet::server::GameOver::Result::RightWin: NOTICE("Right player won!"); break;
            }

            if (role == room::Game::Role::Left && game_over.result == packet::server::GameOver::Result::LeftAbandon
            ||  role == room::Game::Role::Right && game_over.result == packet::server::GameOver::Result::RightAbandon) {
                change_role(room::Game::Role::Spec);
                set_state(Room::ClientState::Spectator);
            } else {
                change_role(room::Game::Role::Waiting);
                set_state(Room::ClientState::Queued);
            }
            return events_on_receive(app, game_over);
        },

        [this, &app] (auto const& event) -> require_event_t<decltype(event)> {
            return events_on_receive(app, event);
        },

        [this, &app] (auto const& non_event) -> require_not_event_t<decltype(non_event)> {
            return action::idle();
        }
    }, game_packet);
}






action::Actions Room::queued_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::client::LeaveQueue const&) {
            set_state(Room::ClientState::Spectator);
            change_role(room::Game::Role::Spec);
            return action::idle();
        },

        [this, &app] (packet::client::LeaveRoom const&) {
            set_state(Room::ClientState::Leaving);
            change_role(room::Game::Role::Spec);
            return action::idle();
        },

        [this, &app] (auto const&) {
            return action::idle();
        }
    }, game_packet);
}

action::Actions Room::queued_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::server::BeNextPlayer const& be_next_player) {
            set_state(Room::ClientState::AcceptingBePlayer);
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






action::Actions Room::next_player_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (auto const&) {
            return action::idle();
        }
    }, game_packet);
}

action::Actions Room::next_player_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::server::DeniedBePlayer const& denied_be_player) {
            set_state(Room::ClientState::Spectator);
            change_role(room::Game::Role::Spec);
            return action::idle();
        },

        [this, &app] (packet::server::BePlayer const& be_player) {
            NOTICE("Be player on side: ", (be_player.side == pong::Side::Left ? "left" : "right"));


            if (be_player.side == pong::Side::Left) {
                update_left_player(username);
                change_role(room::Game::Role::Left);
            }
            else {
                update_right_player(username);
                change_role(room::Game::Role::Right);
            }

            game = room::Game{};
            set_state(Room::ClientState::Player);

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






action::Actions Room::accepting_be_player_on_send(Application app, pong::packet::client::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::client::LeaveRoom const& leave_room) {
            set_state(Room::ClientState::Leaving);
            change_role(room::Game::Role::Spec);
            return action::idle();
        },

        [this, &app] (packet::client::LeaveQueue const& leave_queue) {
            set_state(Room::ClientState::Spectator);
            change_role(room::Game::Role::Spec);
            return action::idle();
        },

        [this, &app] (packet::client::AcceptBePlayer const& accept_be_player) {
            set_state(Room::ClientState::NextPlayer);
            return action::idle();
        },

        [this, &app] (auto const&) {
            return action::idle();
        }
    }, game_packet);
}

action::Actions Room::accepting_be_player_on_receive(Application app, pong::packet::server::Any const& game_packet) {
    return std::visit(Visitor {
        [this, &app] (packet::server::DeniedBePlayer const& denied_be_player) {
            set_state(Room::ClientState::Spectator);
            change_role(room::Game::Role::Spec);
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