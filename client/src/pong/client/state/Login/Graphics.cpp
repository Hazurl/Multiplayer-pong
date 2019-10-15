#include <pong/client/state/Login/Graphics.hpp>

namespace pong::client::state::login {

Graphics::Graphics(Application& app)
:   quit_txt(app.gui_allocator(), "QUIT", app.get_font(), 20)
,   quit_button(
        app.gui_allocator(), 
        [] () {},
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
{
    //////////////
    // Quit
    //////////////

    quit_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float left, float left_offset) {
        return left + 5.f - left_offset;
    }>(quit_txt.left(), { quit_button.left(), quit_txt.left_offset() });

    app.set_constraint<[] (float button_top, float button_height, float height, float top_offset) {
        return button_top + (button_height - height) / 2.f - top_offset;
    }>(quit_txt.top(), { quit_button.top(), quit_button.height(), quit_txt.height(), quit_txt.top_offset() });

    app.set_constraint<[] (float height) {
        return height - 10.f;
    }>(quit_txt.size(), { quit_button.height() });


    //////////////
    // Quit button
    //////////////

    app.set_constraint<[] (float width, float window_width) {
        return window_width - width - 5.f;
    }>(quit_button.left(), { quit_button.width(), app.width_property() });

    app.set_constraint<[] () {
        return 5.f;
    }>(quit_button.top(), {});

    app.set_constraint<[] (float window_height) {
        return std::floor(window_height / 40.f) + 10.f;
    }>(quit_button.height(), { app.height_property() });

    app.set_constraint<[] (float txt_width) {
        return txt_width + 10.f;
    }>(quit_button.width(), { quit_txt.width() });
}


void Graphics::on_connection() {

}

void Graphics::on_disconnection() {

}

void Graphics::cursor_left() {

}

void Graphics::cursor_right() {

}

void Graphics::on_character_entered(char c) {

}

std::optional<std::string> Graphics::validate_username() {
    return std::nullopt;
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


void Graphics::update_animations(float dt) {
    quit_button.update(dt);
}

void Graphics::notify_gui(gui::Gui<>& gui) const {
    quit_button.notify_gui(gui);
    quit_txt.notify_gui(gui);
}

void Graphics::update_properties(gui::Gui<> const& gui) {
    quit_button.update_properties(gui);
    quit_txt.update_properties(gui);
}

void Graphics::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(quit_button, states);
    target.draw(quit_txt, states);
}

}