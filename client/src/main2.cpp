#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <optional>
#include <future>
#include <chrono>
#include <deque>

#include <multipong/Game.hpp>
#include <multipong/Packets.hpp>

#include <pong/client/LoginIn.hpp>
#include <pong/client/State.hpp>

#include <pong/client/gui/NotificationQueue.hpp>

#include <pong/client/gui/Constraint/Constraint.hpp>

struct Options {
    std::optional<std::string> username;
    std::optional<int> port;
};

enum class OptionState {
    Username,
    Port,
    State
};

Options parse_arguments(int argc, char** argv) {
    Options options;
    auto state = OptionState::State;

    for(int i{ 1 }; i < argc; ++i) {
        std::string arg{ argv[i] };
        switch(state) {
            case OptionState::State: {
                if (arg == "-p" || arg == "--port") {
                    state = OptionState::Port;
                } 
                else if (arg == "-u" || arg == "--username") {
                    state = OptionState::Username;
                } else {
                    std::cout << "Unknown command\n";
                    std::cout << "USAGE: ./client [(-p | --port) <port_number>] (-u | --username) <username>\n";
                    std::exit(1);
                }
                break;
            }
            case OptionState::Username: {
                options.username = arg;
                state = OptionState::State;
                break;
            }
            case OptionState::Port: {
                try {
                    options.port = std::stoi(arg);
                } catch(...) {
                    std::cout << "Unknown arguments\n";
                    std::cout << "USAGE: ./client [(-p | --port) <port_number>] (-u | --username) <username>\n";
                    std::exit(1);
                }
                state = OptionState::State;
                break;
            }
        }
    }

    if (state != OptionState::State) {
        std::cout << "Unknown command\n";
        std::cout << "USAGE: ./client [(-p | --port) <port_number>] (-u | --username) <username>\n";
        std::exit(1);
    }

    if (!options.username || options.username->empty()) {
        std::cout << "You must enter your username\n";
        std::cout << "USAGE: ./client [(-p | --port) <port_number>] (-u | --username) <username>\n";
        std::exit(1);
    }

    return options;

}

template<typename T>
bool is_future_ready(std::future<T> const& f) { 
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

int main(int argc, char** argv) {
    int error_code{ 0 };

    auto options = parse_arguments(argc, argv);

    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Multiplayer pong");

    sf::Font font;
    if (!font.loadFromFile("../assets/neoletters.ttf")) {
        std::cout << "Couldn't load font '../assets/neoletters.ttf'\n";
        std::exit(1);
    }/*
    if (!font.loadFromFile("../assets/Roboto-Regular.ttf")) {
        std::cout << "Couldn't load font '../assets/Roboto-Regular.ttf'\n";
        std::exit(1);
    }*/

    sf::Clock clock;

    std::unique_ptr<sf::TcpSocket> socket{ nullptr };
    std::future<std::unique_ptr<sf::TcpSocket>> future_socket;

    std::unique_ptr<pong::client::StateBase> state{ nullptr };

    pong::client::gui::NotificationQueue notification_queue{ font, 5 };
    int i{ 0 };

    future_socket = std::async(std::launch::async, [] (int port) -> std::unique_ptr<sf::TcpSocket> {
        auto socket_ptr = std::make_unique<sf::TcpSocket>();
        if (socket_ptr->connect("127.0.0.1", port) != sf::Socket::Done) {
            return nullptr;
        }
        socket_ptr->setBlocking(false);
        return socket_ptr;
    }, options.port ? *options.port : 48624);

    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            bool propagate = sftk::dispatch(window, event, notification_queue);
            if (propagate && state) {
                sftk::dispatch(window, event, *state);
            }

            switch(event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }
                case sf::Event::KeyPressed: {
                    if (event.key.code == sf::Keyboard::Space) {
                        notification_queue.push("Something #" + std::to_string(i++));
                    }
                }
                default: { break; }
            }
        }

        if (!state) {
            if (is_future_ready(future_socket)) {
                std::unique_ptr<sf::TcpSocket> res = future_socket.get();
                if (!res) {
                    window.close();
                    std::cerr << "Couldn't connect to server... Abording...\n";
                    error_code = 1;
                }

                socket = std::move(res);
                state = std::make_unique<pong::client::LoginIn>(*socket, options.username.value_or(""), font);
            }
        } else {
            {
                auto res = state->send_all();
                if (std::holds_alternative<pong::client::Abord>(res)) {
                    window.close();
                    std::cerr << "Abording after receiving...\n";
                    error_code = 1;
                    break;
                } else if (std::holds_alternative<pong::client::ChangeState>(res)) {
                    state = std::get<pong::client::ChangeState>(res)(*state);
                }
            }
            {
                auto res = state->receive_all();
                if (std::holds_alternative<pong::client::Abord>(res)) {
                    window.close();
                    std::cerr << "Abording after receiving...\n";
                    error_code = 1;
                } else if (std::holds_alternative<pong::client::ChangeState>(res)) {
                    state = std::get<pong::client::ChangeState>(res)(*state);
                } // else Idle
            }
        }

        float dt = clock.restart().asSeconds();
        window.clear(sf::Color::Black);
        if (state) {
            state->update(dt);
            window.draw(*state);
        }
        notification_queue.update(dt);
        window.draw(notification_queue);
        window.display();
    }

    return error_code;
}