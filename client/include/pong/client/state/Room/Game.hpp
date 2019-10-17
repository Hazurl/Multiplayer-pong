#pragma once

#include <multipong/Game.hpp>

namespace pong::client::state::room {

struct Game {
    enum class Role {
        Spec, Left, Right
    };

    pong::Ball ball;
    pong::Pad left;
    pong::Pad right;

    void update(float dt, Role role, pong::Input input);
};

}