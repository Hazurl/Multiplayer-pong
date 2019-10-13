#include <SFML/Graphics.hpp>

#include <sftk/animated/Animated.hpp>

#include <pong/client/gui/Color.hpp>

#include <cmath>

namespace pong::client::gui {


HSLColor rgb_to_hsl(sf::Color const& rgb) {
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



sf::Color hsl_to_rgb(HSLColor const& hsl) {
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



sf::Color color_transition(sf::Color const& start, sf::Color const& end, sftk::interpolation::ratio_t ratio) {
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


}