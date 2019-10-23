#include <utility>

#include <pong/client/State.hpp>
#include <pong/client/Common.hpp>
#include <pong/client/InMainLobby.hpp>
#include <pong/client/LoginIn.hpp>
#include <pong/client/gui/constraint/Allocator.hpp>

namespace pong::client {

LoginIn::LoginIn(socket_ptr_t _socket, gui::Gui<>& _gui, gui::RectProperties _window_properties, sf::Font const& _font) 
:   base_t(std::move(_socket), {
        { pong::packet::PacketID::ChangeUsernameResponse, &LoginIn::on_username_response }
    })
,   font{ _font }
,   gui{ _gui }
,   window_properties{ _window_properties }
,   title_txt(gui::Allocator{ gui }, "MULTIPLAYER PONG", font)
,   login_txt(gui::Allocator{ gui }, "Login:", font)
,   pseudo_txt(gui::Allocator{ gui }, "", font)
,   connecting_txt(gui::Allocator{ gui }, "Connecting...", font)
,   by_hazurl_txt(gui::Allocator{ gui }, "by Hazurl", font)
,   quit_txt(gui::Allocator{ gui }, "QUIT", font)
,   quit_button(gui::Allocator{ gui }, 
        [this] () {
            quit = true;
        },
        gui::Button::Theme{
            sf::Color{ 0x3A, 0x3C, 0x46 },
            sf::Color{ 0x31, 0x33, 0x3a },
            sf::Color{ 0x24, 0x25, 0x29 },
        })
,   pseudo_cursor({ 10, 10 }, 0, 0)
,   cursor_index{ 0 }
,   quit{ false }
{

    //////////////
    // Title
    //////////////

    title_txt.setFillColor({ 0xFF, 0x99, 0x00 });

    gui.set_constraint<[] (float width, float window_left, float window_width) {
        return window_left + (window_width - width) / 2.f;
    }>(title_txt.left(), { title_txt.width(), window_properties.left(), window_properties.width() });

    gui.set_constraint<[] (float height, float window_top, float window_height) {
        return window_top - height / 2.f + window_height / 4.f;
    }>(title_txt.top(), { title_txt.size(), window_properties.top(), window_properties.height() });

    gui.set_constraint<[] (float window_height) {
        return window_height / 10.f;
    }>(title_txt.size(), { window_properties.height() });


    //////////////
    // Login
    //////////////

    login_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float width, float window_left, float window_width) {
        return window_left + (window_width - width) / 2.f;
    }>(login_txt.left(), { login_txt.width(), window_properties.left(), window_properties.width() });

    gui.set_constraint<[] (float window_top, float window_height) {
        return window_top + window_height * 0.6f;
    }>(login_txt.top(), { window_properties.top(), window_properties.height() });

    gui.set_constraint<[] (float window_height) {
        return window_height / 25.f;
    }>(login_txt.size(), { window_properties.height() });


    //////////////
    // Pseudo
    //////////////

    pseudo_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float width, float window_left, float window_width) {
        return window_left + (window_width - width) / 2.f;
    }>(pseudo_txt.left(), { pseudo_txt.width(), window_properties.left(), window_properties.width() });

    gui.set_constraint<[] (float login_top, float login_height, float window_height) {
        return login_top + login_height + window_height / 40.f;
    }>(pseudo_txt.top(), { login_txt.top(), login_txt.size(), window_properties.height() });

    gui.set_constraint<[] (float window_height) {
        return window_height / 20.f;
    }>(pseudo_txt.size(), { window_properties.height() });


    //////////////
    // Connecting
    //////////////

    connecting_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float width, float window_left, float window_width) {
        return window_left + (window_width - width) / 2.f;
    }>(connecting_txt.left(), { connecting_txt.width(), window_properties.left(), window_properties.width() });

    gui.set_constraint<[] (float pseudo_top, float pseudo_size, float window_height) {
        return pseudo_top + pseudo_size + window_height / 40.f;
    }>(connecting_txt.top(), { pseudo_txt.top(), pseudo_txt.size(), window_properties.height() });

    gui.set_constraint<[] (float window_height) {
        return window_height / 25.f;
    }>(connecting_txt.size(), { window_properties.height() });


    //////////////
    // By Hazurl
    //////////////

    by_hazurl_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float window_left) {
        return window_left + 10.f;
    }>(by_hazurl_txt.left(), { window_properties.left() });

    gui.set_constraint<[] (float height, float window_top, float window_height) {
        return window_top - height + window_height - 10.f;
    }>(by_hazurl_txt.top(), { by_hazurl_txt.size(), window_properties.top(), window_properties.height() });

    gui.set_constraint<[] (float window_height) {
        return window_height / 40.f;
    }>(by_hazurl_txt.size(), { window_properties.height() });


    //////////////
    // Quit
    //////////////

    quit_txt.setFillColor(sf::Color::White);

    gui.set_constraint<[] (float left, float left_offset) {
        return left + 5.f - left_offset;
    }>(quit_txt.left(), { quit_button.left(), quit_txt.left_offset() });

    gui.set_constraint<[] (float button_top, float button_height, float height, float top_offset) {
        return button_top + (button_height - height) / 2.f - top_offset;
    }>(quit_txt.top(), { quit_button.top(), quit_button.height(), quit_txt.height(), quit_txt.top_offset() });

    gui.set_constraint<[] (float height) {
        return height - 10.f;
    }>(quit_txt.size(), { quit_button.height() });


    //////////////
    // Quit button
    //////////////

    gui.set_constraint<[] (float width, float window_left, float window_width) {
        return window_left + window_width - width - 5.f;
    }>(quit_button.left(), { quit_button.width(), window_properties.left(), window_properties.width() });

    gui.set_constraint<[] (float window_top) {
        return window_top + 5.f;
    }>(quit_button.top(), { window_properties.top() });

    gui.set_constraint<[] (float window_height) {
        return std::floor(window_height / 40.f) + 10.f;
    }>(quit_button.height(), { window_properties.height() });

    gui.set_constraint<[] (float txt_width) {
        return txt_width + 10.f;
    }>(quit_button.width(), { quit_txt.width() });

    pseudo_cursor.setFillColor(sf::Color::White);
}


void LoginIn::update_properties(gui::Gui<> const& _gui) {
    title_txt.update_properties(_gui);
    login_txt.update_properties(_gui);
    pseudo_txt.update_properties(_gui);
    connecting_txt.update_properties(_gui);
    by_hazurl_txt.update_properties(_gui);
    quit_txt.update_properties(_gui);
    quit_button.update_properties(_gui);


    pseudo_cursor.setSize({
        font.getGlyph('X', pseudo_txt.getCharacterSize(), false, 0).bounds.width,
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

void LoginIn::notify_gui(gui::Gui<>& _gui) const {
    title_txt.notify_gui(_gui);
    login_txt.notify_gui(_gui);
    pseudo_txt.notify_gui(_gui);
    connecting_txt.notify_gui(_gui);
    by_hazurl_txt.notify_gui(_gui);
    quit_txt.notify_gui(_gui);
    quit_button.notify_gui(_gui);
}


Action LoginIn::update(float dt) {
    quit_button.update(dt);

    if (quit) {
        return Abord{};
    }

    return Idle{};
}

void LoginIn::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    target.draw(quit_button, states);

    target.draw(title_txt, states);
    target.draw(login_txt, states);
    target.draw(pseudo_txt, states);
    target.draw(pseudo_cursor, states);

    if (is_connecting() || is_connected()) {
        target.draw(connecting_txt, states);
    }

    target.draw(by_hazurl_txt, states);
    target.draw(quit_txt, states);
}


sftk::PropagateEvent LoginIn::on_mouse_button_pressed(sf::Window& w, sf::Event::MouseButtonEvent const& b) {
    return quit_button.on_mouse_button_pressed(w, b) ;
}

sftk::PropagateEvent LoginIn::on_mouse_button_released(sf::Window& w, sf::Event::MouseButtonEvent const& b) {
    return quit_button.on_mouse_button_released(w, b);
}

sftk::PropagateEvent LoginIn::on_mouse_moved(sf::Window& w, sf::Event::MouseMoveEvent const& b) {
    return quit_button.on_mouse_moved(w, b);
}

sftk::PropagateEvent LoginIn::on_key_pressed(sf::Window&, sf::Event::KeyEvent const& b) {
    if (b.code == sf::Keyboard::Enter && !(is_connecting() || is_connected())) {
        attempt_to_connect("127.0.0.1", 48624);


    } else if (b.code == sf::Keyboard::Left) {
        cursor_index = std::max(std::size_t{ 1 }, cursor_index) - 1;


    } else if (b.code == sf::Keyboard::Right) {
        cursor_index = std::min(cursor_index + 1, pseudo_txt.getString().getSize());


    } else {
        return true;
    }
    return false;
}

sftk::PropagateEvent LoginIn::on_text_entered(sf::Window&, sf::Event::TextEvent const& b) {
    if (is_connecting() || is_connected()) {
        return true;
    }

    // Del
    if(b.unicode == 8) {
        if (cursor_index > 0) {
            auto string = pseudo_txt.getString();
            string.erase(cursor_index - 1);
            pseudo_txt.setString(string);

            --cursor_index;
        }
        return false;
    }

    // Supr
    if(b.unicode == 127) {
        auto string = pseudo_txt.getString();
        string.erase(cursor_index);
        pseudo_txt.setString(string);
        return false;
    }


    if (b.unicode <= 32 || b.unicode >= 127) {
        return false;
    }

    auto string = pseudo_txt.getString();
    string.insert(cursor_index, sf::String(b.unicode));
    pseudo_txt.setString(string);
    ++cursor_index;

    return false;
}


void LoginIn::on_connection() {
    send(pong::packet::ChangeUsername{ pseudo_txt.getString().toAnsiString() });
}



Action LoginIn::on_username_response(packet_t packet) {
    auto res = from_packet<pong::packet::ChangeUsernameResponse>(packet).result;
    std::cout << "ChangeUsernameResponse: " << static_cast<int>(res) << '\n';
    if (res == pong::packet::ChangeUsernameResponse::Okay) {
        return change_state<InMainLobby>(gui, window_properties, font, pseudo_txt.getString().toAnsiString());
    }

    return Abord{};
}

}