#include <pong/client/net/PacketQueue.hpp>

#include <pong/client/Logger.hpp>
#include <pong/client/state/State.hpp>

namespace pong::client::net {

bool PacketQueue::push(pong::packet::GamePacket new_packet) {
    if (packets.size() >= maximum_queue_size) {
        WARN("Maximum capacity reached");
        return false;
    }

    NOTICE("Push packet #", new_packet.index());
    packets.emplace(std::move(new_packet));
    return true;
}

bool PacketQueue::empty() const {
    return packets.empty();
}

std::pair<Status, std::optional<pong::packet::GamePacket>> PacketQueue::send(sf::TcpSocket& socket) {
    if (empty()) {
        WARN("No packet in the queue");
        return std::make_pair(
            Status::Available,
            std::nullopt
        );
    }

    if (packet.endOfPacket()) {
        auto const& game_packet = packets.front();
        NOTICE("Prepare packet #", game_packet.index());
        packet.clear();
        packet << game_packet;
    }

    switch(socket.send(packet)) {
        case sf::Socket::Status::Partial: {
            return std::make_pair(
                Status::Done,
                std::nullopt
            );
        }

        case sf::Socket::Status::Done: {
            SUCCESS("Packet #", packets.front().index(), " sent");
            auto ret = std::make_pair(
                Status::Available,
                std::move(packets.front())
            );
            packets.pop();
            return ret;
        }

        case sf::Socket::Status::NotReady: {
            return std::make_pair(
                Status::Done,
                std::nullopt
            );
        }

        default: {
            ERROR("Error when sending packet #", packets.front().index());
            return std::make_pair(
                Status::Error,
                std::nullopt
            );
        }
    }
}

}