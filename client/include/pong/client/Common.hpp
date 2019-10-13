#pragma once

#include<numeric>

namespace pong::client {


template<typename...Ts>
sf::Packet to_packet(Ts&&... ts) {
    sf::Packet p;
    (p << ... << ts);
    return p;
}

template<typename T>
T from_packet(sf::Packet& p) {
    T t;
    p >> t;
    return t;
}



struct PacketIDHasher {
    inline std::size_t operator()(pong::packet::PacketID const& p) const {
        return static_cast<std::size_t>(p);
    }
};

}