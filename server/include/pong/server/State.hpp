#pragma once

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

#include <pong/server/Common.hpp>

namespace pong::server {

template<typename C>
struct StateBase {
    using packet_t = sf::Packet;

    using receiver_t = Action (C::*)(user_handle_t, packet_t);
    using sender_t = Action (C::*)(user_handle_t);

    using receiver_map_t = std::unordered_map<pong::packet::PacketID, receiver_t, PacketIDHasher>;
    using sender_map_t = std::unordered_map<pong::packet::PacketID, sender_t, PacketIDHasher>;

    using on_user_enter_t = void (C::*)(user_handle_t);
    using on_user_leave_t = void (C::*)(user_handle_t);

    static constexpr bool has_on_user_leave{ std::experimental::is_detected_exact_v<on_user_leave_t, decltype_on_user_leave, C> };
    static constexpr bool has_on_user_enter{ std::experimental::is_detected_exact_v<on_user_enter_t, decltype_on_user_enter, C> };


    static constexpr std::size_t max_number_of_packet{ 16 };


private:

    template<typename C_> friend struct StateBase;

    receiver_map_t receivers;
    std::vector<User> users;


public:


    StateBase(
        receiver_map_t const& receive = {}
    ) : receivers{ receive } {}


    template<typename...Args>
    user_handle_t create(std::unique_ptr<sf::TcpSocket> socket) {
        assert(socket && "Socket must not be null");


        user_handle_t handle = number_of_user();
        users.push_back({ std::move(socket) });


        // State has the `on_user_enter` member
        if constexpr (has_on_user_enter) {
            static_cast<C*>(this)->on_user_enter(handle);
        }


        return handle;
    }


    bool is_valid(user_handle_t handle) const {
        return handle != invalid_user_handle && handle < users.size() && users[handle].socket != nullptr;
    }


    User& get_user(user_handle_t handle) {
        assert(is_valid(handle));

        return users[handle];
    }


    User const& get_user(user_handle_t handle) const {
        assert(is_valid(handle));

        return users[handle];
    }


    user_id_t get_user_id(user_handle_t handle) const {
        return reinterpret_cast<user_id_t>(get_user(handle).socket.get());
    }


    user_handle_t get_user_handle(user_id_t id) const {
        for(user_handle_t handle = 0; handle < users.size(); ++handle) {
            if (reinterpret_cast<user_id_t>(users[handle].socket.get()) == id) {
                assert(is_valid(handle));
                return handle;
            }
        }

        return invalid_user_handle;
    }


    std::size_t number_of_user() const {
        return users.size();
    }


    bool is_empty() const {
        return number_of_user() == 0;
    }


    template<typename...Args>
    void send(user_handle_t handle, Args&&...args) {
        return send_packet(handle, to_packet(std::forward<Args>(args)...));
    }


    void send_packet(user_handle_t handle, sf::Packet const& packet) {
        assert(is_valid(handle));


        if(users[handle].packets.size() >= max_number_of_packet) {
            std::cerr << "User packets count can't exceed the maximum allowed\n";
            return;
        }


        users[handle].packets.push_back(packet);
    }


    template<typename...Ps>
    void broadcast(Ps&&... ps) {
        auto packet = to_packet(std::forward<Ps>(ps)...);
        for(user_handle_t handle{ 0 }; handle < number_of_user(); ++handle) {
            send_packet(handle, packet);
        }
    }


    template<typename...Ps>
    void broadcast_other(user_handle_t except_handle, Ps&&... ps) {
        auto packet = to_packet(std::forward<Ps>(ps)...);
        for(user_handle_t handle{ 0 }; handle < number_of_user(); ++handle) {
            if (handle != except_handle) {
                send_packet(handle, packet);
            }
        }
    }


    template<typename S, typename...Args>
    Action order_change_state(S& state, user_handle_t handle, Args&&...args) {
        return [this, handle, &socket = users[handle].socket, &state, tuple_args = std::make_tuple(std::forward<Args>(args)...)] () mutable {
            auto new_handle = std::apply([&socket, &state] (auto&&..._args) {
                return state.create(std::move(socket), std::forward<decltype(_args)>(_args)...);
            }, std::move(tuple_args));

            std::swap(users[handle].packets, state.users[new_handle].packets);
            for(auto p : users[handle].packets) {
                state.users[new_handle].packets.push_back(p);
            }
        };
    }


    std::optional<Action> invoke_receiver(pong::packet::PacketID packet_id, user_handle_t handle, sf::Packet packet) {
        if (has_receiver_for(packet_id)) {
            return (static_cast<C*>(this)->*receivers[packet_id])(handle, packet);
        } else {
            return std::nullopt;
        }
    }

    bool has_receiver_for(pong::packet::PacketID packet_id) const {
        return receivers.count(packet_id);
    }


protected:


    void swap_users(user_handle_t lhs, user_handle_t rhs) {
        if (lhs != rhs) {
            std::iter_swap(std::begin(users) + lhs, std::begin(users) + rhs);
        }
    }

    void remove_users(user_handle_t first_non_valid_handle) {
        auto first = std::begin(users) + first_non_valid_handle;
        auto last = std::end(users);


        users.erase(first, last);
    }


};





template<typename C, typename T>
struct StateWithData : StateBase<C> {
private:


    std::vector<T> user_datas;

    using base_t = StateBase<C>;


public:


    StateWithData(
        typename base_t::receiver_map_t const& receive = {}
    ) : StateBase<C>(receive) {}


    template<typename...Args>
    user_handle_t create(std::unique_ptr<sf::TcpSocket> socket, Args&&...args) {
        // Push the data before creating the user otherwise on_user_enter won't have access to it
        user_datas.emplace_back(std::forward<Args>(args)...);


        auto handle = base_t::create(std::move(socket));
        assert(handle == user_datas.size() - 1);


        return handle;
    }


    T& get_user_data(user_handle_t handle) {
        assert(base_t::is_valid(handle));
        assert(handle != invalid_user_handle);
        return user_datas[handle];
    }


    T const& get_user_data(user_handle_t handle) const {
        assert(base_t::is_valid(handle));
        return user_datas[handle];
    }


protected:


    void swap_users(user_handle_t lhs, user_handle_t rhs) {
        if (lhs != rhs) {
            std::iter_swap(std::begin(user_datas) + lhs, std::begin(user_datas) + rhs);
            base_t::swap_users(lhs, rhs);
        }
    }

    void remove_users(user_handle_t first_non_valid_handle) {
        auto first = std::begin(user_datas) + first_non_valid_handle;
        auto last = std::end(user_datas);


        user_datas.erase(first, last);


        base_t::remove_users(first_non_valid_handle);
    }

};





template<typename C>
struct StateWithData<C, void> : StateBase<C> {
private:


    using base_t = StateBase<C>;


public:


    StateWithData(
        typename base_t::receiver_map_t const& receive = {}
    ) : StateBase<C>(receive) {}


};


template<typename C, typename T = void>
struct State : StateWithData<C, T> {

    using base_t = StateWithData<C, T>;

    using typename base_t::receiver_t;
    using typename base_t::sender_t;
    using typename base_t::receiver_map_t;
    using typename base_t::sender_map_t;

    using base_t::has_on_user_leave;
    using base_t::has_on_user_enter;

    State(
        receiver_map_t const& receive = {}
    ) : StateWithData<C, T>(receive) {}


private:


    Action
    proccess_receive_packets(user_handle_t handle) {
        sf::Packet packet;
        switch(base_t::get_user(handle).socket->receive(packet)) {
            case sf::Socket::NotReady: {
                return Idle{};
            }

            case sf::Socket::Done: {
                auto packet_id = from_packet<pong::packet::PacketID>(packet); 

                auto action = base_t::invoke_receiver(packet_id, handle, packet);
                if (action) {
                    return std::move(*action);
                }

                
                std::cerr << "[Warning] Received Packet #" << static_cast<int>(packet_id) << " but wasn't expected\n";
                return Idle{};
            }

            default: {
                std::cerr << "Error when sending a packet\n";
                return Abord{};
            }
        }
    }


public:


    void receive_packets() {

        std::size_t first_invalid_handler{ base_t::number_of_user() };


        for(user_handle_t handle{ 0 }; handle < first_invalid_handler;) {         
            auto action = proccess_receive_packets(handle);


            if (std::holds_alternative<Idle>(action)) {


                ++handle;


            } else {
                if constexpr (base_t::has_on_user_leave) {
                    static_cast<C*>(this)->on_user_leave(handle);
                }


                if (auto* finalize_leave = std::get_if<Leave>(&action)) {
                    (*finalize_leave)();
                } 


                base_t::swap_users(handle, first_invalid_handler - 1);
                --first_invalid_handler;


            }
        }

        // Finally, remove the users that got an error
        base_t::remove_users(first_invalid_handler);
    }


private:

    using packet_iterator_t = typename std::vector<sf::Packet>::iterator;

    /*
        std::nullopt => Remove user
        { iterator } => Remove packets before iterator (not included)
    */
    static std::optional<packet_iterator_t>
    proccess_send_packets(packet_iterator_t packet_it, packet_iterator_t const end_packets, sf::TcpSocket& socket) {
        for(; packet_it != end_packets; ++packet_it) {
            switch(socket.send(*packet_it)) {
                case sf::Socket::Partial:
                case sf::Socket::Done:
                    break;

                default: {
                    std::cerr << "Error when sending a packet\n";
                    return std::nullopt;
                }
            }
        }

        return packet_it;
    }


public:


    void send_packets() {


        std::size_t first_invalid_handler{ base_t::number_of_user() };


        for(user_handle_t handle{ 0 }; handle < first_invalid_handler;) {
            auto& user = base_t::get_user(handle);
            auto& packets = user.packets;
            

            auto res = proccess_send_packets(std::begin(packets), std::end(packets), *user.socket);
            if (res) {


                packets.erase(std::begin(packets), *res);

                ++handle;


            } else {


                if constexpr (has_on_user_leave) {
                    static_cast<C*>(this)->on_user_leave(handle);
                }


                base_t::swap_users(handle, first_invalid_handler - 1);
                --first_invalid_handler;

            }
        }


        // Finally, remove the users that got an error
        base_t::remove_users(first_invalid_handler);
    }

};


}