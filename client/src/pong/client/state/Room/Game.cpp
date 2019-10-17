#include <pong/client/state/Room/Game.hpp>

namespace pong::client::state::room {

void Game::update(float dt, Role role, pong::Input input) {

    if (role == Game::Role::Left) {
        left.update(dt, input);
    } else {
        left.update(dt);
    }


    if (role == Game::Role::Right) {
        right.update(dt, input);
    } else {
        right.update(dt);
    }


    ball.update(dt, left.y, right.y);

}


}