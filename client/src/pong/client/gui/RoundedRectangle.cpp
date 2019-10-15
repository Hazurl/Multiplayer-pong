#include <SFML/Graphics.hpp>

#include <pong/client/gui/RoundedRectangle.hpp>

#include <cmath>

namespace pong::client::gui {


RoundedRectangle::RoundedRectangle(sf::Vector2f _size, float _radius, unsigned _additional_precision) 
:   size(_size)
,   radius(_radius)
,   additional_precision(_additional_precision) 
{
    update();
}

std::size_t RoundedRectangle::getPointCount() const {
    return 4 * (additional_precision + 1);
}

sf::Vector2f RoundedRectangle::getPoint(std::size_t index) const {

    if (additional_precision == 0) {
        switch(index) {
            case 0: return { size.x, 0 };
            case 1: return { 0, 0 };
            case 2: return { 0, size.y };
            default:return size;
        }
    }

    float clamped_radius = std::min(std::min(size.x, size.y) / 2.f, radius);

    constexpr float _2pi = 6.28318530718;

    std::size_t quadrant = index / (additional_precision + 1);
    std::size_t circle_index = index - quadrant;
    float angle = circle_index * _2pi / static_cast<float>(4 * additional_precision);
    
    float x = (std::cos(angle) + 1) * clamped_radius;
    float y = (std::sin(angle) + 1) * clamped_radius;

    if (quadrant == 0  || quadrant == 3) { // Right
        x += size.x - clamped_radius * 2.f;
    }
    if (quadrant == 0  || quadrant == 1) { // Bottom
        y += size.y - clamped_radius * 2.f;
    }


    return { x, y };
}

void RoundedRectangle::setSize(sf::Vector2f const& _size) {
    size = _size;
    update();
}


sf::Vector2f const& RoundedRectangle::getSize() const {
    return size;
}


void RoundedRectangle::setRadius(float _radius) {
    radius = _radius;
    update();
}

float RoundedRectangle::getRadius() const {
    return radius;
}


void RoundedRectangle::setPrecision(unsigned _additional_precision) {
    additional_precision = _additional_precision;
    update();
}

unsigned RoundedRectangle::getPrecision() const {
    return additional_precision;
}

}