#include <SFML/Network.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>
#include <algorithm>
#include <variant>
#include <unordered_set>

#include <multipong/Game.hpp>
#include <multipong/Packets.hpp>

std::mutex cout_mutex;

float        const ball_radius    { 8 };
float        const pad_height     { 80 };
float        const pad_width      { 12 };
float        const pad_padding    { 40 };
float        const pad_max_speed  { 200 };
float        const ball_max_speed { 100 };
sf::Vector2f const boundaries     { 800, 600 };
sf::Vector2f const ball_boundaries{ boundaries.x - ball_radius, boundaries.y - ball_radius };
float        const pad_boundary   { boundaries.y - pad_height };

struct Game {
    pong::Input input_left{ pong::Input::Idle };
    pong::Input input_right{ pong::Input::Idle };

    pong::Ball ball{ ball_boundaries / 2.f, { ball_max_speed, ball_max_speed }};
    pong::Pad pad_left{ pad_boundary / 2.f, 0 };
    pong::Pad pad_right{ pad_boundary / 2.f, 0 };

    sf::Clock clock;
    bool force_refresh{ false };

    void update() {
        float dt = clock.restart().asSeconds();
        ball.update(dt, pad_left.y, pad_right.y, boundaries, pad_padding, pad_height, pad_width, ball_radius);
        pad_left.update(dt, input_left, pad_max_speed, pad_boundary);
        pad_right.update(dt, input_right, pad_max_speed, pad_boundary);
    }
};

struct User {
    std::string name;
    std::unique_ptr<sf::TcpSocket> socket;
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
            std::cout << "Packet sent\n"; 
        }

        return status;
    }
};

struct GameRoom {
    Game game;

    std::vector<User> spectators;

    User player_left;
    User player_right;

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

template<typename T, typename F>
void for_each(std::vector<T>& values, F&& func) {
    std::size_t end{ values.size() };
    for(std::size_t i{ 0 }; i < end; ++i) {
        auto& value = values[i];
        auto res = func(value);

        if (!res) {
            if (i != end - 1) {
                std::iter_swap(std::begin(values) + i, std::begin(values) + (end - 1));
            }
            --i;
            --end;
        }
    }

    values.erase(std::begin(values) + end, std::end(values));
}

struct SocketError {};
struct SocketNotReady {};

using receive_code_t = std::variant<SocketNotReady, SocketError, pong::packet::PacketID>;

bool has_id(receive_code_t const& r) {
    return std::holds_alternative<pong::packet::PacketID>(r);
}

pong::packet::PacketID get_id(receive_code_t const& r) {
    return std::get<pong::packet::PacketID>(r);
}

pong::packet::PacketID const* get_id_if(receive_code_t const& r) {
    return std::get_if<pong::packet::PacketID>(&r);
}

bool is_error(receive_code_t const& r) {
    return std::holds_alternative<SocketError>(r);
}

receive_code_t receive_id(sf::TcpSocket& socket, sf::Packet& packet) {
    switch(socket.receive(packet)) {
        case sf::Socket::NotReady: {
            return SocketNotReady{};
        }

        case sf::Socket::Done: {
            pong::packet::PacketID packet_id; 
            packet >> packet_id;
            return packet_id;
        }

        default: {
            return SocketError{};
        }
    }
}

template<typename...Ts>
sf::Packet to_packet(Ts&&... ts) {
    sf::Packet p;
    (p << ... << ts);
    return p;
}

template<typename T>
T from_packet(sf::Packet& p) {
    T t;
    p >> t;
    return t;
}

void broadcast(std::vector<std::pair<sf::Packet, sf::TcpSocket*>>& partial_packets, std::vector<User>& users, sf::Packet const& p) {
    for(auto& user : users) {
        partial_packets.push_back({ p, user.socket.get() });
    }
}

void broadcast_except(std::vector<std::pair<sf::Packet, sf::TcpSocket*>>& partial_packets, std::vector<User>& users, User const& exception, sf::Packet const& p) {
    for(auto& user : users) {
        if (&user != &exception) {
            partial_packets.push_back({ p, user.socket.get() });
        }
    }
}

#define extract_(attr, vec) extract<decltype(vec)::value_type, decltype(decltype(vec)::value_type::attr), &decltype(vec)::value_type::attr>(vec) 

template<typename T, typename A, A T::*a>
std::vector<A> extract(std::vector<T> const& ts) {
    std::vector<A> as;
    for(auto const& t : ts) {
        as.emplace_back(t.*a);
    }

    return as;
}

void client_runner(std::mutex& clients_mutex, std::vector<std::unique_ptr<sf::TcpSocket>>& clients, std::atomic_bool& stop) {
    sf::Clock refresh_clock;
    float const refresh_rate = 0.1;

    std::vector<std::unique_ptr<sf::TcpSocket>> invalid_users;
    std::vector<User> valid_users;
    std::vector<std::pair<sf::Packet, sf::TcpSocket*>> partial_packets;

    std::vector<std::unique_ptr<GameRoom>> rooms;

    while(!stop) {
        {
            std::lock_guard lk{ clients_mutex };

            for(auto& client : clients) {
                invalid_users.emplace_back(std::move(client));
            }

            clients.clear();
        }

        for_each(invalid_users, [&] (std::unique_ptr<sf::TcpSocket>& socket_ptr) {
            auto& socket = *socket_ptr;

            sf::Packet packet;
            auto received_id = receive_id(socket, packet);

            if (auto* id = get_id_if(received_id)) {
                switch(*id) {
                    case pong::packet::PacketID::ChangeUsername: {
                        auto request = from_packet<pong::packet::ChangeUsername>(packet);
                        auto valid = is_username_valid(request.username);

                        if (valid == pong::packet::UsernameResponse::Okay) {
                            auto new_user_message = to_packet(pong::packet::NewUser {
                                request.username
                            });

                            broadcast(partial_packets, valid_users, new_user_message);
                            valid_users.emplace_back(User{ request.username, std::move(socket_ptr) });

                            std::cout << request.username << " got connected\n";
                        }

                        auto users = extract_(name, valid_users);

                        std::vector<int> rooms_id;
                        for(std::size_t room_id{ 0 }; room_id < rooms.size(); ++room_id) {
                            if (rooms[room_id] != nullptr) {
                                rooms_id.emplace_back(static_cast<int>(room_id));
                            }
                        }

                        auto response = to_packet(pong::packet::UsernameResponse{
                            valid, users, rooms_id
                        });

                        partial_packets.push_back({ response, &socket });
                        return valid != pong::packet::UsernameResponse::Okay;
                    }

                    default: {
                        std::cerr << "Warning: Receive packet #" << static_cast<int>(*id) << ", expected one of:" << '\n';
                        std::cerr << "\tChangeUsername #" << static_cast<int>(pong::packet::PacketID::ChangeUsername) << '\n';
                    }
                }
            }

            return is_error(received_id);
        });


        for_each(valid_users, [&] (User& user) {
            auto& socket = *user.socket;

            sf::Packet packet;
            auto received_id = receive_id(socket, packet);

            if (auto* id = get_id_if(received_id)) {
                switch(*id) {
                    case pong::packet::PacketID::CreateRoom: {
                        std::cout << "Create a new room!\n";
                        std::size_t room_id{ 0 };
                        for(; room_id < rooms.size(); ++room_id) {
                            if (rooms[room_id] == nullptr) {
                                break;
                            }
                        }

                        if (room_id >= rooms.size()) {
                            rooms.emplace_back(std::make_unique<GameRoom>());
                        } else {
                            rooms[room_id] = std::make_unique<GameRoom>();
                        }

                        rooms[room_id]->packets.push_back({ user.socket.get(), [game = &rooms[room_id]->game] () {
                            return to_packet(pong::packet::GameState {
                                game->ball,
                                game->pad_left,
                                game->pad_right
                            });
                        }});

                        broadcast_except(partial_packets, valid_users, user, to_packet(pong::packet::NewRoom {
                            static_cast<int>(room_id)
                        }));

                        broadcast_except(partial_packets, valid_users, user, to_packet(pong::packet::OldUser {
                            user.name
                        }));

                        rooms[room_id]->spectators.emplace_back(std::move(user));

                        return false;
                    }

                    default: {
                        std::cerr << "Warning: Receive packet #" << static_cast<int>(*id) << ", expected one of:" << '\n';
                        std::cerr << "\tCreateRoom #" << static_cast<int>(pong::packet::PacketID::CreateRoom) << '\n';
                    }
                }
            }

            if (is_error(received_id)) {
                broadcast_except(partial_packets, valid_users, user, to_packet(pong::packet::OldUser {
                    user.name
                }));
            }

            return is_error(received_id);
        });


        {

            int id{ -1 };
            bool need_refresh = refresh_clock.getElapsedTime().asSeconds() >= refresh_rate;
            if (need_refresh) {
                refresh_clock.restart();
            }

            for(auto& opt_room : rooms) {
                ++id;
                if (!opt_room) {
                    continue;
                }

                auto& room = *opt_room;

                if (!need_refresh && !room.game.force_refresh) { continue; }

                room.game.force_refresh = false;
                room.game.update();
                std::cout << "Update room #" << id << '\n';

                std::unordered_set<sf::TcpSocket*> disconnected;

                for(auto& packet : room.packets) {
                    if (disconnected.count(packet.socket) <= 0 && packet.send() == sf::Socket::Disconnected) {
                        disconnected.insert(packet.socket);
                        std::cout << "disconnection of the client : " << packet.socket << '\n';
                    }
                }

                room.packets.erase(std::remove_if(std::begin(room.packets), std::end(room.packets), [&disconnected] (auto const& p) {
                    return disconnected.count(p.socket) > 0;
                }), std::end(room.packets));

                room.spectators.erase(std::remove_if(std::begin(room.spectators), std::end(room.spectators), [&disconnected] (auto const& p) {
                    return disconnected.count(p.socket.get()) > 0;
                }), std::end(room.spectators));

                if (disconnected.count(room.player_left.socket.get())) {
                    room.player_left = { "", nullptr };
                }

                if (disconnected.count(room.player_right.socket.get())) {
                    room.player_right = { "", nullptr };
                }

                // send disconnection message !

                if (!room.player_left.socket && !room.player_right.socket && room.spectators.size() <= 0) {
                    opt_room = nullptr;
                    std::cout << "Room deleted\n";
                }
            }
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

                    case sf::Socket::Done: {
                        if (i != end - 1) {
                            std::iter_swap(std::begin(partial_packets) + i, std::begin(partial_packets) + (end - 1));
                        }
                        --i;
                        --end;
                        break;
                    }

                    default: {
                        std::cerr << "Error when sending a packet\n";

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

