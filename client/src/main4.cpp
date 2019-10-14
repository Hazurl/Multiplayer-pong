#include <SFML/Graphics.hpp>

#include <pong/client/StateSupervisor.hpp>

#include <pong/client/state/Login.hpp>


int main() {
    auto supervisor = pong::client::make_supervisor<pong::client::state::Login>(sf::VideoMode(800, 600), "SFML Multiplayer pong", sf::Color::Red);

    supervisor.loop();
}