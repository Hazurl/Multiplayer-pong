#pragma once

#include <SFML/Network.hpp>

#include <pong/client/Action.hpp>
#include <pong/client/net/Status.hpp>

namespace pong::client::net {

std::pair<Status, std::optional<pong::packet::GamePacket>> receive(sf::TcpSocket& socket);

}