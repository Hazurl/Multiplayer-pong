#pragma once

#include <SFML/Graphics.hpp>

#include <pong/client/gui/constraint/Constraint.hpp>
#include <pong/client/gui/constraint/Solver.hpp>
#include <pong/client/gui/Element.hpp>
#include <pong/client/gui/constraint/Interface.hpp>
#include <pong/client/gui/constraint/Allocator.hpp>

#include <pong/client/net/Connection.hpp>


#include <vector>

namespace pong::client {

class Application : public gui::Allocator<> {
public:

    Application(gui::RectProperties window_properties, sf::Vector2u _window_size, net::Connection& connection, gui::Gui<>& gui, sf::Font const& font);

    gui::Allocator<>& gui_allocator();

    bool is_connected() const;
    bool is_connecting() const;

    sf::Font const& get_font() const;

    unsigned width() const;
    unsigned height() const;

    gui::property_id_t width_property() const;
    gui::property_id_t height_property() const;


private:

    gui::RectProperties window_properties;
    sf::Vector2u window_size;
    net::Connection& connection;
    sf::Font const& font;

};

}