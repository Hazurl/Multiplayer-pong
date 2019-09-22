#pragma once

#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

namespace pong {

enum class Input {
    Idle = 0, Up = 1, Down = 2
};

enum class CollisionEvent {
    LeftBoundary,
    RightBoundary,
    None
};

float normalize_input(Input input);

struct Pad {
    float y;
    float speed;

    void update(float dt, Input input, float max_speed, float board_height);
    void update(float dt, float board_height);
};

bool operator==(Pad const& lhs, Pad const& rhs);

struct Ball {
    sf::Vector2f position;
    sf::Vector2f speed;

    CollisionEvent update(float dt, float pad_left, float pad_right, sf::Vector2f const& boundaries, float padding, float pad_height, float pad_width, float ball_radius);
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

sf::Packet& operator >>(sf::Packet& packet, Input& input);
sf::Packet& operator <<(sf::Packet& packet, Ball const& ball);
sf::Packet& operator <<(sf::Packet& packet, Pad const& pad);


// Client side

sf::Packet& operator <<(sf::Packet& packet, Input input);
sf::Packet& operator >>(sf::Packet& packet, Ball& ball);
sf::Packet& operator >>(sf::Packet& packet, Pad& pad);

}