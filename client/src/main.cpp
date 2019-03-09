#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <optional>
#include <future>
#include <chrono>

#include <multipong/Game.hpp>
#include <multipong/Packets.hpp>

pong::Input get_input(bool up, bool down) {
    return up == down ? pong::Input::Idle :
           up ? pong::Input::Up :
                pong::Input::Down;
}

std::string state_to_string(pong::State state) {
    switch (state) {
        case pong::State::Offline:      return "You are currently offline";
        case pong::State::Connecting:   return "Connecting to server...";
        case pong::State::InvalidUser:  return "Loging in...";
        case pong::State::ValidUser:    return "...";
        case pong::State::Spectator:    return "Watching as a spectator";
        case pong::State::Player:       return "Playing";
    }

    return "???";
} 

struct Options {
    std::optional<std::string> username;
    std::optional<int> port;
};

enum class OptionState {
    Username,
    Port,
    State
};

Options parse_arguments(int argc, char** argv) {
    Options options;
    auto state = OptionState::State;

    for(int i{ 1 }; i < argc; ++i) {
        std::string arg{ argv[i] };
        switch(state) {
            case OptionState::State: {
                if (arg == "-p" || arg == "--port") {
                    state = OptionState::Port;
                } 
                else if (arg == "-u" || arg == "--username") {
                    state = OptionState::Username;
                } else {
                    std::cout << "Unknown command\n";
                    std::cout << "USAGE: ./client [(-p | --port) <port_number>] (-u | --username) <username>\n";
                    std::exit(1);
                }
                break;
            }
            case OptionState::Username: {
                options.username = arg;
                state = OptionState::State;
                break;
            }
            case OptionState::Port: {
                try {
                    options.port = std::stoi(arg);
                } catch(...) {
                    std::cout << "Unknown arguments\n";
                    std::cout << "USAGE: ./client [(-p | --port) <port_number>] (-u | --username) <username>\n";
                    std::exit(1);
                }
                state = OptionState::State;
                break;
            }
        }
    }

    if (state != OptionState::State) {
        std::cout << "Unknown command\n";
        std::cout << "USAGE: ./client [(-p | --port) <port_number>] (-u | --username) <username>\n";
        std::exit(1);
    }

    if (!options.username || options.username->empty()) {
        std::cout << "You must enter your username\n";
        std::cout << "USAGE: ./client [(-p | --port) <port_number>] (-u | --username) <username>\n";
        std::exit(1);
    }

    return options;

}

template<typename T>
bool is_future_ready(std::future<T> const& f) { 
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

int main(int argc, char** argv) {
    int error_code{ 0 };

    auto options = parse_arguments(argc, argv);

    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Multiplayer pong");

    auto state{ pong::State::Offline };

    float        const ball_radius    { 8 };
    float        const pad_height     { 80 };
    float        const pad_width      { 12 };
    float        const pad_padding    { 40 };
    float        const pad_max_speed  { 200 };
    sf::Vector2f const boundaries     { 800, 600 };
    sf::Vector2f const ball_boundaries{ boundaries.x - ball_radius, boundaries.y - ball_radius };
    float        const pad_boundary   { boundaries.y - pad_height };

    sf::RectangleShape player_sprite({ pad_width, pad_height });
    sf::RectangleShape opponent_sprite({ pad_width, pad_height });
    sf::RectangleShape ball_sprite({ ball_radius, ball_radius });

    sf::Font font;
    if (!font.loadFromFile("../assets/neoletters.ttf")) {
        std::cout << "Couldn't load font '../assets/neoletters.ttf'\n";
        std::exit(1);
    }

    sf::Text text("", font, 32);


    ball_sprite.setPosition(ball_boundaries / 2.f);
    player_sprite.setPosition({ pad_padding - pad_width, pad_boundary / 2.f });
    opponent_sprite.setPosition({ boundaries.x - pad_padding, pad_boundary / 2.f });

    pong::Ball ball;
    pong::Pad pad_left;
    pong::Pad pad_right;

    sf::Clock clock;

    bool up_pressed{ false };
    bool down_pressed{ false };
    bool input_updated{ true };
    sf::Packet input_packet;
    
    std::unique_ptr<sf::TcpSocket> socket;
    sf::Packet packet;
    std::future<std::unique_ptr<sf::TcpSocket>> future_socket;
    

    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            switch(event.type) {
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

        switch(state) {
            case pong::State::Offline: {
                socket = nullptr;
                state = pong::State::Connecting;
                future_socket = std::async(std::launch::async, [] (int port) -> std::unique_ptr<sf::TcpSocket> {
                    auto socket = std::make_unique<sf::TcpSocket>();
                    if (socket->connect("127.0.0.1", port) != sf::Socket::Done) {
                        return nullptr;
                    }
                    socket->setBlocking(false);
                    return std::move(socket);
                }, options.port ? *options.port : 48621);
                break;
            }
            case pong::State::Connecting: {
                if (is_future_ready(future_socket)) {
                    std::unique_ptr<sf::TcpSocket> res = future_socket.get();
                    if (!res) {
                        window.close();
                        std::cerr << "Couldn't connect to server... Abording...\n";
                        error_code = 1;
                    }

                    socket = std::move(res);
                    state = pong::State::InvalidUser;

                    packet.clear();
                    packet << pong::packet::ChangeUsername{ *options.username };
                }
                break;
            }
            case pong::State::InvalidUser: {
                static bool has_send_username{ false };
                if (!has_send_username) {
                    switch(socket->send(input_packet)) {
                        case sf::Socket::Done: {
                            input_packet.clear();
                            has_send_username = true;
                            break;
                        }
                        case sf::Socket::Disconnected: {
                            state = pong::State::Offline;
                            break;
                        }

                        case sf::Socket::Error: {
                            window.close();
                            std::cerr << "Internal error on socket when sending username...\n";
                            error_code = 1;
                            break;
                        }

                        default: {
                            break;
                        }
                    }
                } else {
                    switch(socket->receive(packet)) {
                        case sf::Socket::Done: {
                            pong::packet::PacketID packet_id; 
                            packet >> packet_id;
                            if (packet_id == pong::packet::PacketID::UsernameResponse) {
                                pong::packet::UsernameResponse response;
                                packet >> response;
                                if (response.result != pong::packet::UsernameResponse::Okay) {
                                    window.close();
                                    std::cerr << "Invalid Username...\n";
                                    error_code = 1;
                                }

                                state = pong::State::ValidUser;
                            } else {
                                std::cerr << "Warning: Receive packet #" << static_cast<int>(packet_id) << ", expected UsernameResponse #" << static_cast<int>(pong::packet::PacketID::UsernameResponse) << '\n';
                            }
                            break;
                        }

                        case sf::Socket::Disconnected: {
                            state = pong::State::Offline;
                            break;
                        }

                        case sf::Socket::Error: {
                            window.close();
                            std::cerr << "Internal error on socket when receiving username response...\n";
                            error_code = 1;
                            break;
                        }

                        default: {
                            break;
                        }
                    }
                }

                break;
            }

            case pong::State::ValidUser: {
                window.close();
                std::cout << "All good !\n";
                break;
            }

            default:{
                window.close();
                std::cerr << "Unexpected state... Abording...\n";
                error_code = 1;
                break;
            }
        }

        text.setString(state_to_string(state));
        text.setPosition((boundaries.x + text.getLocalBounds().width) / 2, (boundaries.y + text.getLocalBounds().height) / 2);

/*
        if (input_updated) {

            if (input_packet.getDataSize() == 0) {
                input_packet << get_input(up_pressed, down_pressed);
            }

            switch(socket->send(input_packet)) {
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
*/
/*
        auto dt = clock.restart().asSeconds();

        ball.update(dt, pad_left.y, pad_right.y, boundaries, pad_padding, pad_height, pad_width, ball_radius);
        pad_left.update(dt, pad_boundary);
        pad_right.update(dt, pad_boundary);

        ball_sprite.setPosition(ball.position);
        player_sprite.setPosition(player_sprite.getPosition().x, pad_left.y);
        opponent_sprite.setPosition(opponent_sprite.getPosition().x, pad_right.y);

        switch(socket->receive(packet)) {
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
*/
        window.clear(sf::Color::Black);
//        window.draw(player_sprite);
//        window.draw(opponent_sprite);
//        window.draw(ball_sprite);
        window.draw(text);
        window.display();
    }

    return error_code;
}