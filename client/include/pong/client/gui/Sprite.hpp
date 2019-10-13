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

struct Sprite : Element, sf::RectangleShape {
public:

    /*
        Special methods
    */


    Sprite(Gui<>& gui, sf::Texture const& texture);


    /*
        GUI
    */

    void free_properties(Gui<>& gui);

    property_id_t left() const;
    property_id_t top() const;
    property_id_t width() const;
    property_id_t height() const;
    property_id_t ratio() const; // read only


    void update_properties(Gui<> const& gui) override;
    void notify_gui(Gui<>& gui) const override;

    /* 
        Debug
    */

    void debug_draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:

    static constexpr std::size_t left_id_offset = 0;
    static constexpr std::size_t top_id_offset = 1;
    static constexpr std::size_t width_id_offset = 2;
    static constexpr std::size_t height_id_offset = 3;
    static constexpr std::size_t ratio_id_offset = 4;


    property_id_t id;


};

}