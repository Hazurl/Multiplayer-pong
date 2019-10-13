#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>

#include <pong/client/gui/Button.hpp>

#include <pong/client/InRoom.hpp>
#include <pong/client/InMainLobby.hpp>

namespace pong::client {


Game::Game()
:   ball{ sf::Vector2f{ ball_boundaries_x, ball_boundaries_y } / 2.f, { ball_max_speed, ball_max_speed }}
,   pad_left{ pad_boundary / 2.f, 0 }
,   pad_right{ pad_boundary / 2.f, 0 }
{
    std::cout << "Game::INIT\n";
}

void Game::update(float dt, Playing playing_state, pong::Input input) {
    ball.update(dt, pad_left.y, pad_right.y, { boundaries_x, boundaries_y }, pad_padding, pad_height, pad_width, ball_radius);
    if (playing_state == Playing::Left) {
        pad_left.update(dt, input, pad_max_speed, pad_boundary);
    } else {
        pad_left.update(dt, pad_boundary);
    }
    if (playing_state == Playing::Right) {
        pad_right.update(dt, input, pad_max_speed, pad_boundary);
    } else {
        pad_right.update(dt, pad_boundary);
    }
}



InRoom::InRoom(socket_ptr_t _socket, gui::Gui<>& _gui, gui::RectProperties _window_properties, sf::Font const& _font, std::string username) 
:   InRoom::base_t(std::move(_socket), {
        { pong::packet::PacketID::GameState, &InRoom::on_game_state },
        { pong::packet::PacketID::RoomInfo,  &InRoom::on_room_info },
        { pong::packet::PacketID::NewUser,   &InRoom::on_new_user },
        { pong::packet::PacketID::OldUser,   &InRoom::on_old_user },
        { pong::packet::PacketID::NewPlayer, &InRoom::on_new_player },
        { pong::packet::PacketID::OldPlayer, &InRoom::on_old_player },
        { pong::packet::PacketID::BePlayer,  &InRoom::on_be_player }
    })
// GameState, NewUser, OldUser, NewPlayer, OldPlayer, BePlayer
,   font{ _font }
,   gui{ _gui }
,   window_properties{ _window_properties }
,   left_pad{{ Game::pad_width, Game::pad_height }}
,   right_pad{{ Game::pad_width, Game::pad_height }}
,   ball{{ Game::ball_radius, Game::ball_radius }}
,   playing_state{ Game::Playing::Spectator }
,   left_player{ "" }
,   right_player{ "" }
,   spectators_count{ 0 }
,   username{ std::move(username) }
,   versus_txt{ gui, "vs", font, 15 }
,   left_versus_txt{ gui, "", font, 30 }
,   right_versus_txt{ gui, "", font, 30 }
,   score_dash_txt{ gui, "-", font, 40 }
,   left_score_txt{ gui, "2", font, 80 }
,   right_score_txt{ gui, "1", font, 80 }
,   spectator_count_txt{ gui, "0", font, 20 }
,   spectator_texture{}
,   spectator_spr{ gui, spectator_texture }
,   quit_txt(gui, "QUIT", font, 20)
,   quit_button(gui, 
        [this] () {
            if (playing_state == Game::Playing::Spectator) {
                send_notify([this] () {
                    is_in_queue = false;
                    return_to_main_lobby = true;
                    return Idle{};
                }, pong::packet::LeaveQueue{});
            }
        },
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   join_queue_txt(gui, "JOIN QUEUE", font, 20)
,   join_queue_button(gui, 
        [this] () {
            if (playing_state == Game::Playing::Spectator) {
                send_notify([this] () {
                    is_in_queue = true;
                    return Idle{};
                }, pong::packet::EnterQueue{});
            }
        },
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   leave_queue_txt(gui, "LEAVE QUEUE", font, 20)
,   leave_queue_button(gui, 
        [this] () {
            if (playing_state == Game::Playing::Spectator) {
                send_notify([this] () {
                    is_in_queue = false;
                    return Idle{};
                }, pong::packet::LeaveQueue{});
            }
        },
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   abandon_txt(gui, "ABANDON", font, 20)
,   abandon_button(gui, 
        [this] () {
            std::cout << "abandon!\n";
            if (playing_state != Game::Playing::Spectator) {
                if (want_to_abandon) {
                    send_notify([this] () {
                        is_in_queue = false;
                        want_to_abandon = false;
                        abandon_txt.setString("ABANDON");

                        ++spectators_count;
                        spectator_count_txt.setString(std::to_string(spectators_count));
                        spectator_count_txt.setFillColor({ 0xFF, 0x99, 0x00 });

                        if (playing_state == Game::Playing::Left) {
                            left_player = "";
                            left_versus_txt.setString(left_player);
                        } else {
                            right_player = "";
                            right_versus_txt.setString(right_player);
                        }

                        playing_state = Game::Playing::Spectator;

                        return Idle{};
                    }, pong::packet::Abandon{});
                } else {
                    want_to_abandon = true;
                    abandon_txt.setString("SURE ?");
                }
            }
        },
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   return_to_main_lobby{ false }
,   is_in_queue{ false }
,   want_to_abandon{ false }
{
    left_pad.setPosition(Game::pad_padding - Game::pad_width, Game::pad_boundary / 2.f);
    right_pad.setPosition(Game::boundaries_x - Game::pad_padding, Game::pad_boundary / 2.f);

    if (!spectator_texture.loadFromFile("../assets/eye.png")) {
        std::cerr << "Couldn't load '../assets/eye.png'" << '\n';
        throw std::runtime_error("Couldn't load '../assets/eye.png'");
    }

    spectator_spr.setTexture(&spectator_texture, true);
    {
        auto s = spectator_texture.getSize();
        spectator_spr.setSize({ static_cast<float>(s.x), static_cast<float>(s.y) });
    }


    //////////////
    // Versus
    //////////////

    gui.set_constraint<[] (float height, float top_offset, float lv_top, float lv_height) {
        return lv_top + lv_height - height - top_offset;
    }>(versus_txt.top(), { versus_txt.height(), versus_txt.top_offset(), left_versus_txt.top(), left_versus_txt.height() });

    gui.set_constraint<[] (float left_offset, float width, float w_left, float w_width) {
        return w_left + (w_width - width) / 2.f - left_offset;
    }>(versus_txt.left(), { versus_txt.left_offset(), versus_txt.width(), window_properties.left(), window_properties.width() });


    //////////////
    // Left player name
    //////////////

    gui.set_constraint<[] (float top_offset, float w_top) {
        return w_top + 10.f - top_offset;
    }>(left_versus_txt.top(), { left_versus_txt.top_offset(), window_properties.top() });

    gui.set_constraint<[] (float left_offset, float width, float vs_left, float vs_left_offset) {
        return vs_left + vs_left_offset - width - 10.f - left_offset;
    }>(left_versus_txt.left(), { left_versus_txt.left_offset(), left_versus_txt.width(), versus_txt.left(), versus_txt.left_offset() });


    //////////////
    // Right player name
    //////////////

    gui.set_constraint<[] (float top_offset, float w_top) {
        return w_top + 10.f - top_offset;
    }>(right_versus_txt.top(), { right_versus_txt.top_offset(), window_properties.top() });

    gui.set_constraint<[] (float left_offset, float vs_left, float vs_left_offset, float vs_width) {
        return vs_left + vs_left_offset + vs_width + 10.f - left_offset;
    }>(right_versus_txt.left(), { right_versus_txt.left_offset(), versus_txt.left(), versus_txt.left_offset(), versus_txt.width() });


    //////////////
    // Score dash
    //////////////

    gui.set_constraint<[] (float height, float top_offset, float ls_top, float ls_top_offset, float ls_heigth) {
        return ls_top + ls_top_offset + (ls_heigth - height) / 2.f - top_offset;
    }>(score_dash_txt.top(), { score_dash_txt.height(), score_dash_txt.top_offset(), left_score_txt.top(), left_score_txt.top_offset(), left_score_txt.height() });

    gui.set_constraint<[] (float width, float left_offset, float w_left, float w_width) {
        return w_left + (w_width - width) / 2.f - left_offset;
    }>(score_dash_txt.left(), { score_dash_txt.width(), score_dash_txt.left_offset(), window_properties.left(), window_properties.width() });



    //////////////
    // Left player's score
    //////////////

    gui.set_constraint<[] (float top_offset, float vs_top, float vs_top_offset, float vs_height) {
        return vs_top + vs_top_offset + vs_height + 20.f - top_offset;
    }>(left_score_txt.top(), { left_score_txt.top_offset(), versus_txt.top(), versus_txt.top_offset(), versus_txt.height() });

    gui.set_constraint<[] (float left_offset, float width, float dash_left, float dash_left_offset) {
        return dash_left + dash_left_offset - width - left_offset - 10.f;
    }>(left_score_txt.left(), { left_score_txt.left_offset(), left_score_txt.width(), score_dash_txt.left(), score_dash_txt.left_offset() });


    //////////////
    // Right player's score
    //////////////

    gui.set_constraint<[] (float top_offset, float vs_top, float vs_top_offset, float vs_height) {
        return vs_top + vs_top_offset + vs_height + 20.f - top_offset;
    }>(right_score_txt.top(), { right_score_txt.top_offset(), versus_txt.top(), versus_txt.top_offset(), versus_txt.height() });

    gui.set_constraint<[] (float left_offset, float dash_left, float dash_left_offset, float dash_width) {
        return dash_left + dash_left_offset + dash_width + 10.f - left_offset;
    }>(right_score_txt.left(), { right_score_txt.left_offset(), score_dash_txt.left(), score_dash_txt.left_offset(), score_dash_txt.width() });


    //////////////
    // Quit
    //////////////

    quit_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float left_offset, float width, float w_left, float w_width) {
        return w_left + w_width - left_offset - width - 10.f;
    }>(quit_txt.left(), { quit_txt.left_offset(), quit_txt.width(), window_properties.left(), window_properties.width() });

    gui.set_constraint<[] (float top_offset, float w_top) {
        return w_top + top_offset + 10.f;
    }>(quit_txt.top(), { quit_txt.top_offset(), window_properties.top() });


    //////////////
    // Quit button
    //////////////

    gui.set_constraint<[] (float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - 5.f;
    }>(quit_button.left(), { quit_txt.left(), quit_txt.left_offset() });

    gui.set_constraint<[] (float qt_top, float qt_top_offset) {
        return qt_top + qt_top_offset - 5.f;
    }>(quit_button.top(), { quit_txt.top(), quit_txt.top_offset() });

    gui.set_constraint<[] (float qt_width) {
        return qt_width + 10.f;
    }>(quit_button.width(), { quit_txt.width() });

    gui.set_constraint<[] (float qt_height) {
        return qt_height + 10.f;
    }>(quit_button.height(), { quit_txt.height() });


    //////////////
    // Join Queue
    //////////////

    quit_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float left_offset, float width, float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - left_offset - width - 20.f;
    }>(join_queue_txt.left(), { join_queue_txt.left_offset(), join_queue_txt.width(), quit_txt.left(), quit_txt.left_offset() });

    gui.set_constraint<[] (float top_offset, float w_top) {
        return w_top + top_offset + 10.f;
    }>(join_queue_txt.top(), { join_queue_txt.top_offset(), window_properties.top() });


    //////////////
    // Join Queue button
    //////////////

    gui.set_constraint<[] (float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - 5.f;
    }>(join_queue_button.left(), { join_queue_txt.left(), join_queue_txt.left_offset() });

    gui.set_constraint<[] (float qt_top, float qt_top_offset) {
        return qt_top + qt_top_offset - 5.f;
    }>(join_queue_button.top(), { join_queue_txt.top(), join_queue_txt.top_offset() });

    gui.set_constraint<[] (float qt_width) {
        return qt_width + 10.f;
    }>(join_queue_button.width(), { join_queue_txt.width() });

    gui.set_constraint<[] (float qt_height) {
        return qt_height + 10.f;
    }>(join_queue_button.height(), { join_queue_txt.height() });


    //////////////
    // Leave Queue
    //////////////

    quit_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float left_offset, float width, float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - left_offset - width - 20.f;
    }>(leave_queue_txt.left(), { leave_queue_txt.left_offset(), leave_queue_txt.width(), quit_txt.left(), quit_txt.left_offset() });

    gui.set_constraint<[] (float top_offset, float w_top) {
        return w_top + top_offset + 10.f;
    }>(leave_queue_txt.top(), { leave_queue_txt.top_offset(), window_properties.top() });


    //////////////
    // Leave Queue button
    //////////////

    gui.set_constraint<[] (float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - 5.f;
    }>(leave_queue_button.left(), { leave_queue_txt.left(), leave_queue_txt.left_offset() });

    gui.set_constraint<[] (float qt_top, float qt_top_offset) {
        return qt_top + qt_top_offset - 5.f;
    }>(leave_queue_button.top(), { leave_queue_txt.top(), leave_queue_txt.top_offset() });

    gui.set_constraint<[] (float qt_width) {
        return qt_width + 10.f;
    }>(leave_queue_button.width(), { leave_queue_txt.width() });

    gui.set_constraint<[] (float qt_height) {
        return qt_height + 10.f;
    }>(leave_queue_button.height(), { leave_queue_txt.height() });


    //////////////
    // Abandon
    //////////////

    quit_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float left_offset, float width, float w_left, float w_width) {
        return w_left + w_width - left_offset - width - 10.f;
    }>(abandon_txt.left(), { abandon_txt.left_offset(), abandon_txt.width(), window_properties.left(), window_properties.width() });

    gui.set_constraint<[] (float top_offset, float w_top) {
        return w_top + top_offset + 10.f;
    }>(abandon_txt.top(), { abandon_txt.top_offset(), window_properties.top() });


    //////////////
    // Abandon button
    //////////////

    gui.set_constraint<[] (float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - 5.f;
    }>(abandon_button.left(), { abandon_txt.left(), abandon_txt.left_offset() });

    gui.set_constraint<[] (float qt_top, float qt_top_offset) {
        return qt_top + qt_top_offset - 5.f;
    }>(abandon_button.top(), { abandon_txt.top(), abandon_txt.top_offset() });

    gui.set_constraint<[] (float qt_width) {
        return qt_width + 10.f;
    }>(abandon_button.width(), { abandon_txt.width() });

    gui.set_constraint<[] (float qt_height) {
        return qt_height + 10.f;
    }>(abandon_button.height(), { abandon_txt.height() });


    //////////////
    // Spectator count
    //////////////

    gui.set_constraint<[] (float top_offset, float w_top) {
        return w_top + 10.f - top_offset;
    }>(spectator_count_txt.top(), { spectator_count_txt.top_offset(), window_properties.top() });

    gui.set_constraint<[] (float left_offset, float w_left) {
        return w_left + 10.f - left_offset;
    }>(spectator_count_txt.left(), { spectator_count_txt.left_offset(), window_properties.left() });


    //////////////
    // Spectator sprite
    //////////////

    gui.set_constraint<[] (float height, float sc_top, float sc_top_offset, float sc_height) {
        return sc_top + sc_top_offset + (sc_height - height) / 2.f;
    }>(spectator_spr.top(), { spectator_spr.height(), spectator_count_txt.top(), spectator_count_txt.top_offset(), spectator_count_txt.height() });

    gui.set_constraint<[] (float sc_left, float sc_left_offset, float sc_width) {
        return sc_left + sc_left_offset + sc_width + 10.f;
    }>(spectator_spr.left(), { spectator_count_txt.left(), spectator_count_txt.left_offset(), spectator_count_txt.width() });

    gui.set_constraint<[] (float sc_height) {
        return sc_height;
    }>(spectator_spr.height(), { spectator_count_txt.height() });

    gui.set_constraint<[] (float height, float ratio) {
        return height * ratio;
    }>(spectator_spr.width(), { spectator_spr.height(), spectator_spr.ratio() });


}

Action InRoom::update(float dt) {
    game.update(dt, playing_state, get_input());
    update_graphics();

    quit_button.update(dt);
    join_queue_button.update(dt);
    leave_queue_button.update(dt);
    abandon_button.update(dt);

    if (return_to_main_lobby) {
        return change_state<InMainLobby>(gui, window_properties, font, this->username);
    }

    return Idle{};
}

void InRoom::update_graphics() {
    ball.setPosition(game.ball.position);
    left_pad.setPosition(left_pad.getPosition().x, game.pad_left.y);
    right_pad.setPosition(right_pad.getPosition().x, game.pad_right.y);
}


void InRoom::reset_game() {
    game = Game{};
    is_pressing_down = is_pressing_up = false;
    update_graphics();
}

pong::Input InRoom::get_input() const {
    return is_pressing_down == is_pressing_up ?
        pong::Input::Idle
    :   is_pressing_down ?
        pong::Input::Down
    :   pong::Input::Up;
}


void InRoom::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (left_player != "" && right_player != "") {
        target.draw(ball, states);
    }

    if (left_player != "") {
        target.draw(left_pad, states);
    }

    if (right_player != "") {
        target.draw(right_pad, states);
    }

    target.draw(versus_txt);
    target.draw(left_versus_txt);
    target.draw(right_versus_txt);

    target.draw(score_dash_txt);
    target.draw(left_score_txt);
    target.draw(right_score_txt);

    target.draw(spectator_count_txt);
    target.draw(spectator_spr);

    if (playing_state == Game::Playing::Spectator) {
        target.draw(quit_button);
        target.draw(quit_txt);

        quit_txt.debug_draw(target, states);
        quit_button.debug_draw(target, states);

        if (is_in_queue) {
            target.draw(leave_queue_button);
            target.draw(leave_queue_txt);

            leave_queue_txt.debug_draw(target, states);
            leave_queue_button.debug_draw(target, states);
        } else {
            target.draw(join_queue_button);
            target.draw(join_queue_txt);

            join_queue_txt.debug_draw(target, states);
            join_queue_button.debug_draw(target, states);
        }
    } else {
        target.draw(abandon_button);
        target.draw(abandon_txt);

        abandon_txt.debug_draw(target, states);
        abandon_button.debug_draw(target, states);
    }

    spectator_spr.debug_draw(target, states);
    spectator_count_txt.debug_draw(target, states);
    versus_txt.debug_draw(target, states);
    left_versus_txt.debug_draw(target, states);
    right_versus_txt.debug_draw(target, states);
    score_dash_txt.debug_draw(target, states);
    left_score_txt.debug_draw(target, states);
    right_score_txt.debug_draw(target, states);
}


sftk::PropagateEvent InRoom::on_key_pressed(sf::Window&, sf::Event::KeyEvent const& b) {
    if (playing_state == Game::Playing::Spectator) {
        return true;
    }

    auto last_input = get_input();

    if (b.code == sf::Keyboard::Up) {
        is_pressing_up = true;
    } 
    else if (b.code == sf::Keyboard::Down) {
        is_pressing_down = true;
    } else {
        return true;
    }

    auto input = get_input();
    if(last_input != input) {
        std::cout << "Send Input\n";
        send(pong::packet::Input{ input });
    }

    return false;
}

sftk::PropagateEvent InRoom::on_key_released(sf::Window&, sf::Event::KeyEvent const& b) {
    if (playing_state == Game::Playing::Spectator) {
        return true;
    }

    auto last_input = get_input();

    if (b.code == sf::Keyboard::Up) {
        is_pressing_up = false;
    } 
    else if (b.code == sf::Keyboard::Down) {
        is_pressing_down = false;
    } else {
        return true;
    }

    auto input = get_input();
    if(last_input != input) {
        std::cout << "Send Input\n";
        send(pong::packet::Input{ input });
    }

    return false;
}


sftk::PropagateEvent InRoom::on_mouse_button_pressed(sf::Window& w, sf::Event::MouseButtonEvent const& b) {
    if (playing_state == Game::Playing::Spectator) {
        return 
            quit_button.on_mouse_button_pressed(w, b) &&
            is_in_queue ? 
                leave_queue_button.on_mouse_button_pressed(w, b) :
                join_queue_button.on_mouse_button_pressed(w, b);
    } else {
        return abandon_button.on_mouse_button_pressed(w, b);
    }
}

sftk::PropagateEvent InRoom::on_mouse_button_released(sf::Window& w, sf::Event::MouseButtonEvent const& b) {
    if (playing_state == Game::Playing::Spectator) {
        return 
            quit_button.on_mouse_button_released(w, b) &&
            is_in_queue ? 
                leave_queue_button.on_mouse_button_released(w, b) :
                join_queue_button.on_mouse_button_released(w, b);
    } else {
        return abandon_button.on_mouse_button_released(w, b);
    }
}

sftk::PropagateEvent InRoom::on_mouse_moved(sf::Window& w, sf::Event::MouseMoveEvent const& b) {
    if (playing_state == Game::Playing::Spectator) {
        return 
            quit_button.on_mouse_moved(w, b) &&
            is_in_queue ? 
                leave_queue_button.on_mouse_moved(w, b) :
                join_queue_button.on_mouse_moved(w, b);
    } else {
        return abandon_button.on_mouse_moved(w, b);
    }
}


void InRoom::update_properties(gui::Gui<> const& _gui) {
    versus_txt.update_properties(_gui);
    left_versus_txt.update_properties(_gui);
    right_versus_txt.update_properties(_gui);

    score_dash_txt.update_properties(_gui);
    left_score_txt.update_properties(_gui);
    right_score_txt.update_properties(_gui);

    spectator_count_txt.update_properties(_gui);
    spectator_spr.update_properties(_gui);

    quit_button.update_properties(_gui);
    quit_txt.update_properties(_gui);

    join_queue_button.update_properties(_gui);
    join_queue_txt.update_properties(_gui);

    leave_queue_button.update_properties(_gui);
    leave_queue_txt.update_properties(_gui);

    abandon_button.update_properties(_gui);
    abandon_txt.update_properties(_gui);

}

void InRoom::notify_gui(gui::Gui<>& _gui) const {
    versus_txt.notify_gui(_gui);
    left_versus_txt.notify_gui(_gui);
    right_versus_txt.notify_gui(_gui);

    score_dash_txt.notify_gui(_gui);
    left_score_txt.notify_gui(_gui);
    right_score_txt.notify_gui(_gui);

    spectator_count_txt.notify_gui(_gui);
    spectator_spr.notify_gui(_gui);

    quit_button.notify_gui(_gui);
    quit_txt.notify_gui(_gui);

    join_queue_button.notify_gui(_gui);
    join_queue_txt.notify_gui(_gui);

    leave_queue_button.notify_gui(_gui);
    leave_queue_txt.notify_gui(_gui);

    abandon_button.notify_gui(_gui);
    abandon_txt.notify_gui(_gui);
}


Action InRoom::on_room_info(packet_t packet) {
    auto room_info = from_packet<pong::packet::RoomInfo>(packet);
    std::cout << "Right player: " << room_info.right_player << '\n';
    std::cout << "Left player: " << room_info.left_player << '\n';
    std::cout << "Spectators: ";
    for(auto sp : room_info.spectators) {
        std::cout << sp << " ";
    }
    std::cout << "\n";

    left_player = room_info.left_player;
    right_player = room_info.right_player;
    spectators_count = room_info.spectators.size() + 1 /* me */;

    spectator_count_txt.setString(std::to_string(spectators_count));
    spectator_count_txt.setFillColor({ 0xFF, 0x99, 0x00 });

    left_versus_txt.setString(left_player);
    right_versus_txt.setString(right_player);

    reset_game();

    return Idle{};
}

Action InRoom::on_game_state(packet_t packet) {
    auto gs = from_packet<pong::packet::GameState>(packet);


    game.ball = gs.ball;
    game.pad_left = gs.left;
    game.pad_right = gs.right;

    update_graphics();

    return Idle{};
}

Action InRoom::on_new_user(packet_t packet) {
    auto new_user = from_packet<pong::packet::NewUser>(packet).username;
    std::cout << "New user " << new_user << '\n';
    ++spectators_count;
    spectator_count_txt.setString(std::to_string(spectators_count));
    return Idle{};
}

Action InRoom::on_old_user(packet_t packet) {
    auto old_user = from_packet<pong::packet::OldUser>(packet).username;
    std::cout << "Old user " << old_user << '\n';
    --spectators_count;
    spectator_count_txt.setString(std::to_string(spectators_count));
    return Idle{};
}

Action InRoom::on_new_player(packet_t packet) {
    auto new_player = from_packet<pong::packet::NewPlayer>(packet);
    std::cout << "New player " << new_player.username << " on " << (new_player.side == pong::packet::NewPlayer::Side::Left ? "left" : "right") << " side" << '\n';


    --spectators_count;
    spectator_count_txt.setString(std::to_string(spectators_count));

    if (new_player.side == pong::packet::NewPlayer::Side::Left) {
        left_player = new_player.username;
        left_versus_txt.setString(left_player);
        left_versus_txt.setFillColor(sf::Color::White);
    } else {
        right_player = new_player.username;
        right_versus_txt.setString(right_player);
        right_versus_txt.setFillColor(sf::Color::White);
    }

    reset_game();

    return Idle{};
}

Action InRoom::on_old_player(packet_t packet) {
    auto old_player = from_packet<pong::packet::OldPlayer>(packet);
    std::cout << "Old player " << old_player.username << " on " << (old_player.side == pong::packet::OldPlayer::Side::Left ? "left" : "right") << " side" << '\n';


    ++spectators_count;
    spectator_count_txt.setString(std::to_string(spectators_count));

    if (old_player.side == pong::packet::OldPlayer::Side::Left) {
        left_player = "";
        left_versus_txt.setString(left_player);
    } else {
        right_player = "";
        right_versus_txt.setString(right_player);
    }

    return Idle{};
}

Action InRoom::on_be_player(packet_t packet) {
    auto be_player_side = from_packet<pong::packet::BePlayer>(packet);
    std::cout << "BePlayer " << (be_player_side.side == pong::packet::BePlayer::Side::Left ? "left" : "right") << '\n';


    playing_state = be_player_side.side == pong::packet::BePlayer::Side::Left ? Game::Playing::Left : Game::Playing::Right;

    --spectators_count;
    spectator_count_txt.setString(std::to_string(spectators_count));

    if (be_player_side.side == pong::packet::BePlayer::Side::Left) {
        left_player = username;
        left_versus_txt.setString(left_player);
        left_versus_txt.setFillColor({ 0xFF, 0x99, 0x00 });
        spectator_count_txt.setFillColor(sf::Color::White);
    }
    else {
        right_player = username;
        right_versus_txt.setString(right_player);
        right_versus_txt.setFillColor({ 0xFF, 0x99, 0x00 });
        spectator_count_txt.setFillColor(sf::Color::White);
    }

    reset_game();


    return Idle{};
}

}