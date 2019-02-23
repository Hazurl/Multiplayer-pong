#include <multipong/Game.hpp>

namespace pong {

float normalize_input(Input input) {
    return input == Input::Idle ? 0.f :
           input == Input::Up ? -1.f : 1.f;
}

void Ball::update(float dt, sf::Vector2f const& boundaries) {
    position += dt * speed;

    if (position.x > boundaries.x) {
        auto tmp_dt = dt - (position.x - boundaries.x) / speed.x;
        speed.x = -speed.x;
        position.x = boundaries.x + speed.x * tmp_dt;
    } else if (position.x < 0) {
        auto tmp_dt = dt - -position.x / speed.x;
        speed.x = -speed.x;
        position.x = speed.x * tmp_dt;
    }

    if (position.y > boundaries.y) {
        auto tmp_dt = dt - (position.y - boundaries.y) / speed.y;
        speed.y = -speed.y;
        position.y = boundaries.y + speed.y * tmp_dt;
    } else if (position.y < 0) {
        auto tmp_dt = dt - -position.y / speed.y;
        speed.y = -speed.y;
        position.y = speed.y * tmp_dt;
    }
}

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

sf::Packet& operator <<(sf::Packet& packet, Pad const& pad) {
    return packet << pad.y << pad.speed;
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