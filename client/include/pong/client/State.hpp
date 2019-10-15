#pragma once

#include <SFML/Network.hpp>

#include <sftk/eventListener/EventListener.hpp>
#include <sftk/eventListener/Listener.hpp>

#include <memory>
#include <queue>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <future>
#include <chrono>

#include <multipong/Game.hpp>
#include <multipong/Packets.hpp>

#include <pong/client/Common.hpp>

#include <pong/client/gui/Element.hpp>

namespace pong::client {

namespace details {

template<typename T>
bool is_future_ready(std::future<T> const& f) { 
    return f.valid() && f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

}

struct StateBase;

using socket_ptr_t = std::unique_ptr<sf::TcpSocket>;
using packet_t = sf::Packet;

struct Abord{};
struct Idle{};
using ChangeState = std::function<std::unique_ptr<StateBase>(StateBase&)>;

using Action = std::variant<Idle, Abord, ChangeState>; 

template<typename S, typename...Args>
ChangeState change_state(Args&&...args);

struct StateBase : sf::Drawable, sftk::EventListener, gui::Element {


    StateBase(socket_ptr_t _socket = nullptr) : socket{ std::move(_socket) } {}

    virtual ~StateBase() = default;


    std::unique_ptr<sf::TcpSocket> socket;
    std::future<std::unique_ptr<sf::TcpSocket>> future_socket;

    std::queue<std::pair<packet_t, std::function<Action()>>> outgoing_packets;
    std::optional<std::pair<packet_t, std::function<Action()>>> current_outgoing_packet;

    virtual Action dispatch(pong::packet::PacketID id, packet_t packet) = 0;
    virtual Action update(float) { return Idle{}; }
    virtual void draw(sf::RenderTarget&, sf::RenderStates) const {}

    bool is_connected() const {
        return socket != nullptr;
    }

    bool is_connecting() const {
        return future_socket.valid();
    }

    void stop_connection() {
        socket = nullptr;
        future_socket = {};
        on_diconnection();
        assert(!is_connecting());
    }

    void attempt_to_connect(sf::IpAddress const& _addr, unsigned short _port, sf::Time _timeout = sf::Time::Zero) {
        assert(!is_connecting());
        future_socket = std::async(std::launch::async, [] (sf::IpAddress const& addr, unsigned short port, sf::Time timeout) -> std::unique_ptr<sf::TcpSocket> {
            auto socket_ptr = std::make_unique<sf::TcpSocket>();
            if (socket_ptr->connect(addr, port, timeout) != sf::Socket::Done) {
                return nullptr;
            }
            socket_ptr->setBlocking(false);
            return socket_ptr;
        }, _addr, _port, _timeout);
    }

    bool check_connection() {
        if (details::is_future_ready(future_socket)) {
            std::unique_ptr<sf::TcpSocket> res = future_socket.get();
            if (!res) {
                std::cerr << "Couldn't connect to server... Abording...\n";
                stop_connection();
                return false;
            }

            socket = std::move(res);

            on_connection();   
            return true;
        }
        return false;
    }

    virtual void on_connection() { std::cout << "Connected\n!"; }
    virtual void on_diconnection() { std::cout << "Disconnected\n!";}

    template<typename...Args>
    void send_notify(std::function<Action()> f, Args&&...args) {
        assert(outgoing_packets.size() <= 16);

        sf::Packet packet;
        (packet << ... << std::forward<Args>(args));
        outgoing_packets.emplace(std::move(packet), std::move(f));
        std::cout << "Add packets\n";
    }

    template<typename...Args>
    void send(Args&&...args) {
        assert(outgoing_packets.size() <= 16);

        sf::Packet packet;
        (packet << ... << std::forward<Args>(args));
        outgoing_packets.emplace(std::move(packet), std::function<Action()>{});
        std::cout << "Add packets\n";
    }

    Action send_all() {       
        while(!outgoing_packets.empty()) {
            if (!current_outgoing_packet) {
                current_outgoing_packet = outgoing_packets.front();
                outgoing_packets.pop();
                std::cout << "Remove packets\n";
            }
            

            switch(socket->send(current_outgoing_packet->first)) {
                case sf::Socket::Status::Partial: {
                    return Idle{};
                }

                case sf::Socket::Status::Done: {
                    if (current_outgoing_packet->second) {
                        auto res = current_outgoing_packet->second();
                        if (!std::holds_alternative<Idle>(res)) {
                            if(std::holds_alternative<Abord>(res)) stop_connection();
                            current_outgoing_packet = std::nullopt;
                            return res;
                        }
                    }
                    current_outgoing_packet = std::nullopt;
                    break;
                }

                case sf::Socket::Status::NotReady: {
                    return Idle{};
                }

                case sf::Socket::Status::Error:
                case sf::Socket::Status::Disconnected: {
                    stop_connection();
                    return Abord{};
                }
            }
        }

        return Idle{};
    }

    Action receive_all() {
        packet_t packet;
        sf::Socket::Status status;
        while(true) {
            switch(status = socket->receive(packet)) {
                case sf::Socket::Status::Done: {
                    pong::packet::PacketID id;
                    packet >> id;
                    std::cout << "Received #" << static_cast<int>(id) << '\n';
                    auto res = dispatch(id, packet);

                    if(std::holds_alternative<Idle>(res)) break;
                    if(std::holds_alternative<Abord>(res)) stop_connection();
                    return res;
                }

                case sf::Socket::Status::NotReady: {
                    return Idle{};
                }

                case sf::Socket::Status::Error:
                case sf::Socket::Status::Partial:
                case sf::Socket::Status::Disconnected: {
                    std::cerr << "RECEIVE ERROR #" << static_cast<int>(status) << '\n';
                    stop_connection();
                    return Abord{};
                }
            }
        }
    }

};

template<typename C>
struct State : StateBase {

    using receiver_t = Action (C::*)(packet_t);
    using receiver_map_t = std::unordered_map<pong::packet::PacketID, receiver_t, PacketIDHasher>;

    receiver_map_t receivers;

    State(socket_ptr_t _socket, receiver_map_t _receivers = {}) : StateBase(std::move(_socket)), receivers{ std::move(_receivers) } {}

    Action dispatch(pong::packet::PacketID id, packet_t packet) override {
        if (receivers.count(id)) {
            auto& receiver = receivers[id];
            return (static_cast<C*>(this)->*receiver)(packet);
        }

        std::cerr << "Received unexpected packet #" << static_cast<int>(id) << '\n';
        return Idle{};
    }

};

template<typename S, typename...Args>
ChangeState change_state(Args&&...args) {
    return [tuple_args = std::tuple<Args...>(std::forward<Args>(args)...)] (StateBase& old_state) mutable {
        return std::apply([&old_state] (auto&&..._args) {
            auto s = std::make_unique<S>(std::move(old_state.socket), std::forward<decltype(_args)>(_args)...);
            while(!old_state.outgoing_packets.empty()) {
                std::cout << "Transfert packets\n";
                s->outgoing_packets.emplace(old_state.outgoing_packets.front());
                old_state.outgoing_packets.pop();
            }
            s->current_outgoing_packet = std::move(old_state.current_outgoing_packet);

            return s;
        }, std::move(tuple_args));
    };
}

}