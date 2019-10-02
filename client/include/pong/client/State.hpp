#pragma once

#include <SFML/Network.hpp>

#include <sftk/eventListener/EventListener.hpp>
#include <sftk/eventListener/Listener.hpp>

#include <memory>
#include <queue>
#include <unordered_map>
#include <cassert>
#include <iostream>

#include <multipong/Game.hpp>
#include <multipong/Packets.hpp>

#include <pong/client/Common.hpp>

namespace pong::client {

struct StateBase;

using socket_ref_t = sf::TcpSocket&;
using packet_t = sf::Packet;

struct Abord{};
struct Idle{};
using ChangeState = std::function<std::unique_ptr<StateBase>(StateBase&)>;

using Action = std::variant<Idle, Abord, ChangeState>; 

template<typename S, typename...Args>
ChangeState change_state(Args&&...args);

struct StateBase : sf::Drawable, sftk::EventListener {


    StateBase(socket_ref_t _socket) : socket{ _socket } {}

    socket_ref_t socket;

    std::queue<std::pair<packet_t, std::function<Action()>>> outgoing_packets;
    std::optional<std::pair<packet_t, std::function<Action()>>> current_outgoing_packet;

    virtual Action dispatch(pong::packet::PacketID id, packet_t packet) = 0;
    virtual void update(float dt) {}
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {}

    template<typename...Args>
    void send_notify(std::function<Action()> f, Args&&...args) {
        assert(outgoing_packets.size() <= 16);

        sf::Packet packet;
        (packet << ... << std::forward<Args>(args));
        outgoing_packets.emplace(std::move(packet), std::move(f));
    }

    template<typename...Args>
    void send(Args&&...args) {
        assert(outgoing_packets.size() <= 16);

        sf::Packet packet;
        (packet << ... << std::forward<Args>(args));
        outgoing_packets.emplace(std::move(packet), std::function<Action()>{});
    }

    Action send_all() {
        while(!outgoing_packets.empty()) {
            if (!current_outgoing_packet) {
                current_outgoing_packet = outgoing_packets.front();
                outgoing_packets.pop();
            }
            

            switch(socket.send(current_outgoing_packet->first)) {
                case sf::Socket::Status::Partial: {
                    return Idle{};
                }

                case sf::Socket::Status::Done: {
                    if (current_outgoing_packet->second) {
                        auto res = current_outgoing_packet->second();
                        if (!std::holds_alternative<Idle>(res)) {
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
            switch(status = socket.receive(packet)) {
                case sf::Socket::Status::Done: {
                    pong::packet::PacketID id;
                    packet >> id;
                    std::cout << "Received #" << (int)id << '\n';
                    auto res = dispatch(id, packet);

                    if(std::holds_alternative<Idle>(res)) break;
                    return res;
                }

                case sf::Socket::Status::NotReady: {
                    return Idle{};
                }

                case sf::Socket::Status::Error:
                case sf::Socket::Status::Partial:
                case sf::Socket::Status::Disconnected: {
                    std::cerr << "RECEIVE ERROR #" << (int)status << '\n';
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

    State(socket_ref_t _socket, receiver_map_t _receivers = {}) : StateBase(_socket), receivers{ std::move(_receivers) } {}

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
    return [tuple_args = std::forward_as_tuple(std::forward<Args>(args)...)] (StateBase& old_state) mutable {
        return std::apply([&old_state] (auto&&..._args) {
            auto s = std::make_unique<S>(old_state.socket, std::forward<decltype(_args)>(_args)...);
            while(!old_state.outgoing_packets.empty()) {
                s->outgoing_packets.emplace(old_state.outgoing_packets.front());
                old_state.outgoing_packets.pop();
            }
            s->current_outgoing_packet = std::move(old_state.current_outgoing_packet);


            return s;
        }, std::move(tuple_args));
    };
}

}