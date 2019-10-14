#pragma once

#include <queue>

#include <SFML/Network.hpp>

#include <multipong/Packets.hpp>

#include <pong/client/net/Status.hpp>

namespace pong::client::net {

class PacketQueue {
private:

    std::queue<pong::packet::GamePacket> packets;
    sf::Packet packet;

public:

    static constexpr std::size_t maximum_queue_size = 16;

    bool push(pong::packet::GamePacket packet);
    bool empty() const;

    std::pair<Status, std::optional<pong::packet::GamePacket>> send(sf::TcpSocket& socket);

};

}