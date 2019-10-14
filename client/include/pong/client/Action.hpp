#pragma once

#include <functional>
#include <variant>
#include <memory>

#include <multipong/Packets.hpp>

namespace pong::client::state {

class State;

}


namespace pong::client::action {

struct Quit{};

struct Disconnect{};

using Send = pong::packet::GamePacket;

using ChangeState = std::unique_ptr<state::State>;

struct Connect {
    sf::IpAddress addr;
    unsigned short port;
    sf::Time timeout;
};
using Action = std::variant<
    ChangeState,
    Quit,
    Send,
    Connect,
    Disconnect
>;

using Actions = std::vector<Action>;


template<typename S, typename...Args>
inline Action change_state(Args&&...args) {
    return std::make_unique<S>(std::forward<Args>(args)...);
}

inline Action quit() {
    return Quit{};
} 

inline Action disconnect() {
    return Disconnect{};
} 

template<typename...Args>
inline Action send(Args&&...args) {
    return pong::packet::GamePacket{ std::forward<Args>(args)... };
}

inline Action connect(sf::IpAddress addr, unsigned short port, sf::Time timeout = sf::Time::Zero) {
    return Connect {
        std::move(addr), std::move(port), std::move(timeout)
    };
}

template<typename...Args>
inline Actions seq(Args&&...args) {
    Actions actions;
    actions.reserve(sizeof...(Args));
    (
        actions.emplace_back(std::forward<Args>(args)),
        ...
    );
    return actions;
}

template<typename...Args>
Actions fuse(Actions actions, Actions next_actions, Args&&...args) {
    std::move(std::begin(next_actions), std::end(next_actions), std::back_inserter(actions));
    return fuse(std::move(actions), std::forward<Args>(args)...);
}

inline Actions fuse(Actions actions) {
    return actions;
}



}