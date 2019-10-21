#include <pong/client/state/Room/Graphics.hpp>

#include <pong/client/Logger.hpp>

namespace pong::client::state::room {

Graphics::Graphics(Application app)
:   quit_txt(app.gui_allocator(), "QUIT", app.get_font(), 16)
,   quit_button(
        app.gui_allocator(), 
        [] () {},
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   spectator_count_txt{ app.gui_allocator(), "0", app.get_font(), 16 }
,   spectator_texture{}
,   spectator_spr{ app.gui_allocator(), spectator_texture }
,   join_queue_txt(app.gui_allocator(),  "JOIN QUEUE", app.get_font(), 16)
,   join_queue_button(        
        app.gui_allocator(), 
        [] () {},
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   leave_queue_txt(app.gui_allocator(),  "LEAVE QUEUE", app.get_font(), 16)
,   leave_queue_button(       
        app.gui_allocator(), 
        [] () {},
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   abandon_txt(app.gui_allocator(),  "ABANDON", app.get_font(), 16)
,   abandon_button(       
        app.gui_allocator(), 
        [] () {},
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   versus_txt{ app.gui_allocator(), "vs", app.get_font(), 15 }
,   left_versus_txt{ app.gui_allocator(), "", app.get_font(), 30 }
,   right_versus_txt{ app.gui_allocator(), "", app.get_font(), 30 }
,   score_dash_txt{ app.gui_allocator(), "-", app.get_font(), 40 }
,   left_score_txt{ app.gui_allocator(), "2", app.get_font(), 80 }
,   right_score_txt{ app.gui_allocator(), "1", app.get_font(), 80 }
,   left_pad{{ pong::meta::pad::width, pong::meta::pad::height }}
,   right_pad{{ pong::meta::pad::width, pong::meta::pad::height }}
,   ball{{ pong::meta::ball::radius, pong::meta::ball::radius }}
{

    left_pad.setPosition(pong::meta::pad::padding - pong::meta::pad::width, pong::meta::pad::bounds_y / 2.f);
    right_pad.setPosition(pong::meta::bounds_x - pong::meta::pad::padding, pong::meta::pad::bounds_y / 2.f);

    if (!spectator_texture.loadFromFile("../assets/eye.png")) {
        ERROR("Couldn't load '../assets/eye.png'");
        throw std::runtime_error("Couldn't load '../assets/eye.png'");
    }

    spectator_spr.setTexture(&spectator_texture, true);
    {
        auto s = spectator_texture.getSize();
        spectator_spr.setSize({ static_cast<float>(s.x), static_cast<float>(s.y) });
    }

    //////////////
    // Quit
    //////////////

    quit_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float left_offset, float width, float w_width) {
        return w_width - left_offset - width - 10.f;
    }>(quit_txt.left(), { quit_txt.left_offset(), quit_txt.width(), app.width_property() });

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(quit_txt.top(), { quit_txt.top_offset()});


    //////////////
    // Quit button
    //////////////

    app.set_constraint<[] (float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - 5.f;
    }>(quit_button.left(), { quit_txt.left(), quit_txt.left_offset() });

    app.set_constraint<[] (float qt_top, float qt_top_offset) {
        return qt_top + qt_top_offset - 5.f;
    }>(quit_button.top(), { quit_txt.top(), quit_txt.top_offset() });

    app.set_constraint<[] (float qt_width) {
        return qt_width + 10.f;
    }>(quit_button.width(), { quit_txt.width() });

    app.set_constraint<[] (float qt_height) {
        return qt_height + 10.f;
    }>(quit_button.height(), { quit_txt.height() });


    //////////////
    // Spectator count
    //////////////

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(spectator_count_txt.top(), { spectator_count_txt.top_offset() });

    app.set_constraint<[] (float left_offset) {
        return 10.f - left_offset;
    }>(spectator_count_txt.left(), { spectator_count_txt.left_offset() });


    //////////////
    // Spectator sprite
    //////////////

    app.set_constraint<[] (float height, float sc_top, float sc_top_offset, float sc_height) {
        return sc_top + sc_top_offset + (sc_height - height) / 2.f;
    }>(spectator_spr.top(), { spectator_spr.height(), spectator_count_txt.top(), spectator_count_txt.top_offset(), spectator_count_txt.height() });

    app.set_constraint<[] (float sc_left, float sc_left_offset, float sc_width) {
        return sc_left + sc_left_offset + sc_width + 10.f;
    }>(spectator_spr.left(), { spectator_count_txt.left(), spectator_count_txt.left_offset(), spectator_count_txt.width() });

    app.set_constraint<[] (float sc_height) {
        return sc_height;
    }>(spectator_spr.height(), { spectator_count_txt.height() });

    app.set_constraint<[] (float height, float ratio) {
        return height * ratio;
    }>(spectator_spr.width(), { spectator_spr.height(), spectator_spr.ratio() });


    //////////////
    // Join Queue
    //////////////

    join_queue_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float left_offset, float width, float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - left_offset - width - 20.f;
    }>(join_queue_txt.left(), { join_queue_txt.left_offset(), join_queue_txt.width(), quit_txt.left(), quit_txt.left_offset() });

    app.set_constraint<[] (float top_offset) {
        return top_offset + 10.f;
    }>(join_queue_txt.top(), { join_queue_txt.top_offset() });

    app.set_constraint<[] (float left_offset, float width, float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - left_offset - width - 20.f;
    }>(join_queue_txt.left(), { join_queue_txt.left_offset(), join_queue_txt.width(), quit_txt.left(), quit_txt.left_offset() });

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(join_queue_txt.top(), { join_queue_txt.top_offset()});


    //////////////
    // Join Queue button
    //////////////

    app.set_constraint<[] (float jq_left, float jq_left_offset) {
        return jq_left + jq_left_offset - 5.f;
    }>(join_queue_button.left(), { join_queue_txt.left(), join_queue_txt.left_offset() });

    app.set_constraint<[] (float jq_top, float jq_top_offset) {
        return jq_top + jq_top_offset - 5.f;
    }>(join_queue_button.top(), { join_queue_txt.top(), join_queue_txt.top_offset() });

    app.set_constraint<[] (float jq_width) {
        return jq_width + 10.f;
    }>(join_queue_button.width(), { join_queue_txt.width() });

    app.set_constraint<[] (float jq_height) {
        return jq_height + 10.f;
    }>(join_queue_button.height(), { join_queue_txt.height() });


    //////////////
    // Leave Queue
    //////////////

    leave_queue_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float left_offset, float width, float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - left_offset - width - 20.f;
    }>(leave_queue_txt.left(), { leave_queue_txt.left_offset(), leave_queue_txt.width(), quit_txt.left(), quit_txt.left_offset() });

    app.set_constraint<[] (float top_offset) {
        return top_offset + 10.f;
    }>(leave_queue_txt.top(), { leave_queue_txt.top_offset() });

    app.set_constraint<[] (float left_offset, float width, float qt_left, float qt_left_offset) {
        return qt_left + qt_left_offset - left_offset - width - 20.f;
    }>(leave_queue_txt.left(), { leave_queue_txt.left_offset(), leave_queue_txt.width(), quit_txt.left(), quit_txt.left_offset() });

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(leave_queue_txt.top(), { leave_queue_txt.top_offset()});

    //////////////
    // Leave Queue button
    //////////////

    app.set_constraint<[] (float lq_left, float lq_left_offset) {
        return lq_left + lq_left_offset - 5.f;
    }>(leave_queue_button.left(), { leave_queue_txt.left(), leave_queue_txt.left_offset() });

    app.set_constraint<[] (float lq_top, float lq_top_offset) {
        return lq_top + lq_top_offset - 5.f;
    }>(leave_queue_button.top(), { leave_queue_txt.top(), leave_queue_txt.top_offset() });

    app.set_constraint<[] (float lq_width) {
        return lq_width + 10.f;
    }>(leave_queue_button.width(), { leave_queue_txt.width() });

    app.set_constraint<[] (float lq_height) {
        return lq_height + 10.f;
    }>(leave_queue_button.height(), { leave_queue_txt.height() });

    //////////////
    // Abandon
    //////////////

    abandon_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float left_offset, float width, float w_width) {
        return w_width - left_offset - width - 10.f;
    }>(abandon_txt.left(), { abandon_txt.left_offset(), abandon_txt.width(), app.width_property() });

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(abandon_txt.top(), { abandon_txt.top_offset()});


    //////////////
    // Abandon button
    //////////////

    app.set_constraint<[] (float ab_left, float ab_left_offset) {
        return ab_left + ab_left_offset - 5.f;
    }>(abandon_button.left(), { abandon_txt.left(), abandon_txt.left_offset() });

    app.set_constraint<[] (float ab_top, float ab_top_offset) {
        return ab_top + ab_top_offset - 5.f;
    }>(abandon_button.top(), { abandon_txt.top(), abandon_txt.top_offset() });

    app.set_constraint<[] (float ab_width) {
        return ab_width + 10.f;
    }>(abandon_button.width(), { abandon_txt.width() });

    app.set_constraint<[] (float ab_height) {
        return ab_height + 10.f;
    }>(abandon_button.height(), { abandon_txt.height() });


    //////////////
    // Versus
    //////////////

    app.set_constraint<[] (float height, float top_offset, float lv_top, float lv_height) {
        return lv_top + lv_height - height - top_offset;
    }>(versus_txt.top(), { versus_txt.height(), versus_txt.top_offset(), left_versus_txt.top(), left_versus_txt.height() });

    app.set_constraint<[] (float left_offset, float width, float w_width) {
        return (w_width - width) / 2.f - left_offset;
    }>(versus_txt.left(), { versus_txt.left_offset(), versus_txt.width(), app.width_property() });


    //////////////
    // Left player name
    //////////////

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(left_versus_txt.top(), { left_versus_txt.top_offset() });

    app.set_constraint<[] (float left_offset, float width, float vs_left, float vs_left_offset) {
        return vs_left + vs_left_offset - width - 10.f - left_offset;
    }>(left_versus_txt.left(), { left_versus_txt.left_offset(), left_versus_txt.width(), versus_txt.left(), versus_txt.left_offset() });


    //////////////
    // Right player name
    //////////////

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(right_versus_txt.top(), { right_versus_txt.top_offset() });

    app.set_constraint<[] (float left_offset, float vs_left, float vs_left_offset, float vs_width) {
        return vs_left + vs_left_offset + vs_width + 10.f - left_offset;
    }>(right_versus_txt.left(), { right_versus_txt.left_offset(), versus_txt.left(), versus_txt.left_offset(), versus_txt.width() });


    //////////////
    // Score dash
    //////////////

    app.set_constraint<[] (float height, float top_offset, float ls_top, float ls_top_offset, float ls_heigth) {
        return ls_top + ls_top_offset + (ls_heigth - height) / 2.f - top_offset;
    }>(score_dash_txt.top(), { score_dash_txt.height(), score_dash_txt.top_offset(), left_score_txt.top(), left_score_txt.top_offset(), left_score_txt.height() });

    app.set_constraint<[] (float width, float left_offset, float w_width) {
        return (w_width - width) / 2.f - left_offset;
    }>(score_dash_txt.left(), { score_dash_txt.width(), score_dash_txt.left_offset(), app.width_property() });



    //////////////
    // Left player's score
    //////////////

    app.set_constraint<[] (float top_offset, float vs_top, float vs_top_offset, float vs_height) {
        return vs_top + vs_top_offset + vs_height + 20.f - top_offset;
    }>(left_score_txt.top(), { left_score_txt.top_offset(), versus_txt.top(), versus_txt.top_offset(), versus_txt.height() });

    app.set_constraint<[] (float left_offset, float width, float dash_left, float dash_left_offset) {
        return dash_left + dash_left_offset - width - left_offset - 10.f;
    }>(left_score_txt.left(), { left_score_txt.left_offset(), left_score_txt.width(), score_dash_txt.left(), score_dash_txt.left_offset() });


    //////////////
    // Right player's score
    //////////////

    app.set_constraint<[] (float top_offset, float vs_top, float vs_top_offset, float vs_height) {
        return vs_top + vs_top_offset + vs_height + 20.f - top_offset;
    }>(right_score_txt.top(), { right_score_txt.top_offset(), versus_txt.top(), versus_txt.top_offset(), versus_txt.height() });

    app.set_constraint<[] (float left_offset, float dash_left, float dash_left_offset, float dash_width) {
        return dash_left + dash_left_offset + dash_width + 10.f - left_offset;
    }>(right_score_txt.left(), { right_score_txt.left_offset(), score_dash_txt.left(), score_dash_txt.left_offset(), score_dash_txt.width() });


}

void Graphics::free_properties(gui::Allocator<> gui) const {
    quit_txt.free_properties(gui);
    quit_button.free_properties(gui);

    spectator_count_txt.free_properties(gui);
    spectator_spr.free_properties(gui);

    join_queue_txt.free_properties(gui);
    join_queue_button.free_properties(gui);

    leave_queue_txt.free_properties(gui);
    leave_queue_button.free_properties(gui);

    abandon_txt.free_properties(gui);
    abandon_button.free_properties(gui);

    score_dash_txt.free_properties(gui);
    left_score_txt.free_properties(gui);
    right_score_txt.free_properties(gui);

    versus_txt.free_properties(gui);
    left_versus_txt.free_properties(gui);
    right_versus_txt.free_properties(gui);
}


std::optional<Graphics::Button> Graphics::on_click(Game::Role role, sf::Vector2f const& position) {
    if (role == Game::Role::Spec || role == Game::Role::Waiting) {
        if (quit_button.on_click(position)) {
            return Graphics::Button::Quit;
        }
        if (role == Game::Role::Waiting) {
            if (leave_queue_button.on_click(position)) {
                return Graphics::Button::LeaveQueue;
            }
        } else {
            if (join_queue_button.on_click(position)) {
                return Graphics::Button::JoinQueue;
            }
        }
    } else {
        if (abandon_button.on_click(position)) {
            return Graphics::Button::Abandon;
        }
    }

    return std::nullopt;
}

std::optional<Graphics::Button> Graphics::on_release_click(Game::Role role, sf::Vector2f const& position) {
    if (role == Game::Role::Spec || role == Game::Role::Waiting) {
        if (quit_button.on_release_click(position)) {
            return Graphics::Button::Quit;
        }
        if (role == Game::Role::Waiting) {
            if (leave_queue_button.on_release_click(position)) {
                return Graphics::Button::LeaveQueue;
            }
        } else {
            if (join_queue_button.on_release_click(position)) {
                return Graphics::Button::JoinQueue;
            }
        }
    } else {
        if (abandon_button.on_release_click(position)) {
            return Graphics::Button::Abandon;
        }
    }
    return std::nullopt;
}

void Graphics::on_hover(sf::Vector2f const& position) {
    quit_button.on_hover(position);
    join_queue_button.on_hover(position);
    leave_queue_button.on_hover(position);
    abandon_button.on_hover(position);
}


void Graphics::update_animations(Application, float dt) {
    quit_button.update(dt);
    join_queue_button.update(dt);
    leave_queue_button.update(dt);
    abandon_button.update(dt);
}

void Graphics::update_game(Application, Game const& game) {
    left_pad.setPosition(left_pad.getPosition().x, game.left.y);
    right_pad.setPosition(right_pad.getPosition().x, game.right.y);
    ball.setPosition(game.ball.position);
}

void Graphics::update_score(Application, unsigned left, unsigned right) {
    left_score_txt.setString(std::to_string(left));
    right_score_txt.setString(std::to_string(right));
}

void Graphics::set_spectator_count(unsigned spectator_count) {
    spectator_count_txt.setString(std::to_string(spectator_count));
}

void Graphics::set_left_player(std::string username) {
    left_versus_txt.setString(std::move(username));
}

void Graphics::set_right_player(std::string username) {
    right_versus_txt.setString(std::move(username));
}

void Graphics::unhighlight_left_player() {
    left_versus_txt.setFillColor(sf::Color::White);
}

void Graphics::unhighlight_right_player() {
    right_versus_txt.setFillColor(sf::Color::White);
}

void Graphics::unhighlight_spectator_count() {
    spectator_count_txt.setFillColor(sf::Color::White);
}

void Graphics::highlight_left_player() {
    left_versus_txt.setFillColor({ 0xFF, 0x99, 0x00 });
}

void Graphics::highlight_right_player() {
    right_versus_txt.setFillColor({ 0xFF, 0x99, 0x00 });
}

void Graphics::highlight_spectator_count() {
    spectator_count_txt.setFillColor({ 0xFF, 0x99, 0x00 });
}


void Graphics::notify_gui(gui::Gui<>& gui) const {
    quit_txt.notify_gui(gui);
    quit_button.notify_gui(gui);

    spectator_count_txt.notify_gui(gui);
    spectator_spr.notify_gui(gui);

    join_queue_txt.notify_gui(gui);
    join_queue_button.notify_gui(gui);

    leave_queue_txt.notify_gui(gui);
    leave_queue_button.notify_gui(gui);

    abandon_txt.notify_gui(gui);
    abandon_button.notify_gui(gui);

    score_dash_txt.notify_gui(gui);
    left_score_txt.notify_gui(gui);
    right_score_txt.notify_gui(gui);

    versus_txt.notify_gui(gui);
    left_versus_txt.notify_gui(gui);
    right_versus_txt.notify_gui(gui);
}

void Graphics::update_properties(gui::Gui<> const& gui) {
    quit_txt.update_properties(gui);
    quit_button.update_properties(gui);

    spectator_count_txt.update_properties(gui);
    spectator_spr.update_properties(gui);

    join_queue_txt.update_properties(gui);
    join_queue_button.update_properties(gui);

    leave_queue_txt.update_properties(gui);
    leave_queue_button.update_properties(gui);

    abandon_txt.update_properties(gui);
    abandon_button.update_properties(gui);

    score_dash_txt.update_properties(gui);
    left_score_txt.update_properties(gui);
    right_score_txt.update_properties(gui);

    versus_txt.update_properties(gui);
    left_versus_txt.update_properties(gui);
    right_versus_txt.update_properties(gui);
}

void Graphics::draw(bool left_player_present, bool right_player_present, Game::Role role, sf::RenderTarget& target, sf::RenderStates states) const {
    // Draw game
    if (left_player_present) {
        target.draw(left_pad, states);
    }
   
    if (right_player_present) {
        target.draw(right_pad, states);
    }

    if (left_player_present && right_player_present) {
        target.draw(ball, states);

        target.draw(score_dash_txt, states);
        target.draw(left_score_txt, states);
        target.draw(right_score_txt, states);
    }

    // Draw GUI after
    if (role == Game::Role::Spec || role == Game::Role::Waiting) {
        target.draw(quit_button, states);
        target.draw(quit_txt, states);

        if (role == Game::Role::Waiting) {
            target.draw(leave_queue_button, states);
            target.draw(leave_queue_txt, states);
        } else {
            target.draw(join_queue_button, states);
            target.draw(join_queue_txt, states);
        }

    } else {
        target.draw(abandon_button, states);
        target.draw(abandon_txt, states);
    }

    target.draw(spectator_spr, states);
    target.draw(spectator_count_txt, states);

    target.draw(versus_txt, states);
    target.draw(left_versus_txt, states);
    target.draw(right_versus_txt, states);

}

}