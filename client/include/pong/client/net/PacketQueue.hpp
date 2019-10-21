#pragma once

#include <queue>

#include <SFML/Network.hpp>

#include <pong/packet/Client.hpp>

#include <pong/client/net/Status.hpp>

namespace pong::client::net {

class PacketQueue {
private:

    std::queue<pong::packet::client::Any> packets;
    sf::Packet packet;
    bool partially_send = false;

public:

    static constexpr std::size_t maximum_queue_size = 16;

    bool push(pong::packet::client::Any packet);
    bool empty() const;

    std::pair<Status, std::optional<pong::packet::client::Any>> send(sf::TcpSocket& socket);

};

}