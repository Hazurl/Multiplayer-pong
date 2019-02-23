#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

#include <multipong/Game.hpp>

pong::Input get_input(bool up, bool down) {
    return up == down ? pong::Input::Idle :
           up ? pong::Input::Up :
                pong::Input::Down;
}

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
    sf::RectangleShape opponent_sprite({ 12, 80 });
    sf::RectangleShape ball_sprite({ 8, 8 });

    ball_sprite.setPosition({ 400, 300 });
    player_sprite.setPosition({ 12, 300 });
    opponent_sprite.setPosition({ 800-12-12, 300 });

    pong::Ball ball;
    pong::Pad pad_left;
    pong::Pad pad_right;

    sf::Vector2f const boundaries{ 792, 592 };

    sf::Clock clock;

    bool up_pressed{ false };
    bool down_pressed{ false };
    bool input_updated{ true };
    sf::Packet input_packet;
    
    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            switch( event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                } 

                case sf::Event::KeyPressed: {
                    input_updated = true;

                    if (event.key.code == sf::Keyboard::Up && !up_pressed) up_pressed = true;
                    else if (event.key.code == sf::Keyboard::Down && !down_pressed) down_pressed = true;
                    else input_updated = false;

                    break;
                }

                case sf::Event::KeyReleased: {
                    input_updated = true;

                    if (event.key.code == sf::Keyboard::Up && up_pressed) up_pressed = false;
                    else if (event.key.code == sf::Keyboard::Down && down_pressed) down_pressed = false;
                    else input_updated = false;

                    break;
                }

                default: { break; }
            }
        }

        if (input_updated) {

            if (input_packet.getDataSize() == 0) {
                input_packet << get_input(up_pressed, down_pressed);
            }

            switch(socket.send(input_packet)) {
                case sf::Socket::Done: {
                    input_packet.clear();
                    input_updated = false;
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
        }

        auto dt = clock.restart().asSeconds();

        ball.update(dt, boundaries);
        pad_left.update(dt, boundaries.y);
        pad_right.update(dt, boundaries.y);

        ball_sprite.setPosition(ball.position);
        player_sprite.setPosition(player_sprite.getPosition().x, pad_left.y);
        opponent_sprite.setPosition(opponent_sprite.getPosition().x, pad_right.y);

        switch(socket.receive(packet)) {
            case sf::Socket::Done: {
                packet >> ball >> pad_left >> pad_right;
                
                ball_sprite.setPosition(ball.position);
                player_sprite.setPosition(player_sprite.getPosition().x, pad_left.y);
                opponent_sprite.setPosition(opponent_sprite.getPosition().x, pad_right.y);

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
        window.draw(opponent_sprite);
        window.draw(ball_sprite);
        window.display();
    }
}