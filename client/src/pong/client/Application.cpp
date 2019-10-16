#include <pong/client/Application.hpp>

#include <pong/client/gui/constraint/Interface.hpp>
#include <pong/client/net/Connection.hpp>
#include <pong/client/state/State.hpp>

namespace pong::client {

Application::Application(gui::RectProperties _window_properties, sf::Vector2u _window_size, net::Connection& _connection, gui::Gui<>& _gui, sf::Font const& _font) 
:   Allocator<>{ _gui }
,   window_properties{ _window_properties }
,   window_size{ _window_size }
,   connection{ _connection }
,   font{ _font }
{}



gui::Allocator<>& Application::gui_allocator() {
    return *this;
}



bool Application::is_connected() const {
    return connection.is_connected();
}

bool Application::is_connecting() const {
    return connection.is_connecting();
}



sf::Font const& Application::get_font() const {
    return font;
}



unsigned Application::width() const {
    return window_size.x;
}

unsigned Application::height() const {
    return window_size.y;
}


gui::property_id_t Application::width_property() const {
    return window_properties.width();
}

gui::property_id_t Application::height_property() const {
    return window_properties.height();
}


}