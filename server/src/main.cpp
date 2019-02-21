#include <SFML/Network.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>

#include <multipong/Game.hpp>

std::mutex cout_mutex;

void client_runner(std::mutex& clients_mutex, std::vector<std::unique_ptr<sf::TcpSocket>>& clients, std::atomic_bool& stop) {
    sf::Clock clock;
    sf::Clock c;

    sf::Vector2f const boundaries{ 800, 600 };

    float const ball_radius{ 8 };
    sf::Vector2f const ball_boundaries{ boundaries.x - ball_radius, boundaries.y - ball_radius };

    float const pad_height{ 80 };
    float const pad_boundary{ boundaries.y - pad_height };

    pong::Ball ball {
        ball_boundaries / 2.f, // position
        { 300, 300 }  // speed
    };

    float const pad_max_speed{ 200 };
    auto input = pong::Input::Idle;
    pong::Pad pad {
        pad_boundary / 2.f, // position
        0 // speed
    };

    float refresh_rate = 0.4;
    sf::Packet packet;

    while(!stop) {/*
        std::lock_guard lk{ clients_mutex };
        sf::SocketSelector selector;
        {
            unsigned id{ 0 };
            for(auto& client : clients) {
                if (client->getRemoteAddress() == sf::IpAddress::None) {
                    std::lock_guard cout_lk{ cout_mutex };
                    std::cout << "a client doesn't have a connection " << id << '\n';
                } else {
                    //selector.add(*client);
                    sf::Packet packet;
                    client->setBlocking(false);
                    switch (client->receive(packet)) {
                        case sf::Socket::Done: {
                            std::lock_guard cout_lk{ cout_mutex };
                            std::cout << "receive something from " << id << '\n';
                            break;
                        }

                        case sf::Socket::Disconnected: {
                            std::lock_guard cout_lk{ cout_mutex };
                            std::cout << "disconnection of " << id << '\n';
                            break;
                        }

                        default: {

                        }
                    }
                    client->setBlocking(true);
                }

                ++id;
            }
        }*/
/*
        if (selector.wait()) {
            unsigned id{ 0 };
            for(auto& client : clients) {
                if (selector.isReady(*client)) {
                    sf::Packet packet;
                    switch (client->receive(packet)) {
                        case sf::Socket::Done: {
                            std::lock_guard cout_lk{ cout_mutex };
                            std::cout << "receive something from " << id << '\n';
                            break;
                        }

                        case sf::Socket::Disconnected: {
                            std::lock_guard cout_lk{ cout_mutex };
                            std::cout << "disconnection of " << id << '\n';
                            break;
                        }

                        default: {}
                    }
                }

                ++id;
            }
        }*/

        auto dt = clock.restart().asSeconds();
        static float d{ 0 };
        d += dt;

        if (d > 5) input = pong::Input::Up;
        if (d > 6) input = pong::Input::Idle;
        if (d > 7) input = pong::Input::Down;
        if (d > 9) input = pong::Input::Idle;
        if (d > 10) input = pong::Input::Up;
        if (d > 11) { input = pong::Input::Idle; d = 2; }


        ball.update(dt, ball_boundaries);
        pad.update(dt, input, pad_max_speed, pad_boundary);

        if (c.getElapsedTime().asSeconds() < refresh_rate) { continue; }
        c.restart();
        
        {
            std::lock_guard cout_lk{ cout_mutex };
            std::cout << "UPDATE\n";
        }

        if (packet.getDataSize() == 0) {
            packet << ball << pad;
        }

        std::lock_guard lk{ clients_mutex };

        std::vector<std::size_t> disconnected;

        std::size_t id{ 0 };
        for(auto& client : clients) {
            switch (client->send(packet)) {
                case sf::Socket::Disconnected: {
                    disconnected.push_back(id);
                    std::lock_guard cout_lk{ cout_mutex };
                    std::cout << "disconnection of the client\n";
                    break;
                }

                case sf::Socket::NotReady:
                case sf::Socket::Error: {
                    std::lock_guard cout_lk{ cout_mutex };
                    std::cout << "Internal error\n";
                    break;
                }

                case sf::Socket::Done: {
                    packet.clear();
                    break;
                }

                default: {}
            }

            ++id;
        }

        for(auto it = std::rbegin(disconnected); it != std::rend(disconnected); ++it) {
            clients.erase(std::begin(clients) + *it);
        }

    }
}

int main() {
    sf::TcpListener listener;
    listener.listen(48621);

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
        
        std::lock_guard lk{ clients_mutex };
        clients.emplace_back(std::move(client));
    }

    stop_thread = true;

}