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
#include <deque>

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

struct SendPacket {
    sf::Packet packet;
    std::function<void()> on_done;
};

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

    pong::packet::GameState game_state;
    sf::Clock clock;

    bool up_pressed{ false };
    bool down_pressed{ false };
    bool input_updated{ true };
    sf::Packet input_packet;
    
    bool has_send_username{ false };

    std::deque<SendPacket> packets_to_send;
    sf::Packet packet;
    std::unique_ptr<sf::TcpSocket> socket;
    std::future<std::unique_ptr<sf::TcpSocket>> future_socket;

    std::vector<sf::Text> users_connected;
    std::vector<sf::Text> rooms_online;

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

                    if (state == pong::State::ValidUser) {
                        if (event.key.code == sf::Keyboard::R) {
                            sf::Packet create_room_packet;
                            create_room_packet << pong::packet::CreateRoom{};
                            packets_to_send.push_back({
                                create_room_packet,
                                [&state] () {
                                    state = pong::State::Spectator;
                                }
                            });
                        }
                    }

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
                    auto socket_ptr = std::make_unique<sf::TcpSocket>();
                    if (socket_ptr->connect("127.0.0.1", port) != sf::Socket::Done) {
                        return nullptr;
                    }
                    socket_ptr->setBlocking(false);
                    return std::move(socket_ptr);
                }, options.port ? *options.port : 48622);
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
                    has_send_username = false;

                    sf::Packet change_username_packet;
                    change_username_packet << pong::packet::ChangeUsername{ *options.username };
                    packets_to_send.push_back({
                        change_username_packet,
                        [&has_send_username] () {
                            has_send_username = true;
                        } 
                    });
                }
                break;
            }
            case pong::State::InvalidUser: {
                if (has_send_username) {
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

                                for(auto const& user : response.users) {
                                    auto& tmp_text = users_connected.emplace_back(user, font, 15);
                                    tmp_text.setPosition(20, 20 + (users_connected.size() - 1) * (tmp_text.getCharacterSize() + 5));
                                }

                                for(auto const& room : response.rooms) {
                                    auto& tmp_text = rooms_online.emplace_back("#" + std::to_string(room), font, 15);
                                    tmp_text.setPosition(boundaries.x - 20 - tmp_text.getLocalBounds().width, 20 + (rooms_online.size() - 1) * (tmp_text.getCharacterSize() + 5));
                                }

                                state = pong::State::ValidUser;
                                std::cout << "All good !\n";
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
                switch(socket->receive(packet)) {
                    case sf::Socket::Done: {
                        pong::packet::PacketID packet_id; 
                        packet >> packet_id;
                        if (packet_id == pong::packet::PacketID::NewUser) {
                            pong::packet::NewUser new_user;
                            packet >> new_user;
                            
                            auto& tmp_text = users_connected.emplace_back(new_user.username, font, 15);
                            tmp_text.setPosition(20, 20 + (users_connected.size() - 1) * (tmp_text.getCharacterSize() + 5));
                        } else if (packet_id == pong::packet::PacketID::OldUser) {
                            pong::packet::OldUser old_user;
                            packet >> old_user;

                            sf::String old_username{ old_user.username };
                            for(auto it = std::begin(users_connected); it != std::end(users_connected); ++it) {
                                if (it->getString() == old_username) {
                                    auto to_delete = it;
                                    for(; it != std::end(users_connected); ++it) {
                                        it->setPosition(20, it->getPosition().y - (it->getCharacterSize() + 5));
                                    }

                                    users_connected.erase(to_delete);

                                    break;
                                }
                            }
                        } else if (packet_id == pong::packet::PacketID::NewRoom) {
                            pong::packet::NewRoom new_room;
                            packet >> new_room;

                            auto& tmp_text = rooms_online.emplace_back("#" + std::to_string(new_room.id), font, 15);
                            tmp_text.setPosition(boundaries.x - 20 - tmp_text.getLocalBounds().width, 20 + (rooms_online.size() - 1) * (tmp_text.getCharacterSize() + 5));
                        } else {
                            std::cerr << "Warning: Receive packet #" << static_cast<int>(packet_id) << ", expected UsernameResponse #" << static_cast<int>(pong::packet::PacketID::NewUser);
                            std::cerr << " or OldUser #" << static_cast<int>(pong::packet::PacketID::OldUser) << " or NewRoom #" << static_cast<int>(pong::packet::PacketID::NewRoom) << '\n';
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
                break;
            }

            case pong::State::Spectator: {
                switch(socket->receive(packet)) {
                    case sf::Socket::Done: {
                        pong::packet::PacketID packet_id; 
                        packet >> packet_id;
                        if (packet_id == pong::packet::PacketID::GameState) {
                            packet >> game_state;
                            ball_sprite.setPosition(game_state.ball.position);
                            player_sprite.setPosition(player_sprite.getPosition().x, game_state.left.y);
                            opponent_sprite.setPosition(opponent_sprite.getPosition().x, game_state.right.y);

                        } else {
                            std::cerr << "Warning: Receive packet #" << static_cast<int>(packet_id) << ", expected GameState #" << static_cast<int>(pong::packet::PacketID::GameState) << '\n';
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
                break;
            }

            default:{
                window.close();
                std::cerr << "Unexpected state... Abording...\n";
                error_code = 1;
                break;
            }
        }

        if (!packets_to_send.empty()) {
            std::cout << packets_to_send.size() << " packets remaining to send\n";
            auto& p = packets_to_send.front();
            switch(socket->send(p.packet)) {
                case sf::Socket::Done: {
                    p.on_done();
                    packets_to_send.pop_front();
                    std::cout << "packet successfully sent\n";
                    break;
                }
                case sf::Socket::Disconnected: {
                    state = pong::State::Offline;
                    packets_to_send.clear();
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
        }

        text.setString(state_to_string(state));
        text.setPosition((boundaries.x - text.getLocalBounds().width) / 2, (boundaries.y - text.getLocalBounds().height) / 2);
        window.clear(sf::Color::Black);
        if (state == pong::State::Spectator) {
            window.draw(ball_sprite);
            window.draw(player_sprite);
            window.draw(opponent_sprite);
        }
        window.draw(text);
        for(auto& user : users_connected) {
            window.draw(user);
        }
        for(auto& room : rooms_online) {
            window.draw(room);
        }
        window.display();
    }

    return error_code;
}