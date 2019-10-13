#pragma once

#include <SFML/Graphics.hpp>

#include <cmath>

namespace pong::client::gui {

struct RoundedRectangle : sf::Shape {
public:

    RoundedRectangle(sf::Vector2f _size, float _radius, unsigned _additional_precision = 2);

    std::size_t getPointCount() const override;
    sf::Vector2f getPoint(std::size_t index) const override;


    void setSize(sf::Vector2f const& _size);
    sf::Vector2f const& getSize() const;


    void setRadius(float _radius);
    float getRadius() const;


    void setPrecision(unsigned _additional_precision);
    unsigned getPrecision() const;
    

private:

    sf::Vector2f size;
    float radius;
    unsigned additional_precision;

};

}