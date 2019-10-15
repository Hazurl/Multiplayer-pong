#include <pong/client/state/Login/Graphics.hpp>

namespace pong::client::state::login {

Graphics::Graphics(Application app)
:   title_txt(app.gui_allocator(), "MULTIPLAYER PONG", app.get_font())
,   login_txt(app.gui_allocator(), "Login:", app.get_font())
,   pseudo_txt(app.gui_allocator(), "", app.get_font())
,   connecting_txt(app.gui_allocator(), "Connecting...", app.get_font())
,   by_hazurl_txt(app.gui_allocator(), "by Hazurl", app.get_font())
,   quit_txt(app.gui_allocator(), "QUIT", app.get_font(), 20)
,   quit_button(
        app.gui_allocator(), 
        [] () {},
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   pseudo_cursor({ 10, 10 }, 0, 0)
,   cursor_index{ 0 }
{
    //////////////
    // Title
    //////////////

    title_txt.setFillColor({ 0xFF, 0x99, 0x00 });

    app.set_constraint<[] (float width, float window_width) {
        return (window_width - width) / 2.f;
    }>(title_txt.left(), { title_txt.width(), app.width_property() });

    app.set_constraint<[] (float height, float window_height) {
        return window_height / 4.f - height / 2.f;
    }>(title_txt.top(), { title_txt.size(), app.height_property() });

    app.set_constraint<[] (float window_height) {
        return window_height / 10.f;
    }>(title_txt.size(), { app.height_property() });


    //////////////
    // Login
    //////////////

    login_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float width, float window_width) {
        return (window_width - width) / 2.f;
    }>(login_txt.left(), { login_txt.width(), app.width_property() });

    app.set_constraint<[] (float window_height) {
        return  window_height * 0.6f;
    }>(login_txt.top(), { app.height_property() });

    app.set_constraint<[] (float window_height) {
        return window_height / 25.f;
    }>(login_txt.size(), { app.height_property() });


    //////////////
    // Pseudo
    //////////////

    pseudo_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float width, float window_width) {
        return (window_width - width) / 2.f;
    }>(pseudo_txt.left(), { pseudo_txt.width(), app.width_property() });

    app.set_constraint<[] (float login_top, float login_height, float window_height) {
        return login_top + login_height + window_height / 40.f;
    }>(pseudo_txt.top(), { login_txt.top(), login_txt.size(), app.height_property() });

    app.set_constraint<[] (float window_height) {
        return window_height / 20.f;
    }>(pseudo_txt.size(), { app.height_property() });


    //////////////
    // Connecting
    //////////////

    connecting_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float width,  float window_width) {
        return (window_width - width) / 2.f;
    }>(connecting_txt.left(), { connecting_txt.width(), app.width_property() });

    app.set_constraint<[] (float pseudo_top, float pseudo_size, float window_height) {
        return pseudo_top + pseudo_size + window_height / 40.f;
    }>(connecting_txt.top(), { pseudo_txt.top(), pseudo_txt.size(), app.height_property() });

    app.set_constraint<[] (float window_height) {
        return window_height / 25.f;
    }>(connecting_txt.size(), { app.height_property() });


    //////////////
    // By Hazurl
    //////////////

    by_hazurl_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] () {
        return 10.f;
    }>(by_hazurl_txt.left(), {});

    app.set_constraint<[] (float height, float window_height) {
        return window_height - 10.f - height;
    }>(by_hazurl_txt.top(), { by_hazurl_txt.size(), app.height_property() });

    app.set_constraint<[] (float window_height) {
        return window_height / 40.f;
    }>(by_hazurl_txt.size(), { app.height_property() });



    //////////////
    // Quit
    //////////////

    quit_txt.setFillColor(sf::Color::White);

    app.set_constraint<[] (float left_offset, float width, float w_width) {
        return w_width - left_offset - width - 10.f;
    }>(quit_txt.left(), { quit_txt.left_offset(), quit_txt.width(), app.width_property() });

    app.set_constraint<[] (float top_offset) {
        return top_offset + 10.f;
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
    title_txt.free_properties(gui);
    login_txt.free_properties(gui);
    pseudo_txt.free_properties(gui);
    connecting_txt.free_properties(gui);
    by_hazurl_txt.free_properties(gui);
    quit_txt.free_properties(gui);
    quit_button.free_properties(gui);
}


void Graphics::cursor_left() {
    cursor_index = std::max(std::size_t{ 1 }, cursor_index) - 1;
}

void Graphics::cursor_right() {
    cursor_index = std::min(cursor_index + 1, pseudo_txt.getString().getSize());
}

void Graphics::on_character_entered(char c) {
    // Del
    if(c == 8) {
        if (cursor_index > 0) {
            auto string = pseudo_txt.getString();
            string.erase(cursor_index - 1);
            pseudo_txt.setString(string);

            --cursor_index;
        }
        return;
    }

    // Supr
    if(c == 127) {
        auto string = pseudo_txt.getString();
        string.erase(cursor_index);
        pseudo_txt.setString(string);
        return;
    }


    if (c <= 32 || c >= 127) {
        return;
    }

    auto string = pseudo_txt.getString();
    string.insert(cursor_index, sf::String(c));
    pseudo_txt.setString(string);
    ++cursor_index;
}

std::optional<std::string> Graphics::validate_username() {
    auto username = pseudo_txt.getString().toAnsiString();

    // Too long or too short
    if (username.size() < 3 || username.size() > 20) {
        return std::nullopt;
    } 

    // Contains invalid characters
    if (!std::all_of(std::begin(username), std::end(username), [] (auto c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    })) {
        return std::nullopt;
    }

    return { std::move(username) };
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

    pseudo_cursor.setSize({
        app.get_font().getGlyph('X', pseudo_txt.getCharacterSize(), false, 0).bounds.width,
        2
    });
    {
        auto position = pseudo_txt.findCharacterPos(cursor_index);
        if (pseudo_txt.getString().getSize() == 0) {
            position.x -= pseudo_cursor.getSize().x / 2.f;
        }
        pseudo_cursor.setPosition({
            position.x,
            position.y + pseudo_txt.getCharacterSize() + 3
        });
    }
}

void Graphics::notify_gui(gui::Gui<>& gui) const {
    title_txt.notify_gui(gui);
    login_txt.notify_gui(gui);
    pseudo_txt.notify_gui(gui);
    connecting_txt.notify_gui(gui);
    by_hazurl_txt.notify_gui(gui);
    quit_txt.notify_gui(gui);
    quit_button.notify_gui(gui);
}

void Graphics::update_properties(gui::Gui<> const& gui) {
    title_txt.update_properties(gui);
    login_txt.update_properties(gui);
    pseudo_txt.update_properties(gui);
    connecting_txt.update_properties(gui);
    by_hazurl_txt.update_properties(gui);
    quit_txt.update_properties(gui);
    quit_button.update_properties(gui);
}

void Graphics::draw(bool is_connecting, sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(quit_button, states);

    target.draw(title_txt, states);
    target.draw(login_txt, states);
    target.draw(pseudo_txt, states);
    target.draw(pseudo_cursor, states);

    if (is_connecting) {
        target.draw(connecting_txt, states);
    }

    target.draw(by_hazurl_txt, states);
    target.draw(quit_txt, states);
}

}