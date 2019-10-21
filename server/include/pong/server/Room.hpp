#pragma once

#include <pong/server/Common.hpp>
#include <pong/server/State.hpp>

#include <deque>

namespace pong::server {

struct MainLobbyState;


struct Game {
    static constexpr float ball_radius      { 8 };
    static constexpr float pad_height       { 80 };
    static constexpr float pad_width        { 12 };
    static constexpr float pad_padding      { 40 };
    static constexpr float pad_max_speed    { 200 };
    static constexpr float ball_max_speed   { 100 };
    static constexpr float boundaries_x     { 800 };
    static constexpr float boundaries_y     { 600 };
    static constexpr float ball_boundaries_x{ boundaries_x - ball_radius };
    static constexpr float ball_boundaries_y{ boundaries_y - ball_radius };
    static constexpr float pad_boundary     { boundaries_y - pad_height };

    static const     sf::Vector2f boundaries;
    static const     sf::Vector2f ball_boundaries;

    pong::Input input_left;
    pong::Input input_right;

    pong::Ball ball;
    pong::Pad pad_left;
    pong::Pad pad_right;

    Game() 
    :   input_left{ pong::Input::Idle }
    ,   input_right{ pong::Input::Idle }
    ,   ball{ sf::Vector2f{ ball_boundaries_x, ball_boundaries_y } / 2.f, { ball_max_speed, ball_max_speed }}
    ,   pad_left{ pad_boundary / 2.f, 0 }
    ,   pad_right{ pad_boundary / 2.f, 0 }
    {}

    bool force_refresh{ false };

    pong::CollisionEvent update(float dt) {
        pad_left.update(dt, input_left, pad_max_speed, pad_boundary);
        pad_right.update(dt, input_right, pad_max_speed, pad_boundary);
        return ball.update(dt, pad_left.y, pad_right.y, { boundaries_x, boundaries_y }, pad_padding, pad_height, pad_width, ball_radius);
    }
};

struct RoomState : public State<RoomState, user_t> {
    RoomState(MainLobbyState& _main_lobby) : State({

        // Receive
        { id_of(pong::packet::client::Input{}), &RoomState::on_input },
        { id_of(pong::packet::client::Abandon{}), &RoomState::on_abandon },
        { id_of(pong::packet::client::EnterQueue{}), &RoomState::on_enter_queue },
        { id_of(pong::packet::client::LeaveQueue{}), &RoomState::on_leave_queue },
        { id_of(pong::packet::client::LeaveRoom{}), &RoomState::on_leave_room }


    }), main_lobby{ _main_lobby }, left_player{ invalid_user_id }, right_player{ invalid_user_id }, score{0, 0} {}

    /*
        [CLIENT] Send LeaveRoom, EnterQueue, LeaveQueue
        [SERVER] Send GameState, NewUser, OldUser, NewPlayer, OldPlayer, BePlayer 
    */

    /*
        [CLIENT] Send Input, Abandon
        [SERVER] Send GameState, NewUser, OldUser, NewPlayer, OldPlayer 
    */

    MainLobbyState& main_lobby;
    user_id_t left_player;
    user_id_t right_player;

    std::deque<user_id_t> queue;

    float time;
    static constexpr float game_state_packet_interval = 0.1; 

    Game game;
    pong::packet::server::Score score;


    void update_players() {
        if (queue.empty()) return;
        if (left_player == invalid_user_id) {
            auto id = queue.back(); queue.pop_back();
            left_player = id;
            std::cout << "! Add left player(ID#" << id << ")\n";

            auto handle = get_user_handle(id);

            std::cout << "Send NewPlayer\n";
            broadcast_other(handle, pong::packet::server::NewPlayer{
                pong::Side::Left,
                get_user_data(handle)
            });
            std::cout << "Send BePlayer\n";
            send(handle, pong::packet::server::BePlayer{
                pong::Side::Left
            });

            game = Game{};
            score = { 0, 0 };
            
            if (queue.empty()) return;
        }


        if (right_player == invalid_user_id) {
            auto id = queue.back(); queue.pop_back();
            right_player = id;
            std::cout << "! Add right player(ID#" << id << ")\n";

            auto handle = get_user_handle(id);

            std::cout << "Send NewPlayer\n";
            broadcast_other(handle, pong::packet::server::NewPlayer{
                pong::Side::Right,
                get_user_data(handle)
            });
            std::cout << "Send BePlayer\n";
            send(handle, pong::packet::server::BePlayer{
                pong::Side::Right
            });

            game = Game{};
            score = { 0, 0 };
        }
    }


    void update_game(float dt) {
        if (left_player != invalid_user_id && right_player != invalid_user_id) {
            auto event = game.update(dt);
            if (event == pong::CollisionEvent::LeftBoundary) {
                ++score.left;
                broadcast(score);
                game = Game{};

                // force sending packet GameState
                time = game_state_packet_interval;
            }
            else if (event == pong::CollisionEvent::RightBoundary) {
                ++score.right;
                broadcast(score);
                game = Game{};

                // force sending packet GameState
                time = game_state_packet_interval;
            }

            time += dt;

            if (time > game_state_packet_interval) {
                time -= game_state_packet_interval;
                broadcast(pong::packet::server::GameState {
                    game.ball,
                    game.pad_left,
                    game.pad_right
                });
            }
        }
    }


    Action on_abandon(user_handle_t handle, packet_t) {
        auto id = get_user_id(handle);

        if (id == left_player) {
            std::cout << "Send OldPlayer\n";
            broadcast_other(handle, pong::packet::server::OldPlayer{
                pong::Side::Left,
                get_user_data(handle)
            });
            std::cout << "Send NewUser\n";
            broadcast_other(handle, pong::packet::server::NewUser{
                get_user_data(handle)
            });
            left_player = invalid_user_id;
            std::cout << "! Remove left player\n";
            update_players();
        }
        else if (id == right_player) {
            std::cout << "Send OldPlayer\n";
            broadcast_other(handle, pong::packet::server::OldPlayer{
                pong::Side::Right,
                get_user_data(handle)
            });
            std::cout << "Send NewUser\n";
            broadcast_other(handle, pong::packet::NewUser{
                get_user_data(handle)
            });
            right_player = invalid_user_id;
            std::cout << "! Remove right player\n";
            update_players();
        }
        else {
            std::cerr << "[Warning] Received PacketID::Abandon from a spectator\n";
        }

        return Idle{};
    }


    Action on_enter_queue(user_handle_t handle, packet_t) {
        auto id = get_user_id(handle);

        if (id == left_player || id == right_player) {
            std::cerr << "[Warning] Received PacketID::EnterQueue from a player\n";
        } else {
            queue.push_front(id);
            update_players();
        }

        return Idle{};
    }


    Action on_leave_queue(user_handle_t handle, packet_t) {
        auto id = get_user_id(handle);

        if (id == left_player || id == right_player) {
            std::cerr << "[Warning] Received PacketID::LeaveQueue from a player\n";
        } else {
            queue.erase(std::remove(std::begin(queue), std::end(queue), id), std::end(queue));
        }

        return Idle{};
    }


    Action on_input(user_handle_t handle, packet_t packet) {
        auto id = get_user_id(handle);

        std::cout << "! Recieved input from handle ID#" << id << '\n';
        if (id == left_player) {
            std::cout << "Received Left input\n";
            auto input = from_packet<pong::packet::client::Input>(packet);
            game.input_left = input.input;
        }
        else if (id == right_player) {
            std::cout << "Received Right input\n";
            auto input = from_packet<pong::packet::client::Input>(packet);
            game.input_right = input.input;
        }
        else {
            std::cerr << "[Warning] Received PacketID::Input from a spectator\n";
        }

        return Idle{};
    }


    Action on_leave_room(user_handle_t handle, packet_t) {
        return order_change_state(main_lobby, handle, get_user_data(handle));
    }


    void on_user_enter(user_handle_t handle) {
        std::cout << "Send NewUser\n";
        broadcast_other(handle, pong::packet::server::NewUser{
            get_user_data(handle)
        });


        std::vector<std::string> spectators;
        spectators.reserve(number_of_user());


        for(user_handle_t h{ 0 }; h < number_of_user(); ++h) {
            std::cout << "### Check handle #" << h << '\n';
            std::cout << is_valid(h) << ", " << (get_user_id(h) != left_player) << ", " << (get_user_id(h) != right_player) << ", " << (h != handle) << '\n';
            if (is_valid(h) && get_user_id(h) != left_player && get_user_id(h) != right_player && h != handle) {
                std::cout << "Push " << get_user_data(h) << '\n';
                spectators.push_back(get_user_data(h));
            }
        }


        std::cout << "Send RoomInfo\n";
        auto left_player_handle = get_user_handle(left_player);
        auto right_player_handle = get_user_handle(right_player);
        send(handle, pong::packet::server::RoomInfo{
            is_valid(left_player_handle) ? get_user_data(left_player_handle) : "",
            is_valid(right_player_handle) ? get_user_data(right_player_handle) : "",
            std::move(spectators)
        });
    }


    void on_user_leave(user_handle_t handle) {
        auto id = get_user_id(handle);

        if (id == left_player) {
            std::cout << "! Remove left player\n";
            left_player = invalid_user_id;
            update_players();
            std::cout << "Send OldPlayer Left\n";
            broadcast_other(handle, pong::packet::server::OldPlayer{
                pong::Side::Left,
                get_user_data(handle)
            });
        }
        else if (id == right_player) {
            std::cout << "! Remove right player\n";
            right_player = invalid_user_id;
            update_players();
            std::cout << "Send OldPlayer Right\n";
            broadcast_other(handle, pong::packet::server::OldPlayer{
                pong::Side::Right,
                get_user_data(handle)
            });
        } else {
            queue.erase(std::remove(std::begin(queue), std::end(queue), id), std::end(queue));
        }

        std::cout << "Send OldUser\n";
        broadcast_other(handle, pong::packet::server::OldUser{
            get_user_data(handle)
        });
    }
};

}