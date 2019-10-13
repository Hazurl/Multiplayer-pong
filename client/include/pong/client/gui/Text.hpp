#pragma once

#include <SFML/Graphics.hpp>

#include <sftk/animated/Animated.hpp>
#include <sftk/eventListener/EventListener.hpp>
#include <sftk/print/Printer.hpp>
#include <sftk/fancyText/FancyText.hpp>

#include <pong/client/gui/RoundedRectangle.hpp>
#include <pong/client/gui/Element.hpp>
#include <pong/client/gui/Color.hpp>

#include <cmath>
#include <iostream>

namespace pong::client::gui {

struct Text : Element, sf::Text {
public:

    /*
        Special methods
    */


    Text(Gui<>& gui, sf::String const& str, sf::Font const& font, unsigned char size = 30);



    /*
        Setters/Getters
    */

    sf::Vector2f getSize() const;
    sf::Vector2f get_offset() const;


    /*
        GUI
    */

    void free_properties(Gui<>& gui);

    property_id_t left_offset() const; // read-only
    property_id_t top_offset() const; // read-only
    property_id_t left() const;
    property_id_t top() const;
    property_id_t width() const; // read-only
    property_id_t height() const; // read-only
    property_id_t size() const;


    void update_properties(Gui<> const& gui) override;
    void notify_gui(Gui<>& gui) const override;


    /*
        Debug
    */

    void debug_draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:

    static constexpr std::size_t left_offset_id_offset = 0;
    static constexpr std::size_t top_offset_id_offset = 1;
    static constexpr std::size_t width_id_offset = 2;
    static constexpr std::size_t height_id_offset = 3;
    static constexpr std::size_t size_id_offset = 4;
    static constexpr std::size_t left_id_offset = 5;
    static constexpr std::size_t top_id_offset = 6;


    property_id_t id;


};

}