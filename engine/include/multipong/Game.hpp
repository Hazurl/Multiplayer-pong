#pragma once

#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

namespace pong {

namespace meta {

    static constexpr float bounds_x     { 800 };
    static constexpr float bounds_y     { 600 };

    namespace ball {
        static constexpr float radius   { 8 };
        static constexpr float max_speed{ 100 };
        static constexpr float bounds_x { meta::bounds_x - radius };
        static constexpr float bounds_y { meta::bounds_y - radius };
    }

    namespace pad {
        static constexpr float height   { 80 };
        static constexpr float width    { 12 };
        static constexpr float padding  { 40 };
        static constexpr float max_speed{ 200 };
        static constexpr float bounds_y { meta::bounds_y - height };
    }

}

enum class Input : char {
    Idle = 0, Up = 1, Down = 2
};

enum class Side : char {
    Left, Right
};

enum class CollisionEvent : char {
    LeftBoundary,
    RightBoundary,
    None
};

float normalize_input(Input input);

struct Pad {
    float y;
    float speed;

    Pad();
    Pad(float y, float speed);

    void update(float dt, Input input, float max_speed = meta::pad::max_speed, float board_height = meta::pad::bounds_y);
    void update(float dt, float board_height = meta::pad::bounds_y);
};

bool operator==(Pad const& lhs, Pad const& rhs);

struct Ball {
    sf::Vector2f position;
    sf::Vector2f speed;

    Ball();
    Ball(sf::Vector2f const& position, sf::Vector2f const& speed);

    CollisionEvent update(
        float dt, 
        float pad_left, 
        float pad_right, 
        sf::Vector2f const& boundaries = { meta::bounds_x, meta::bounds_y }, 
        float padding = meta::pad::padding, 
        float pad_height = meta::pad::height, 
        float pad_width = meta::pad::width, 
        float ball_radius = meta::ball::radius);
};

bool operator==(Ball const& lhs, Ball const& rhs);

/* 
           Server     Client

                  <== Input
    Ball Position ==>
       Ball Speed ==>
    Pad Positions ==>
        Pad Speed ==>
*/


// Server side

sf::Packet& operator <<(sf::Packet& packet, Ball const& ball);
sf::Packet& operator <<(sf::Packet& packet, Pad const& pad);


// Client side

sf::Packet& operator >>(sf::Packet& packet, Ball& ball);
sf::Packet& operator >>(sf::Packet& packet, Pad& pad);

}