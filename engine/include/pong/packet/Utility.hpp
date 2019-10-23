#pragma once

#include <vector>
#include <variant>
#include <type_traits>

#include <SFML/Network.hpp>

namespace pong::packet::details {

template<typename V, typename T, std::size_t I>
struct IndexOf;

template<typename...Vs, typename T, std::size_t I>
struct IndexOf<std::variant<T, Vs...>, T, I> {
    static constexpr std::size_t value = I;
};

template<typename V, typename...Vs, typename T, std::size_t I>
struct IndexOf<std::variant<V, Vs...>, T, I> : IndexOf<std::variant<Vs...>, T, I + 1> {};


template<typename V, typename T>
constexpr std::size_t index_of_v = IndexOf<V, T, 0>::value;

template<typename E, typename T>
struct By {
    static_assert(sizeof(E) >= sizeof(T), "Oops");
    T& t;
};


template<typename E, typename T>
By<E, T> by(T& t) {
    return { t };
}

template<typename E, typename T>
sf::Packet& operator >> (sf::Packet& p, By<E, T>&& by) {
    E value;
    p >> value;
    by.t = static_cast<T>(value);
    return p;
}

template<typename E, typename T>
sf::Packet& operator << (sf::Packet& p, By<E, T> const& by) {
    return p << static_cast<E>(by.t);
}




template<typename T>
sf::Packet& operator << (sf::Packet& p, std::vector<T> const& v) {
    p << by<sf::Uint64, std::size_t const>(v.size());
    for(auto const& s : v) {
        p << s;
    }

    return p;
}

template<typename T>
sf::Packet& operator >> (sf::Packet& p, std::vector<T>& v) {
    sf::Uint64 size;
    p >> size;

    v.clear();
    v.reserve(size);
    for(; size > 0; --size) {
        p >> v.emplace_back();
    }

    return p;
}



}