#pragma once

#include <SFML/Graphics.hpp>

#include <cmath>

namespace pong::client::gui {

struct RoundedRectangle : sf::Shape {
public:

    RoundedRectangle(sf::Vector2f _size, float _radius, std::size_t _additional_precision = 2) 
    :   radius(_radius)
    ,   additional_precision(_additional_precision) 
    {
        set_size(_size);
    }

    std::size_t getPointCount() const override {
        return 4 * (additional_precision + 1);
    }

    sf::Vector2f getPoint(std::size_t index) const override {
        float clamped_radius = std::min(std::min(size.x, size.y) / 2.f, radius);


        std::size_t circle_index = index - (index / (additional_precision + 1));
        float angle = circle_index * 6.28318530718 / (getPointCount() - 4.f);


        float x = (std::cos(angle) + 1) * clamped_radius; // [0, 40]
        float y = (std::sin(angle) + 1) * clamped_radius; // [0, 40]


        auto quadrant = index * 4 / getPointCount();
        if (quadrant == 0  || quadrant == 3) { // Left
            x += size.x - clamped_radius * 2.f;
        }
        if (quadrant == 0  || quadrant == 1) { // Bottom
            y += size.y - clamped_radius * 2.f;
        }


        return { x, y };
    }

    void set_size(sf::Vector2f const& _size) {
        size = _size;
        update();
    }


    sf::Vector2f const& get_size() const {
        return size;
    }

private:

    sf::Vector2f size;
    float radius;
    std::size_t additional_precision;

};

}