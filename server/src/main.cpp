#include <SFML/Network.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>
#include <algorithm>

#include <multipong/Game.hpp>
#include <multipong/Packets.hpp>

std::mutex cout_mutex;

float        const ball_radius    { 8 };
float        const pad_height     { 80 };
float        const pad_width      { 12 };
float        const pad_padding    { 40 };
float        const pad_max_speed  { 200 };
sf::Vector2f const boundaries     { 800, 600 };
sf::Vector2f const ball_boundaries{ boundaries.x - ball_radius, boundaries.y - ball_radius };
float        const pad_boundary   { boundaries.y - pad_height };

struct Game {
    pong::Input input_left;
    pong::Input input_right;

    pong::Ball ball;
    pong::Pad pad_left;
    pong::Pad pad_right;

    sf::Clock clock;

    Game() 
        : input_left{ pong::Input::Idle }
        , input_right{ pong::Input::Idle }
        , ball{ ball_boundaries / 2.f, { 100, 100 }}
        , pad_left{ pad_boundary / 2.f, 0 }
        , pad_right{ pad_boundary / 2.f, 0 }
    {}

    void update() {
        float dt = clock.restart().asSeconds();
        ball.update(dt, pad_left.y, pad_right.y, boundaries, pad_padding, pad_height, pad_width, ball_radius);
        pad_left.update(dt, input_left, pad_max_speed, pad_boundary);
        pad_right.update(dt, input_right, pad_max_speed, pad_boundary);
    }
};

struct PeriodicPacket {
    sf::TcpSocket* socket;
    std::function<sf::Packet()> packet_maker;
    sf::Packet packet;

    sf::Socket::Status send() {
        if (packet.getDataSize() == 0) {
            packet = packet_maker();
        }

        auto status = socket->send(packet);

        if (status == sf::Socket::Done) {
            packet.clear();
        }

        return status;
    }
};

struct GameRoom {
    Game game;

    std::vector<std::unique_ptr<sf::TcpSocket>> spectators;

    std::unique_ptr<sf::TcpSocket> player_left;
    std::unique_ptr<sf::TcpSocket> player_right;

    std::vector<PeriodicPacket> packets;
};

sf::Socket::Status receive_input(sf::TcpSocket& player, pong::Input& res, bool& got_input) {
    sf::Packet packet;
    auto status = player.receive(packet);

    if (status == sf::Socket::Done) {
        got_input = true;
        packet >> res;
    }

    return status;
}

pong::packet::UsernameResponse::Result is_username_valid(std::string const& username) {
    if (username.size() < 3) {
        std::cout << '"' << username << '"' << " is too short\n";
        return pong::packet::UsernameResponse::TooShort;
    }

    if (username.size() > 20) {
        std::cout << '"' << username << '"' << " is too long\n";
        return pong::packet::UsernameResponse::TooLong;
    }

    if (!std::all_of(std::begin(username), std::end(username), [] (auto c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    })) {
        std::cout << '"' << username << '"' << " is not valid\n";
        return pong::packet::UsernameResponse::InvalidCharacters;
    }

    return pong::packet::UsernameResponse::Okay;

}

struct User {
    std::string name;
    std::unique_ptr<sf::TcpSocket> socket;
};

void client_runner(std::mutex& clients_mutex, std::vector<std::unique_ptr<sf::TcpSocket>>& clients, std::atomic_bool& stop) {
    sf::Clock refresh_clock;
    float const refresh_rate = 0.05;
    bool force_refresh{ false };

    std::vector<std::unique_ptr<sf::TcpSocket>> invalid_users;
    std::vector<User> valid_user;
    std::vector<std::pair<sf::Packet, sf::TcpSocket*>> partial_packets;

    while(!stop) {
        {
            std::lock_guard lk{ clients_mutex };

            for(auto& client : clients) {
                invalid_users.emplace_back(std::move(client));
            }

            clients.clear();
        }

        {
            std::size_t end{ invalid_users.size() };
            for(std::size_t i{ 0 }; i < end; ++i) {
                auto& user{ invalid_users[i] };
                auto& socket = *user;

                sf::Packet packet;
                switch(socket.receive(packet)) {
                    case sf::Socket::NotReady: {
                        break;
                    }

                    case sf::Socket::Done: {
                        pong::packet::PacketID packet_id; 
                        packet >> packet_id;
                        if (packet_id == pong::packet::PacketID::ChangeUsername) {
                            pong::packet::ChangeUsername request;
                            packet >> request;
                            auto valid = is_username_valid(request.username);
                            if (valid == pong::packet::UsernameResponse::Okay) {
                                valid_user.emplace_back(User{ request.username, std::move(user) });
                                if (i != end - 1) {
                                    std::iter_swap(std::begin(partial_packets) + i, std::begin(partial_packets) + (end - 1));
                                }
                                --i;
                                --end;

                                std::cout << request.username << " got connected\n";
                            }

                            sf::Packet response;
                            std::vector<std::string> users;
                            for(auto const& u : valid_user) {
                                users.emplace_back(u.name);
                            }
                            response << pong::packet::UsernameResponse {
                                valid, users, {0, 1, 2, 42}
                            };

                            partial_packets.push_back({ response, &socket });
                        } else {
                            std::cerr << "Warning: Receive packet #" << static_cast<int>(packet_id) << ", expected ChangeUsername #" << static_cast<int>(pong::packet::PacketID::ChangeUsername) << '\n';
                        }
                        break;
                    }

                    default: {
                        std::cout << "Remove a user\n";
                        if (i != end - 1) {
                            std::iter_swap(std::begin(partial_packets) + i, std::begin(partial_packets) + (end - 1));
                        }
                        --i;
                        --end;
                        break;
                    }
                }
            }

            invalid_users.erase(std::begin(invalid_users) + end, std::end(invalid_users));
        }

        if (!partial_packets.empty()) {
            std::size_t end{ partial_packets.size() };

            std::cout << end << " packets to send...\n";

            for(std::size_t i{ 0 }; i < end; ++i) {
                auto& pair{ partial_packets[i] };

                auto& socket = *pair.second;
                auto& packet = pair.first; 

                switch(socket.send(packet)) {
                    case sf::Socket::Partial: {
                        break;
                    }

                    default: {
                        if (i != end - 1) {
                            std::iter_swap(std::begin(partial_packets) + i, std::begin(partial_packets) + (end - 1));
                        }
                        --i;
                        --end;
                        break;
                    }
                }
            }

            partial_packets.erase(std::begin(partial_packets) + end, std::end(partial_packets));

            std::cout << partial_packets.size() << " packets partially sent\n";
        }
    }
}

int main() {
    sf::TcpListener listener;
    listener.listen(48622);

    std::mutex clients_mutex;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;
    std::atomic_bool stop_thread{ false };

    std::thread client_thread(client_runner, std::ref(clients_mutex), std::ref(clients), std::ref(stop_thread));

    while(true) {
        auto client = std::make_unique<sf::TcpSocket>();
        if (listener.accept(*client) != sf::Socket::Done) {
            std::cerr << "Error\n";
            continue;
        }

        std::cout << "New client: " << client->getRemoteAddress() << ":" << client->getRemotePort() << std::endl;
        client->setBlocking(false);
        
        std::lock_guard lk{ clients_mutex };
        clients.emplace_back(std::move(client));
    }

    stop_thread = true;

}