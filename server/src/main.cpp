#include <SFML/Network.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>

#include <multipong/Game.hpp>

std::mutex cout_mutex;

float        const ball_radius    { 8 };
float        const pad_height     { 80 };
float        const pad_padding    { 12 };
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
        , ball{ ball_boundaries / 2.f, { 300, 300 }}
        , pad_left{ pad_boundary / 2.f, 0 }
        , pad_right{ pad_boundary / 2.f, 0 }
    {}

    void update() {
        float dt = clock.restart().asSeconds();
        ball.update(dt, boundaries);
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

void client_runner(std::mutex& clients_mutex, std::vector<std::unique_ptr<sf::TcpSocket>>& clients, std::atomic_bool& stop) {
    sf::Clock refresh_clock;
    float const refresh_rate = 5;
    bool force_refresh{ false };

    GameRoom room;

    while(!stop) {
        {
            std::lock_guard lk{ clients_mutex };

            for(auto& client : clients) {
                if (!room.player_left) {
                    room.player_left = std::move(client);
                    room.packets.push_back(PeriodicPacket { room.player_left.get(), [game = &room.game] () {
                        sf::Packet packet;
                        packet << game->ball << game->pad_left << game->pad_right;
                        return packet;
                    }});
                }
                else if (!room.player_right) {
                    room.player_right = std::move(client);
                    room.packets.push_back(PeriodicPacket { room.player_right.get(), [game = &room.game] () {
                        sf::Packet packet;
                        packet << game->ball << game->pad_left << game->pad_right;
                        return packet;
                    }});
                }
                else {
                    room.spectators.emplace_back(std::move(client));
                    room.packets.push_back(PeriodicPacket { room.spectators.back().get(), [game = &room.game] () {
                        sf::Packet packet;
                        packet << game->ball << game->pad_left << game->pad_right;
                        return packet;
                    }});
                }
            }

            clients.clear();
        }

        if (room.player_left && room.player_right) {
            if (receive_input(*room.player_left, room.game.input_left, force_refresh) == sf::Socket::Disconnected) {
                std::cout << "disconnection of the left player\n";

                for(std::size_t i{ 0 }; i < room.packets.size(); ++i) {
                    auto& packet = room.packets[i];
                    if (packet.socket == room.player_left.get()) {
                        room.packets.erase(std::begin(room.packets) + i);
                        break;
                    }
                }

                room.player_left = nullptr;
            }

            if (receive_input(*room.player_right, room.game.input_right, force_refresh) == sf::Socket::Disconnected) {
                std::cout << "disconnection of the right player\n";

                for(std::size_t i{ 0 }; i < room.packets.size(); ++i) {
                    auto& packet = room.packets[i];
                    if (packet.socket == room.player_right.get()) {
                        room.packets.erase(std::begin(room.packets) + i);
                        break;
                    }
                }

                room.player_right = nullptr;
            }

            room.game.update();
        } else {
            room.game.clock.restart();
        }

        if (!force_refresh && refresh_clock.getElapsedTime().asSeconds() < refresh_rate) { continue; }
        refresh_clock.restart();

        force_refresh = false;

        for(auto& packet : room.packets) {
            if (packet.send() == sf::Socket::Disconnected) {
                std::lock_guard cout_lk{ cout_mutex };
                std::cout << "disconnection of the client : " << packet.socket << '\n';
            }
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
        client->setBlocking(false);
        
        std::lock_guard lk{ clients_mutex };
        clients.emplace_back(std::move(client));
    }

    stop_thread = true;

}