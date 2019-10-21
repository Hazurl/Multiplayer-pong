#include <pong/client/net/Receiver.hpp>
#include <pong/client/Logger.hpp>
#include <pong/client/state/State.hpp>

namespace pong::client::net {

std::pair<Status, std::optional<pong::packet::server::Any>> receive(sf::TcpSocket& socket) {
    sf::Packet packet;
    switch(socket.receive(packet)) {
        case sf::Socket::Status::Done: {
            pong::packet::server::Any game_packet;
            packet >> game_packet;
            return std::make_pair(
                Status::Available,
                std::move(game_packet)
            );
        }

        case sf::Socket::Status::NotReady: {
            return std::make_pair(
                Status::Done,
                std::nullopt
            );
        }

        default: {
            ERROR("Error when receiving a packet");
            return std::make_pair(
                Status::Error,
                std::nullopt
            );
        }
    }
}

}