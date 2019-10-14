#pragma once

#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

#include <multipong/Packets.hpp>

#include <variant>

namespace pong::client {

struct Resized : sf::Event::SizeEvent {};
struct TextEntered : sf::Event::TextEvent {};
struct KeyPressed : sf::Event::KeyEvent {};
struct KeyReleased : sf::Event::KeyEvent {};
struct MouseButtonPressed : sf::Event::MouseButtonEvent {};
struct MouseButtonReleased : sf::Event::MouseButtonEvent {};
struct MouseMoved : sf::Event::MouseMoveEvent {};
struct MouseWheelScrolled : sf::Event::MouseWheelScrollEvent {};

using WindowEvent = std::variant<
    Resized,
    TextEntered,
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseWheelScrolled
>;

}