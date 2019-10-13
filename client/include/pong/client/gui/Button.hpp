#pragma once

#include <SFML/Graphics.hpp>

#include <sftk/animated/Animated.hpp>
#include <sftk/eventListener/EventListener.hpp>
#include <sftk/print/Printer.hpp>
#include <sftk/fancyText/FancyText.hpp>

#include <pong/client/gui/RoundedRectangle.hpp>
#include <pong/client/gui/Element.hpp>
#include <pong/client/gui/Color.hpp>

#include <functional>
#include <cmath>
#include <iostream>

namespace pong::client::gui {

struct Button : RectElement<Button>, sf::Transformable, sf::Drawable, sftk::EventListener {
public:

    /*
        General Structures
    */


    enum class State {
        Idle, Hovered, Clicked
    };




    struct Theme {

        Theme(
            sf::Color _idle = { 0x34, 0x98, 0xDB }
        ,   sf::Color _hovered = { 0x25, 0x6C, 0x9C }
        ,   sf::Color _clicked = { 0x16, 0x40, 0x5C }) 
        :   idle(_idle)
        ,   hovered(_hovered)
        ,   clicked(_clicked)
        {}

        sf::Color idle;
        sf::Color hovered;
        sf::Color clicked;

        sf::Color const& get(State state) const;
        sf::Color& get(State state);
    };



    /*
        Special methods
    */


    Button(Gui<>& gui, std::function<void()> _on_click, Theme _theme = {});



    /*
        Concept methods
    */


private:

    void change_color();
    void change_state(State _state);



public:

    /*
        Setters/Getters
    */


    void set_theme(Theme _theme);

    void setSize(sf::Vector2f const& _size);
    sf::Vector2f const& getSize() const;

    void set_rounded_radius(float _radius);
    float get_rounded_radius() const;

    void set_rounded_precision(unsigned _precision);
    unsigned get_rounded_precision() const;


    /*
        Update animations
    */

    void update(float dt);



    /* 
        sftk::EventListener
    */

    sftk::PropagateEvent on_mouse_button_pressed(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_button_released(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_moved(sf::Window&, sf::Event::MouseMoveEvent const& b) override;


    /* 
        sf::Drawable
    */

    void draw(sf::RenderTarget& target, sf::RenderStates state) const override;


    /*
        Debug
    */

    void debug_draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:

    RoundedRectangle rectangle;
    sftk::Animated<sf::Color> color;
    Theme theme;
    State state;
    std::function<void()> on_click;


};

}