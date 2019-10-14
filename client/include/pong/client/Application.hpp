#pragma once

#include <SFML/Graphics.hpp>

#include <pong/client/gui/constraint/Constraint.hpp>
#include <pong/client/gui/constraint/Solver.hpp>
#include <pong/client/gui/Element.hpp>
#include <pong/client/gui/constraint/Interface.hpp>

#include <pong/client/net/Connection.hpp>


#include <vector>

namespace pong::client {

class Application {
public:

    Application(gui::RectProperties window_properties, net::Connection& connection, gui::Gui<>& gui, sf::Font const& font);

    bool is_connected() const;
    bool is_connecting() const;

    gui::property_id_t allocate_property(float value = 0.0);
    gui::property_id_t allocate_properties(std::size_t const count);

    void free_property(std::size_t const index);
    void free_properties(gui::property_id_t const index, std::size_t const count);

    template<auto f>
    void set_constraint(gui::property_id_t id, std::vector<gui::property_id_t> dependencies) {
        return gui.set_constraint<f>(id, dependencies);
    }

    sf::Font const& get_font() const;


private:

    gui::RectProperties window_properties;
    net::Connection& connection;
    gui::Gui<>& gui;
    sf::Font const& font;

};

}