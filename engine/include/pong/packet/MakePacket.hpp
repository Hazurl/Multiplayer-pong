#pragma once

#include <SFML/Network.hpp>

namespace pong::packet {
    using id_t = sf::Uint32;
}
/*
    Create a packet with the name P_ and the id N
    It supports `==`, `to_string` and `<<` with std::ostream
    And obviously can be (de)serialized into sf::Packet
*/

#define MAKE_PACKET(P_)                                             \
struct P_;                                                          \
                                                                    \
sf::Packet& operator >> (sf::Packet& p, P_& packet);                \
sf::Packet& operator << (sf::Packet& p, P_ const& packet);          \
bool operator == (P_ const& lhs, P_ const& rhs);                    \
std::ostream& operator <<(std::ostream& os, P_ const& packet);      \
std::string to_string(P_ const& packet);                            \
                                                                    \
struct P_
