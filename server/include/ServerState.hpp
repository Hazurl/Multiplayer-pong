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
#include <cassert>
#include <unordered_set>
#include <experimental/type_traits>
#include <unordered_map>

#include <multipong/Game.hpp>
#include <multipong/Packets.hpp>

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

#define make_getter(C, F) getter<C, decltype(C::F), &C::F>

template<typename C, typename T, T (C::*field)>
T& getter(C& c) {
    return c.*field;
}

template<typename T>
struct UserData {
    std::unique_ptr<sf::TcpSocket> socket;
    T data;
    std::vector<sf::Packet> packets;
};

template<>
struct UserData<void> {
    std::unique_ptr<sf::TcpSocket> socket;
    std::vector<sf::Packet> packets;
};

struct Idle {};
using Leave = std::function<void()>;
struct Abord {}; 

using Action = std::variant<Idle, Leave, Abord>;

template<typename S, typename T>
Action change_state(S& state, std::unique_ptr<sf::TcpSocket>& socket, T&& data) {
    return [&socket, &state, _data = std::forward<T>(data)] () mutable {
        state.create(std::move(socket), std::move(_data));
    };
}

template<typename S, typename T>
Action change_state(S& state, std::unique_ptr<sf::TcpSocket>& socket) {
    return [&socket, &state] () mutable {
        state.create(std::move(socket));
    };
}

template<typename S>
using decltype_on_user_leave = decltype(&S::on_user_leave); 

template<typename S>
using decltype_on_user_enter = decltype(&S::on_user_enter); 

struct PacketIDHasher {
    std::size_t operator()(pong::packet::PacketID const& p) const {
        return static_cast<std::size_t>(p);
    }
};


template<typename C, typename T>
struct State {

    using user_personal_data_t = T;
    using user_data_t = UserData<user_personal_data_t>;
    using packet_t = sf::Packet;
    using user_handle_t = std::size_t;

    using receiver_t = Action (C::*)(user_data_t&, packet_t&);
    using sender_t = Action (C::*)(user_data_t&);

    using receiver_map_t = std::unordered_map<pong::packet::PacketID, receiver_t, PacketIDHasher>;
    using sender_map_t = std::unordered_map<pong::packet::PacketID, sender_t, PacketIDHasher>;

    using on_user_enter_t = void (C::*)(user_data_t&);
    using on_user_leave_t = void (C::*)(user_data_t&);

    std::vector<user_data_t> users;
    receiver_map_t receivers;

    State(
        receiver_map_t const& receive = {},
        sender_map_t   const& /* send */ = {}
    ) : receivers{ receive } {}

    void send_packets() {
        std::size_t end{ users.size() };
        for(std::size_t i{ 0 }; i < end; ++i) {
            auto& user = users[i];
            auto& socket = *user.socket;
            auto& packets = user.packets;

            {
                auto packet_it = std::begin(packets);
                bool stop{ false };

                for(; !stop && packet_it != std::end(packets); ++packet_it) {
                    auto& packet = *packet_it;
                    switch(socket.send(packet)) {
                        case sf::Socket::Partial: {
                            // Can't send any packets to this user
                            stop = true;
                            break;
                        }

                        case sf::Socket::Done: {
                            break;
                        }

                        default: {
                            std::cerr << "Error when sending a packet\n";

                            // Prepare the user to be removed without breaking the current loop
                            if (i != end - 1) {
                                std::iter_swap(std::begin(users) + i, std::begin(users) + (end - 1));
                            }
                            --i;    
                            --end;

                            stop = true;
                            break;
                        }
                    }
                }

                packets.erase(std::begin(packets), packet_it);
            }
        }

        {
            // Finnally, remove the users that got an error
            auto first = std::begin(users) + end;
            auto last = std::end(users);

            if constexpr (std::experimental::is_detected_exact_v<on_user_leave_t, decltype_on_user_leave, C>) {
                auto current = first;
                for(; current != last; ++current) {
                    static_cast<C*>(this)->on_user_leave(*current);
                }
            }

            users.erase(first, last);
        }
    }

    void receive_packets() {
        std::size_t end{ users.size() };
        for(std::size_t i{ 0 }; i < end; ++i) {
            auto& user = users[i];
            auto& socket = *user.socket;

            sf::Packet packet;
            switch(socket.receive(packet)) {
                case sf::Socket::NotReady: {
                    break;
                }

                case sf::Socket::Done: {
                    pong::packet::PacketID packet_id; 
                    packet >> packet_id;
                    if (receivers.count(packet_id)) {
                        auto action = (static_cast<C*>(this)->*receivers[packet_id])(user, packet);
                        if (auto* finalize_leave = std::get_if<Leave>(&action)) {
                            if constexpr (std::experimental::is_detected_exact_v<on_user_leave_t, decltype_on_user_leave, C>) {
                                static_cast<C*>(this)->on_user_leave(user);
                            }
                            (*finalize_leave)();
                        }

                        if (!std::holds_alternative<Idle>(action)) {
                            if (i != end - 1) {
                                std::iter_swap(std::begin(users) + i, std::begin(users) + (end - 1));
                            }
                            --i;
                            --end;
                        }
                    } else {
                        std::cerr << "[Warning] Received Packet #" << static_cast<int>(packet_id) << " but wasn't expected\n";
                    }
                    break;
                }

                default: {
                    std::cerr << "Error when receiving a packet\n";

                    // Prepare the user to be removed without breaking the current loop
                    if (i != end - 1) {
                        std::iter_swap(std::begin(users) + i, std::begin(users) + (end - 1));
                    }
                    --i;    
                    --end;

                    break;
                }
            }
        }

        {
            // Finnally, remove the users that got an error
            auto first = std::begin(users) + end;
            auto last = std::end(users);

            if constexpr (std::experimental::is_detected_exact_v<on_user_leave_t, decltype_on_user_leave, C>) {
                auto current = first;
                for(; current != last; ++current) {
                    if (current->socket) {
                        static_cast<C*>(this)->on_user_leave(*current);
                    }
                }
            }

            users.erase(first, last);
        }
    }

    template<typename...Ps>
    void broadcast(Ps&&... ps) {
        auto packet = to_packet(std::forward<Ps>(ps)...);
        for(auto& user : users) {
            send_packet(user, packet);
        }
    }

    template<typename...Ps>
    void broadcast_other(user_data_t const& exception, Ps&&... ps) {
        auto packet = to_packet(std::forward<Ps>(ps)...);
        for(auto& user : users) {
            if (&user != &exception) {
                send_packet(user, packet);
            }
        }
    }

    std::enable_if_t<! std::is_same_v<void, user_personal_data_t>, 
    user_handle_t> create(std::unique_ptr<sf::TcpSocket> socket, T&& data) {
        assert(socket && "Socket must not be null");
        users.push_back({ std::move(socket), std::forward<T>(data) });

        // State has the `on_user_enter` member
        if constexpr (std::experimental::is_detected_exact_v<on_user_enter_t, decltype_on_user_enter, C>) {
            auto& new_user = users.back();
            static_cast<C*>(this)->on_user_enter(new_user);
        }

        return users.size() - 1;
    }

    std::enable_if_t<std::is_same_v<void, user_personal_data_t>, 
    user_handle_t> create(std::unique_ptr<sf::TcpSocket> socket) {
        assert(socket && "Socket must not be null");
        users.push_back({ std::move(socket) });

        // State has the `on_user_enter` member
        if constexpr (std::experimental::is_detected_exact_v<on_user_enter_t, decltype_on_user_enter, C>) {
            auto& new_user = users.back();
            static_cast<C*>(this)->on_user_enter(new_user);
        }

        return users.size() - 1;
    }

    template<typename D>
    void send(user_data_t& user, D&& data) {
        return send_packet(user, to_packet(std::forward<D>(data)));
    }

    void send_packet(user_data_t& user, sf::Packet const& packet) {
        if(user.packets.size() >= 16) {
            std::cerr <<"User packets count can't exceed the maximum allowed\n";
            return;
        }
        user.packets.push_back(packet);
    }

};

using user_t = std::string;

struct MainLobbyState;

struct RoomState : public State<RoomState, user_t> {
    RoomState(MainLobbyState& main_lobby) : State({
        // Receive
        { pong::packet::PacketID::Input, &RoomState::on_input },
        { pong::packet::PacketID::LeaveRoom, &RoomState::on_leave_room },
    }, {

    }), main_lobby{ main_lobby }, left_player{ nullptr }, right_player{ nullptr } {}

    MainLobbyState& main_lobby;
    user_data_t* left_player;
    user_data_t* right_player;

    Action on_input(user_data_t& user, packet_t& packet) {
        if (&user == left_player) {
            auto input = from_packet<pong::packet::Input>(packet);
        }
        else if (&user == right_player) {
            auto input = from_packet<pong::packet::Input>(packet);
        }
        else {
            std::cerr << "[Warning] Received PacketID::Input from a spectator\n";
        }

        return Idle{};
    }

    Action on_leave_room(user_data_t& user, packet_t& packet) {
        return change_state(main_lobby, user.socket, user.data);
    }

    void on_user_enter(user_data_t& user_data) {
        broadcast_other(user_data, pong::packet::NewUser{
            user_data.data
        });

        std::vector<std::string> spectators;
        spectators.reserve(users.size());
        for(auto const& user : users) {
            if (&user != left_player && &user != right_player) {
                spectators.push_back(user.data);
            }
        }

        send(user_data, pong::packet::RoomInfo{
            left_player ? left_player->data : "",
            right_player ? right_player->data : "",
            spectators
        });
    }

    void on_user_leave(user_data_t& user) {
        if (&user == left_player) {
            left_player = nullptr;
        }
        else if (&user == right_player) {
            right_player = nullptr;
        }

        broadcast_other(user, pong::packet::OldUser{
            user.data
        });
    }
};

struct MainLobbyState : public State<MainLobbyState, user_t> {
    MainLobbyState(std::vector<std::unique_ptr<RoomState>>& rooms) : State({
        // Receive
        { pong::packet::PacketID::CreateRoom, &MainLobbyState::on_create_room }
    }, {

    }), rooms{ rooms } {}

    std::vector<std::unique_ptr<RoomState>>& rooms;

    Action on_create_room(user_data_t& user, packet_t& packet) {
        // Find an ID without a room
        std::size_t room_id{ 0 };
        for(; room_id < rooms.size(); ++room_id) {
            if (!rooms[room_id]) {
                break;
            }
        }

        std::cout << "New room #" << room_id << " created\n";
        if (room_id >= rooms.size()) {
            rooms.emplace_back(std::make_unique<RoomState>(*this));
        } else {
            rooms[room_id] = std::make_unique<RoomState>(*this);
        }

        broadcast_other(user, pong::packet::NewRoom{
            static_cast<int>(room_id)
        });

        return change_state(
            *rooms[room_id],
            user.socket,
            user.data
        );
    }

    void on_user_enter(user_data_t& user) {
        std::vector<std::string> usernames;
        usernames.reserve(users.size());
        std::transform(std::begin(users), std::end(users), std::back_inserter(usernames), make_getter(user_data_t, data));

        std::vector<int> rooms_id;

        for(std::size_t room_id{ 0 }; room_id < rooms.size(); ++room_id) {
            if (rooms[room_id]) {
                rooms_id.emplace_back(static_cast<int>(room_id));
            }
        }

        send(user, pong::packet::LobbyInfo{
            usernames, rooms_id
        });

        broadcast_other(user, pong::packet::NewUser{
            user.data
        });
    }

    void on_user_leave(user_data_t& user) {
        broadcast_other(user, pong::packet::OldUser{
            user.data
        });
    }
};

struct NewUserState : public State<NewUserState, void> {
    NewUserState(MainLobbyState& main_lobby) : State({
        // Receive
        { pong::packet::PacketID::ChangeUsername, &NewUserState::on_username_changed }
    }, {

    }), main_lobby{ main_lobby } {}

    MainLobbyState& main_lobby;

    Action on_username_changed(user_data_t& user, packet_t& packet) {
        auto username = from_packet<pong::packet::ChangeUsername>(packet).username;

        auto is_valid = is_username_valid(username);

        send(user, pong::packet::UsernameResponse{
            is_valid
        });

        if (!is_valid) {
            return Idle{};
        }

        std::cout << username << " is now connected\n";

        return change_state(
            main_lobby,
            user.socket,
            username
        );

    }
};