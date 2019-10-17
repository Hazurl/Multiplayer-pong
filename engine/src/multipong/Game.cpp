#include <multipong/Game.hpp>

namespace pong {

float normalize_input(Input input) {
    return input == Input::Idle ? 0.f :
           input == Input::Up ? -1.f : 1.f;
}

Ball::Ball() 
:   position{ meta::ball::bounds_x / 2.f, meta::ball::bounds_y / 2.f }
,   speed{ meta::ball::max_speed, meta::ball::max_speed }
{}

Ball::Ball(sf::Vector2f const& _position, sf::Vector2f const& _speed) 
:   position{ _position }
,   speed{ _speed }
{}

CollisionEvent Ball::update(float dt, float pad_left, float pad_right, sf::Vector2f const& boundaries, float padding, float pad_height, float pad_width, float ball_radius) {
    position += dt * speed;
    auto event = CollisionEvent::None;

    // moving up
    if (speed.y < 0) {
        // bounding box, top
        if (position.y < 0) {
            speed.y = - speed.y;
            position.y = - position.y;
        }
    }
    // moving down 
    else {
        // bounding box, bottom
        if (position.y + ball_radius > boundaries.y) {
            speed.y = - speed.y;
            position.y = 2 * boundaries.y - position.y - 2 * ball_radius;
        }
    }


    // moving left
    if (speed.x < 0) {
        // bounding box, left
        if (position.x < 0) {
            //std::cout << "LEFT: " << speed.x << ", " << position.x;
            speed.x = - speed.x;
            position.x = - position.x;
            event = CollisionEvent::LeftBoundary;
            //std::cout << "\t|\t" << speed.x << ", " << position.x << '\n';
        }

        // in the left pad
        if (position.x >= padding - pad_width && position.x <= padding &&
            position.y >= pad_left && position.y <= pad_left + pad_height) {
            
            speed.x = - speed.x;
            position.x = 2 * padding - position.x;
        }
    }
    // moving right
    else {
        // bounding box, right
        if (position.x + ball_radius > boundaries.x) {
            //std::cout << "RIGHT: " << speed.x << ", " << position.x;
            speed.x = - speed.x;
            position.x = 2 * boundaries.x - position.x - 2 * ball_radius;
            event = CollisionEvent::RightBoundary;
            //std::cout << "\t|\t" << speed.x << ", " << position.x << '\n';
        }

        // in the right pad
        if (position.x + ball_radius >= boundaries.x - padding && position.x + ball_radius <= boundaries.x - padding + pad_width &&
            position.y >= pad_right && position.y <= pad_right + pad_height) {
            
            speed.x = - speed.x;
            position.x = 2 * (boundaries.x - padding) - position.x - 2 * ball_radius;
        }
    }

    return event;
}

Pad::Pad() 
:   y{ (meta::pad::bounds_y - meta::pad::width) / 2.f }
,   speed{ 0.0f }
{}

Pad::Pad(float _y, float _speed) 
:   y{ _y }
,   speed{ _speed }
{}

void Pad::update(float dt, Input input, float max_speed, float board_height) {
    speed = max_speed * normalize_input(input);
    return update(dt, board_height);
}

void Pad::update(float dt, float board_height) {
    y += speed * dt;

    if (y > board_height) {
        y = board_height;
    } else if (y < 0) {
        y = 0;
    }
}

sf::Packet& operator >>(sf::Packet& packet, Input& input) {
    int i;

    packet >> i;
    input = static_cast<Input>(i);

    return packet;
}

sf::Packet& operator <<(sf::Packet& packet, Ball const& ball) {
           packet << ball.position.x << ball.position.y;
    return packet << ball.speed   .x << ball.speed   .y;
}

bool operator==(Ball const& lhs, Ball const& rhs) {
    return lhs.position == rhs.position && lhs.speed == rhs.speed;
}

sf::Packet& operator <<(sf::Packet& packet, Pad const& pad) {
    return packet << pad.y << pad.speed;
}

bool operator==(Pad const& lhs, Pad const& rhs) {
    return lhs.y == rhs.y && lhs.speed == rhs.speed;
}

sf::Packet& operator <<(sf::Packet& packet, Input input) {
    return packet << static_cast<int>(input);
}

sf::Packet& operator >>(sf::Packet& packet, Ball& ball) {
           packet >> ball.position.x >> ball.position.y;
    return packet >> ball.speed   .x >> ball.speed   .y;
}

sf::Packet& operator >>(sf::Packet& packet, Pad& pad) {
    return packet >> pad.y >> pad.speed;
}

}