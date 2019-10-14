#include <pong/client/Application.hpp>

#include <pong/client/gui/constraint/Interface.hpp>
#include <pong/client/net/Connection.hpp>
#include <pong/client/state/State.hpp>

namespace pong::client {

Application::Application(gui::RectProperties _window_properties, net::Connection& _connection, gui::Gui<>& _gui, sf::Font const& _font) 
:   window_properties{ _window_properties }
,   connection{ _connection }
,   gui{ _gui }
,   font{ _font }
{}



bool Application::is_connected() const {
    return connection.is_connected();
}

bool Application::is_connecting() const {
    return connection.is_connecting();
}



gui::property_id_t Application::allocate_property(float value) {
    return gui.allocate_property(value);
}

gui::property_id_t Application::allocate_properties(std::size_t const count) {
    return gui.allocate_properties(count);
}



void Application::free_property(std::size_t const index) {
    return gui.free_property(index);
}

void Application::free_properties(gui::property_id_t const index, std::size_t const count) {
    return gui.free_properties(index, count);
}



sf::Font const& Application::get_font() const {
    return font;
}


}