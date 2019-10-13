#pragma once

#include <SFML/Graphics.hpp>

#include <sftk/animated/Animated.hpp>

#include <cmath>

namespace pong::client::gui {


struct HSLColor {
    float h;
    float s;
    float l;
    sf::Uint8 a;
};

HSLColor rgb_to_hsl(sf::Color const& rgb);
sf::Color hsl_to_rgb(HSLColor const& hsl);
sf::Color color_transition(sf::Color const& start, sf::Color const& end, sftk::interpolation::ratio_t ratio);


}