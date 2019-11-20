#pragma once

#include <functional>
#include <variant>
#include <memory>

#include <pong/packet/Client.hpp>

#include <pong/client/Notification.hpp>

namespace pong::client::state {

class State;

}


namespace pong::client::action {

struct Quit{};

struct Disconnect{};

using Send = pong::packet::client::Any;

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
    Disconnect,
    notif::Notification
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
    return pong::packet::client::Any{ std::forward<Args>(args)... };
}

inline Action connect(sf::IpAddress addr, unsigned short port, sf::Time timeout = sf::Time::Zero) {
    return Connect {
        std::move(addr), std::move(port), std::move(timeout)
    };
}

inline notif::Notification notification_simple(sf::String message, float close_after = 5.f) {
    return notif::Notification {
        std::move(message),
        notif::Notification::Category::Gameplay,
        notif::Lifetime{
            close_after,
            true
        },
        notif::NoButton{}
    };
}

inline notif::Notification notification_blank(sf::String message) {
    return notif::Notification {
        std::move(message),
        notif::Notification::Category::Gameplay,
        notif::Lifetime{
            {},
            false
        },
        notif::NoButton{}
    };
}

inline notif::Notification notification_with_refresh(sf::String message) {
    return notif::Notification {
        std::move(message),
        notif::Notification::Category::Gameplay,
        notif::Lifetime{
            std::nullopt,
            false
        },
        notif::RefreshButton{}
    };
}

inline notif::Notification notification_with_button(sf::String message, sf::String button_message, notif::Lifetime lifetime = { std::nullopt, false }) {
    return notif::Notification {
        std::move(message),
        notif::Notification::Category::Gameplay,
        std::move(lifetime),
        notif::GenericButton{ std::move(button_message) }
    };

}

inline Actions idle() {
    return Actions{};
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