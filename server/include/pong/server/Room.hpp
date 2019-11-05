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
    RoomState(MainLobbyState& _main_lobby) 
    : State({

            // Receive
            { id_of(pong::packet::client::Input{}), &RoomState::on_input },
            { id_of(pong::packet::client::Abandon{}), &RoomState::on_abandon },
            { id_of(pong::packet::client::EnterQueue{}), &RoomState::on_enter_queue },
            { id_of(pong::packet::client::LeaveQueue{}), &RoomState::on_leave_queue },
            { id_of(pong::packet::client::LeaveRoom{}), &RoomState::on_leave_room },
            { id_of(pong::packet::client::AcceptBePlayer{}), &RoomState::on_accept_be_player }

        })
    ,   main_lobby{ _main_lobby }
    ,   left_player{ invalid_user_id }
    ,   right_player{ invalid_user_id }
    ,   next_player_left{ invalid_user_id }
    ,   next_player_right{ invalid_user_id }
    ,   score{0, 0} {}






    MainLobbyState& main_lobby;
    user_id_t left_player;
    user_id_t right_player;

    std::deque<user_id_t> queue;

    user_id_t next_player_left;
    float next_player_left_timer;

    user_id_t next_player_right;
    float next_player_right_timer;

    float time;
    static constexpr float game_state_packet_interval = 1.f / (32 /* ticks */); 
    static constexpr float next_player_max_timer = 5.f /* seconds */;

    Game game;
    pong::packet::server::Score score;






    void update_players() {

        if (!queue.empty() && left_player == invalid_user_id && next_player_left == invalid_user_id) {
            auto id = queue.back(); queue.pop_back();
            next_player_left = id;
            next_player_left_timer = next_player_max_timer;
            std::cout << "! Try add left player(ID#" << id << ")\n";

            auto handle = get_user_handle(id);
/*
            std::cout << "Send NewPlayer\n";
            broadcast_other(handle, pong::packet::server::NewPlayer{
                pong::Side::Left,
                get_user_data(handle)
            });
    
            std::cout << "Send BePlayer\n";
            send(handle, pong::packet::server::BePlayer{
                pong::Side::Left
            });
*/

            std::cout << "Send BeNextPlayer\n";
            send(handle, pong::packet::server::BeNextPlayer{});
        }


        if (!queue.empty() && right_player == invalid_user_id && next_player_right == invalid_user_id) {
            auto id = queue.back(); queue.pop_back();
            next_player_right = id;
            next_player_right_timer = next_player_max_timer;
            std::cout << "! Try add right player(ID#" << id << ")\n";

            auto handle = get_user_handle(id);
/*
            std::cout << "Send NewPlayer\n";
            broadcast_other(handle, pong::packet::server::NewPlayer{
                pong::Side::Right,
                get_user_data(handle)
            });
    
            std::cout << "Send BePlayer\n";
            send(handle, pong::packet::server::BePlayer{
                pong::Side::Right
            });
*/

            std::cout << "Send BeNextPlayer\n";
            send(handle, pong::packet::server::BeNextPlayer{});
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

        if (next_player_left != invalid_user_id) {
            next_player_left_timer -= dt;
            if (next_player_left_timer < 0) {
                auto handle = get_user_handle(next_player_left);
                std::cout << "Send DeniedBePlayer\n";
                send(handle, packet::server::DeniedBePlayer{});
                next_player_left = invalid_user_id;
                update_players();
            }
        }

        if (next_player_right != invalid_user_id) {
            next_player_right_timer -= dt;
            if (next_player_right_timer < 0) {
                auto handle = get_user_handle(next_player_right);
                std::cout << "Send DeniedBePlayer\n";
                send(handle, packet::server::DeniedBePlayer{});
                next_player_right = invalid_user_id;
                update_players();
            }
        }
    }


    Action on_abandon(user_handle_t handle, packet_t) {
        auto id = get_user_id(handle);

        if (id == left_player) {

            std::cout << "Send Game Over\n";
            broadcast(pong::packet::server::GameOver{
                packet::server::GameOver::Result::LeftAbandon
            });

            std::cout << "Send OldPlayer\n";
            broadcast_other(handle, pong::packet::server::OldPlayer{
                pong::Side::Left,
                get_user_data(handle)
            });

            std::cout << "! Remove left player\n";
            left_player = invalid_user_id;

            if (right_player != invalid_user_id) {
                std::cout << "Send OldPlayer\n";
                auto right_handle = get_user_handle(right_player);
                broadcast_other(right_handle, pong::packet::server::OldPlayer{
                    pong::Side::Right,
                    get_user_data(right_handle)
                });

                std::cout << "! Put right player in queue\n";
                queue.push_front(right_player);
                right_player = invalid_user_id;
            }

            update_players();
        }
        else if (id == right_player) {
            std::cout << "Send Game Over\n";
            broadcast(pong::packet::server::GameOver{
                packet::server::GameOver::Result::RightAbandon
            });

            std::cout << "Send OldPlayer\n";
            broadcast_other(handle, pong::packet::server::OldPlayer{
                pong::Side::Right,
                get_user_data(handle)
            });

            std::cout << "! Remove right player\n";
            right_player = invalid_user_id;

            if (left_player != invalid_user_id) {
                std::cout << "Send OldPlayer\n";
                auto left_handle = get_user_handle(left_player);
                broadcast_other(left_handle, pong::packet::server::OldPlayer{
                    pong::Side::Left,
                    get_user_data(left_handle)
                });

                std::cout << "! Put left player in queue\n";
                queue.push_front(left_player);
                left_player = invalid_user_id;
            }

            update_players();
        }
        else {
            std::cerr << "[Warning] Received PacketID::Abandon from a spectator\n";
        }

        return Idle{};
    }


    Action on_enter_queue(user_handle_t handle, packet_t) {
        auto id = get_user_id(handle);

        if (id == left_player || id == right_player || id == next_player_left || id == next_player_right) {
            std::cerr << "[Warning] Received PacketID::EnterQueue from a [next] player\n";
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
            if (id == next_player_left) {
                next_player_left = invalid_user_id;
            }

            else if (id == next_player_right) {
                next_player_right = invalid_user_id;
            }

            queue.erase(std::remove(std::begin(queue), std::end(queue), id), std::end(queue));
            update_players();
        }

        return Idle{};
    }


    Action on_input(user_handle_t handle, packet_t packet) {
        auto id = get_user_id(handle);

        std::cout << "! Received input from handle ID#" << id << '\n';
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
        std::cout << "Send Valid LeaveRoomResponse\n";
        send(handle, packet::server::LeaveRoomResponse{ packet::server::LeaveRoomResponse::Reason::Okay });
        return order_change_state(main_lobby, handle, get_user_data(handle));
    }


    Action on_accept_be_player(user_handle_t handle, packet_t) {
        auto id = get_user_id(handle);
        if (id == next_player_left) {
            next_player_left = invalid_user_id;
            left_player = id;

            std::cout << "Send NewPlayer\n";
            broadcast_other(handle, pong::packet::server::NewPlayer{
                pong::Side::Left,
                get_user_data(handle)
            });
    
            std::cout << "Send BePlayer\n";
            send(handle, pong::packet::server::BePlayer{
                pong::Side::Left
            });

            if (right_player != invalid_user_id) {
                std::cout << "Start Game !\n";
                game = Game{};
                score = {0, 0};
            }
        }  
        
        else if (id == next_player_right) {
            next_player_right = invalid_user_id;
            right_player = id;

            std::cout << "Send NewPlayer\n";
            broadcast_other(handle, pong::packet::server::NewPlayer{
                pong::Side::Right,
                get_user_data(handle)
            });
    
            std::cout << "Send BePlayer\n";
            send(handle, pong::packet::server::BePlayer{
                pong::Side::Right
            });

            if (left_player != invalid_user_id) {
                std::cout << "Start Game !\n";
                game = Game{};
                score = {0, 0};
            }
        } 
        
        else {
            std::cerr << "[Warning] Received PacketID::AcceptBePlayer not from a next player\n";
        }
        return Idle{};
    }


    void on_user_enter(user_handle_t handle) {
        std::cout << "Send NewUser\n";
        broadcast_other(handle, pong::packet::server::NewUser{
            get_user_data(handle)
        });


        std::vector<std::string> spectators;
        spectators.reserve(number_of_user());


        for(user_handle_t h{ 0 }; h < number_of_user(); ++h) {
            if (is_valid(h) && get_user_id(h) != left_player && get_user_id(h) != right_player && h != handle) {
                std::cout << "Push " << get_user_data(h) << '\n';
                spectators.push_back(get_user_data(h));
            }
        }


        std::cout << "Send RoomInfo with " << spectators.size() << " spectators\n";
        auto left_player_handle = get_user_handle(left_player);
        auto right_player_handle = get_user_handle(right_player);
        send(handle, pong::packet::server::RoomInfo{
            is_valid(left_player_handle) ? get_user_data(left_player_handle) : "",
            is_valid(right_player_handle) ? get_user_data(right_player_handle) : "",
            std::move(spectators)
        });
        send(handle, score);
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
            if (id == next_player_left) {
                next_player_left = invalid_user_id;
            }

            else if (id == next_player_right) {
                next_player_right = invalid_user_id;
            }
            
            queue.erase(std::remove(std::begin(queue), std::end(queue), id), std::end(queue));
            update_players();
        }

        std::cout << "Send OldUser\n";
        broadcast_other(handle, pong::packet::server::OldUser{
            get_user_data(handle)
        });
    }
};

}