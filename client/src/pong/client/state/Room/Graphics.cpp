#include <pong/client/state/Room/Graphics.hpp>

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
,   left_pad{{ pong::meta::pad::width, pong::meta::pad::height }}
,   right_pad{{ pong::meta::pad::width, pong::meta::pad::height }}
,   ball{{ pong::meta::ball::radius, pong::meta::ball::radius }}
{

    left_pad.setPosition(pong::meta::pad::padding - pong::meta::pad::width, (pong::meta::pad::bounds_y - pong::meta::pad::width) / 2.f);
    right_pad.setPosition(pong::meta::bounds_x - pong::meta::pad::padding, (pong::meta::pad::bounds_y - pong::meta::pad::width) / 2.f);

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

}

void Graphics::free_properties(gui::Allocator<> gui) const {
    quit_txt.free_properties(gui);
    quit_button.free_properties(gui);
}


std::optional<Graphics::Button> Graphics::on_click(sf::Vector2f const& position) {
    if (quit_button.on_click(position)) {
        return Graphics::Button::Quit;
    }

    return std::nullopt;
}

std::optional<Graphics::Button> Graphics::on_release_click(sf::Vector2f const& position) {
    if (quit_button.on_release_click(position)) {
        return Graphics::Button::Quit;
    }

    return std::nullopt;
}

void Graphics::on_hover(sf::Vector2f const& position) {
    quit_button.on_hover(position);
}


void Graphics::update_animations(Application app, float dt) {
    quit_button.update(dt);
}

void Graphics::update_game(Application app, Game const& game) {
    left_pad.setPosition(left_pad.getPosition().x, game.left.y);
    right_pad.setPosition(right_pad.getPosition().x, game.right.y);
    ball.setPosition(game.ball.position);
}

void Graphics::notify_gui(gui::Gui<>& gui) const {
    quit_txt.notify_gui(gui);
    quit_button.notify_gui(gui);
}

void Graphics::update_properties(gui::Gui<> const& gui) {
    quit_txt.update_properties(gui);
    quit_button.update_properties(gui);
}

void Graphics::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    // Draw game
    target.draw(left_pad, states);
    target.draw(right_pad, states);
    target.draw(ball, states);


    // Draw GUI after
    target.draw(quit_button, states);
    target.draw(quit_txt, states);
}

}