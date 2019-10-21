#pragma once

#include <multipong/Game.hpp>
#include <pong/client/Logger.hpp>

namespace pong::client::state::room {

struct Game {
    enum class Role {
        Spec, Left, Right, Waiting
    };

    pong::Ball ball;
    pong::Pad left;
    pong::Pad right;

    pong::CollisionEvent update(float dt, Role role, pong::Input input);
};

}