#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

#include <multipong/Game.hpp>

int main() {
    sf::TcpSocket socket;
    if (socket.connect("127.0.0.1", 48621) != sf::Socket::Done) {
        std::cerr << "Error: Couldn't connect to the server\n";
        std::exit(1);
    }

    socket.setBlocking(false);
    sf::Packet packet;

    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Multiplayer pong");

    sf::RectangleShape player_sprite({ 12, 80 });
    sf::RectangleShape ball_sprite({ 8, 8 });

    ball_sprite.setPosition({ 400, 300 });

    pong::Ball ball;
    pong::Pad pad;

    auto input = pong::Input::Idle;

    sf::Vector2f const boundaries{ 792, 592 };

    sf::Clock clock;

    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            switch( event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }

                default: { break; }
            }
        }

        auto dt = clock.restart().asSeconds();

        ball.update(dt, boundaries);
        pad.update(dt, boundaries.y);

        ball_sprite.setPosition(ball.position);
        player_sprite.setPosition(player_sprite.getPosition().x, pad.y);

        switch(socket.receive(packet)) {
            case sf::Socket::Done: {
                packet >> ball >> pad;
                
                ball_sprite.setPosition(ball.position);
                player_sprite.setPosition(player_sprite.getPosition().x, pad.y);

                packet.clear();
                break;
            }

            case sf::Socket::Disconnected: {
                std::cerr << "Lost connection with server\n";
                window.close();
                break;
            }

            case sf::Socket::Error: {
                std::cerr << "Internal error on socket...\n";
                break;
            }

            default: {
                break;
            }
        }

        window.clear(sf::Color::Black);
        window.draw(player_sprite);
        window.draw(ball_sprite);
        window.display();
    }
}