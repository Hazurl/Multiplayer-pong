#pragma once

#include <SFML/Network.hpp>

#include <pong/client/Action.hpp>
#include <pong/client/net/Status.hpp>

#include <pong/packet/Server.hpp>

namespace pong::client::net {

std::pair<Status, std::optional<pong::packet::server::Any>> receive(sf::TcpSocket& socket);

}