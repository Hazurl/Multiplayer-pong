#include <pong/client/state/MainLobby/Graphics.hpp>

namespace pong::client::state::mainlobby {

Graphics::Graphics(Application app)
:   title_txt(app.gui_allocator(), "MULTIPLAYER PONG", app.get_font(), 32)
,   by_hazurl_txt(app.gui_allocator(), "by Hazurl", app.get_font(), 16)
,   quit_txt(app.gui_allocator(), "QUIT", app.get_font(), 16)
,   quit_button(
        app.gui_allocator(), 
        [] () {},
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   create_room_txt(app.gui_allocator(), "Create Room", app.get_font(), 32)
,   create_room_button(
        app.gui_allocator(), 
        [] () {},
        gui::Button::Theme{
            sf::Color{ 0xFF, 0xFF, 0xFF },
            sf::Color{ 0xD6, 0xD6, 0xD3 },
            sf::Color{ 0xFF, 0x99, 0x00 },
        })
,   people_texture{}
,   people_spr{ app.gui_allocator(), people_texture }
,   people_count_txt{ app.gui_allocator(), "0", app.get_font(), 16 }
{

    if (!people_texture.loadFromFile("../assets/people.png")) {
        ERROR("Couldn't load '../assets/people.png'");
        throw std::runtime_error("Couldn't load '../assets/people.png'");
    }

    people_spr.setTexture(&people_texture, true);
    {
        auto s = people_texture.getSize();
        people_spr.setSize({ static_cast<float>(s.x), static_cast<float>(s.y) });
    }

    //////////////
    // Title
    //////////////

    title_txt.setFillColor({ 0xFF, 0x99, 0x00 });

    app.set_constraint<[] (float width, float window_width) {
        return (window_width - width) / 2.f;
    }>(title_txt.left(), { title_txt.width(), app.width_property() });

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(title_txt.top(), { title_txt.top_offset() });


    //////////////
    // Spectator count
    //////////////

    app.set_constraint<[] (float top_offset) {
        return 10.f - top_offset;
    }>(people_count_txt.top(), { people_count_txt.top_offset() });

    app.set_constraint<[] (float left_offset) {
        return 10.f - left_offset;
    }>(people_count_txt.left(), { people_count_txt.left_offset() });


    //////////////
    // Spectator sprite
    //////////////

    app.set_constraint<[] (float height, float sc_top, float sc_top_offset, float sc_height) {
        return sc_top + sc_top_offset + (sc_height - height) / 2.f;
    }>(people_spr.top(), { people_spr.height(), people_count_txt.top(), people_count_txt.top_offset(), people_count_txt.height() });

    app.set_constraint<[] (float sc_left, float sc_left_offset, float sc_width) {
        return sc_left + sc_left_offset + sc_width + 10.f;
    }>(people_spr.left(), { people_count_txt.left(), people_count_txt.left_offset(), people_count_txt.width() });

    app.set_constraint<[] (float sc_height) {
        return sc_height;
    }>(people_spr.height(), { people_count_txt.height() });

    app.set_constraint<[] (float height, float ratio) {
        return height * ratio;
    }>(people_spr.width(), { people_spr.height(), people_spr.ratio() });


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
    // Create room
    //////////////

    create_room_txt.setFillColor(sf::Color{ 0x3A, 0x3C, 0x46 });

    app.set_constraint<[] (float window_width, float width, float left_offset) {
        return (window_width - width) / 2.f - left_offset;
    }>(create_room_txt.left(), { app.width_property(), create_room_txt.width(), create_room_txt.left_offset() });

    app.set_constraint<[] (float height, float top_offset, float window_height) {
        return window_height - top_offset - 50.f - height;
    }>(create_room_txt.top(), { create_room_txt.height(), create_room_txt.top_offset(), app.height_property() });


    //////////////
    // Create room button
    //////////////

    create_room_button.set_rounded_precision(0);

    app.set_constraint<[] (float cr_left, float cr_left_offset) {
        return cr_left + cr_left_offset - 50.f;
    }>(create_room_button.left(), { create_room_txt.left(), create_room_txt.left_offset() });

    app.set_constraint<[] (float cr_top, float cr_top_offset) {
        return cr_top + cr_top_offset - 20.f;
    }>(create_room_button.top(), { create_room_txt.top(), create_room_txt.top_offset() });

    app.set_constraint<[] (float cr_width) {
        return cr_width + 100.f;
    }>(create_room_button.width(), { create_room_txt.width() });

    app.set_constraint<[] (float cr_height) {
        return cr_height + 40.f;
    }>(create_room_button.height(), { create_room_txt.height() });

}

void Graphics::free_properties(gui::Allocator<> gui) const {
    title_txt.free_properties(gui);
    by_hazurl_txt.free_properties(gui);
    quit_txt.free_properties(gui);
    quit_button.free_properties(gui);
    create_room_txt.free_properties(gui);
    create_room_button.free_properties(gui);
    people_count_txt.free_properties(gui);
    people_spr.free_properties(gui);
}


std::optional<Graphics::Button> Graphics::on_click(sf::Vector2f const& position) {
    if (quit_button.on_click(position)) {
        return Graphics::Button::Quit;
    }

    if (create_room_button.on_click(position)) {
        return Graphics::Button::CreateRoom;
    }

    return std::nullopt;
}

std::optional<Graphics::Button> Graphics::on_release_click(sf::Vector2f const& position) {
    if (quit_button.on_release_click(position)) {
        return Graphics::Button::Quit;
    }

    if (create_room_button.on_release_click(position)) {
        return Graphics::Button::CreateRoom;
    }

    return std::nullopt;
}

void Graphics::on_hover(sf::Vector2f const& position) {
    quit_button.on_hover(position);
    create_room_button.on_hover(position);
}


void Graphics::set_people_count(unsigned people_count) {
    people_count_txt.setString(std::to_string(people_count));
}


void Graphics::update_animations(Application app, float dt) {
    quit_button.update(dt);
    create_room_button.update(dt);
}

void Graphics::notify_gui(gui::Gui<>& gui) const {
    title_txt.notify_gui(gui);
    by_hazurl_txt.notify_gui(gui);
    quit_txt.notify_gui(gui);
    quit_button.notify_gui(gui);
    create_room_txt.notify_gui(gui);
    create_room_button.notify_gui(gui);
    people_count_txt.notify_gui(gui);
    people_spr.notify_gui(gui);
}

void Graphics::update_properties(gui::Gui<> const& gui) {
    title_txt.update_properties(gui);
    by_hazurl_txt.update_properties(gui);
    quit_txt.update_properties(gui);
    quit_button.update_properties(gui);
    create_room_txt.update_properties(gui);
    create_room_button.update_properties(gui);
    people_count_txt.update_properties(gui);
    people_spr.update_properties(gui);
}

void Graphics::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(title_txt, states);
    target.draw(by_hazurl_txt, states);
    target.draw(quit_button, states);
    target.draw(quit_txt, states);
    target.draw(create_room_button, states);
    target.draw(create_room_txt, states);
    target.draw(people_count_txt, states);
    target.draw(people_spr, states);
}

}