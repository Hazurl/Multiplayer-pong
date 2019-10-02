#pragma once

#include <SFML/Graphics.hpp>

#include <sftk/animated/Animated.hpp>
#include <sftk/eventListener/EventListener.hpp>
#include <sftk/print/Printer.hpp>
#include <sftk/fancyText/FancyText.hpp>

#include <pong/client/gui/RoundedRectangle.hpp>

#include <functional>
#include <cmath>
#include <iostream>

namespace pong::client::gui {

struct HSLColor {
    float h;
    float s;
    float l;
    sf::Uint8 a;
};

static HSLColor rgb_to_hsl(sf::Color const& rgb) {
    float r = rgb.r / 255.0f;
    float g = rgb.g / 255.0f;
    float b = rgb.b / 255.0f;

    float* c_max;
    if (r >= g && r >= b) {
        c_max = &r;
    }
    else if (g >= r && g >= b) {
        c_max = &g;
    }
    else {
        c_max = &b;
    }

    float c_min = std::min(r, std::min(g, b));
    float diff = *c_max - c_min;

    float h = 0;
    if (diff != 0) {
        if(c_max == &r) {
            h = 60.f * std::fmod((g - b) / diff, 6.f); 
        } 
        else if (c_max == &g) {
            h = 60.f * ((b - r) / diff + 2.f); 
        }
        else {
            h = 60.f * ((r - g) / diff + 4.f); 
        }
    }

    float l = (*c_max + c_min) / 2.f;
    float s = diff == 0 ? 0.f : (diff / (1 - std::abs(2 * l - 1)));

    return HSLColor{
        h, s, l, rgb.a
    };
}

static sf::Color hsl_to_rgb(HSLColor const& hsl) {
    float c = (1.f - std::abs(2.f * hsl.l - 1.f)) * hsl.s;
    float x = c * (1.f - std::abs(std::fmod(hsl.h / 60.f, 2.f) - 1.f));
    float m = hsl.l - c/2.f;

    float r = 0, g = 0, b = 0;
    if (hsl.h < 60.f) {
        r = c;
        g = x;
    }
    else if (hsl.h < 120.f) {
        r = x;
        g = c;
    }
    else if (hsl.h < 180.f) {
        g = c;
        b = x;
    }
    else if (hsl.h < 240.f) {
        g = x;
        b = c;
    }
    else if (hsl.h < 300.f) {
        r = x;
        b = c;
    }
    else {
        r = c;
        b = x;
    }

    return sf::Color(
        static_cast<sf::Uint8>((r + m) * 255.f),
        static_cast<sf::Uint8>((g + m) * 255.f),
        static_cast<sf::Uint8>((b + m) * 255.f),
        hsl.a
    );
}

static sf::Color color_transition(sf::Color const& start, sf::Color const& end, sftk::interpolation::ratio_t ratio) {
    //return end;
    auto start_hsl = rgb_to_hsl(start);
    auto end_hsl = rgb_to_hsl(end);

    float h = std::abs(start_hsl.h - end_hsl.h) <= std::abs(end_hsl.h - start_hsl.h) ? 
        sftk::interpolation::linear(start_hsl.h, end_hsl.h, ratio)
    :   sftk::interpolation::linear(end_hsl.h, start_hsl.h, ratio);

    HSLColor current_hsl_color{
        h,
        sftk::interpolation::linear(start_hsl.s, end_hsl.s, ratio),
        sftk::interpolation::linear(start_hsl.l, end_hsl.l, ratio),
        sftk::interpolation::linear(start_hsl.a, end_hsl.a, ratio)
    };

    return hsl_to_rgb(current_hsl_color);
}

struct Button : sf::Transformable, sf::Drawable {
public:

    struct Customization {

        struct Colors {
            sf::Color bg;
            sf::Color text;
        };

        Customization(
            Colors _idle = { sf::Color{ 0x34, 0x98, 0xDB }, sf::Color::White }
        ,   Colors _hovered = { sf::Color{ 0x25, 0x6C, 0x9C }, sf::Color::White }
        ,   Colors _clicked = { sf::Color{ 0x16, 0x40, 0x5C }, sf::Color::White }) 
        :   idle(_idle)
        ,   hovered(_hovered)
        ,   clicked(_clicked)
        {}

        Colors idle;
        Colors hovered;
        Colors clicked;
    };

    Button(std::function<void()> _on_click, sf::Font const& font, sf::String str, unsigned height, float padding = 8, Customization _custom = Customization())
    :   text(str, font, height)
    ,   rectangle({ text.getLocalBounds().width + padding * 2, text.getLocalBounds().height + padding * 2 }, padding, 5)
    ,   bg_color(&color_transition, _custom.idle.bg) 
    ,   text_color(&color_transition, _custom.idle.text) 
    ,   custom(_custom)
    ,   state(State::Idle)
    ,   on_click(std::move(_on_click))
    {
        auto pos = rectangle.get_size() / 2.f;
        text.setPosition(pos);
        auto bounds = text.getLocalBounds();
    	text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        text.setFillColor(text_color);

        rectangle.setFillColor(bg_color);
    }


    void draw(sf::RenderTarget& target, sf::RenderStates state) const override {
        state.transform *= getTransform();
        target.draw(rectangle, state);
        target.draw(text, state);
    }

    void update(float dt) {
        bg_color.update(dt);
        rectangle.setFillColor(bg_color);

        text_color.update(dt);
        text.setFillColor(text_color);
    }

    sftk::PropagateEvent on_mouse_button_pressed(sf::Event::MouseButtonEvent const& b) {
        bool hovering = getTransform().transformRect(rectangle.getGlobalBounds()).contains(static_cast<float>(b.x), static_cast<float>(b.y));
        if (hovering) {
            state = State::Clicked;
            bg_color.animate(custom.clicked.bg, transition_duration);
            text_color.animate(custom.clicked.text, transition_duration);
            return false;
        }

        return true;
    }

    sftk::PropagateEvent on_mouse_button_released(sf::Event::MouseButtonEvent const& b) {
        if (state == State::Clicked) {
            
            bool hovering = getTransform().transformRect(rectangle.getGlobalBounds()).contains(static_cast<float>(b.x), static_cast<float>(b.y));
            if (hovering) {
                state = State::Hovered;
                bg_color.animate(custom.hovered.bg, transition_duration);
                text_color.animate(custom.hovered.text, transition_duration);
                on_click();
                return false;
            } else {
                state = State::Idle;
                bg_color.animate(custom.idle.bg, transition_duration);
                text_color.animate(custom.idle.text, transition_duration);
            }
        }

        return true;
    }

    sftk::PropagateEvent on_mouse_moved(sf::Event::MouseMoveEvent const& b) {
        if (state == State::Clicked) {
            return true;
        }

        bool hovering = getTransform().transformRect(rectangle.getGlobalBounds()).contains(static_cast<float>(b.x), static_cast<float>(b.y));
        if (hovering && state != State::Hovered) {
            state = State::Hovered;
            bg_color.animate(custom.hovered.bg, transition_duration);
            text_color.animate(custom.hovered.text, transition_duration);
        } else if (!hovering && state == State::Hovered) {
            state = State::Idle;
            bg_color.animate(custom.idle.bg, transition_duration);
            text_color.animate(custom.idle.text, transition_duration);
        }
        return true;
    }


private:

    enum class State {
        Idle, Hovered, Clicked
    };

    sf::Text text;
    RoundedRectangle rectangle;
    sftk::Animated<sf::Color> bg_color;
    sftk::Animated<sf::Color> text_color;
    Customization custom;
    State state;
    std::function<void()> on_click;

    static constexpr float transition_duration = 0.1;

};

}