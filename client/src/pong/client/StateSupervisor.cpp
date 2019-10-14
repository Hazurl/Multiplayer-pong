#include <pong/client/StateSupervisor.hpp>

#include <pong/client/Visitor.hpp>
#include <pong/client/state/State.hpp>

namespace pong::client {

void StateSupervisor::loop() {
    sf::Clock clock;

    while(window.isOpen()) {

        connection.check_connection();
        process_events(clock.restart().asSeconds());
        update_gui();
        draw();

    }
}

void StateSupervisor::load_fonts() {
    if (!font.loadFromFile("../assets/neoletters.ttf")) {
        ERROR("Couldn't load font '../assets/neoletters.ttf'");
        throw std::runtime_error("Couldn't load font '../assets/neoletters.ttf'");
    }
}

Application StateSupervisor::make_application() {
    return Application(window_properties, connection, gui, font);
}

std::vector<WindowEvent> StateSupervisor::poll_window_events() {
    std::vector<WindowEvent> window_events;

    sf::Event event;
    while (window.pollEvent(event)) {
        // Internal events
        switch(event.type) {
            case sf::Event::Closed: {
                window.close();
                break;
            }
            case sf::Event::Resized: {
                sf::View view({0, 0, event.size.width, event.size.height});
                window.setView(view);

                gui.set_property(window_properties.width(), event.size.width);
                gui.set_property(window_properties.height(), event.size.height);
                break;
            }
            default: { break; }
        }

        // Dispatch to state
        switch(event.type) {
            case sf::Event::Resized: {
                window_events.emplace_back(Resized{ event.size });
                break;
            }
            case sf::Event::TextEntered: {
                window_events.emplace_back(TextEntered{ event.text });
                break;
            }
            case sf::Event::KeyPressed: {
                window_events.emplace_back(KeyPressed{ event.key });
                break;
            }
            case sf::Event::KeyReleased: {
                window_events.emplace_back(KeyReleased{ event.key });
                break;
            }
            case sf::Event::MouseButtonPressed: {
                window_events.emplace_back(MouseButtonPressed{ event.mouseButton });
                break;
            }
            case sf::Event::MouseButtonReleased: {
                window_events.emplace_back(MouseButtonReleased{ event.mouseButton });
                break;
            }
            case sf::Event::MouseMoved: {
                window_events.emplace_back(MouseMoved{ event.mouseMove });
                break;
            }
            case sf::Event::MouseWheelScrolled: {
                window_events.emplace_back(MouseWheelScrolled{ event.mouseWheelScroll });
                break;
            }
            default: { break; }
        }
    }

    return window_events;
}

template<typename F, typename G>
std::vector<pong::packet::GamePacket> accumulate_packets(net::Connection& connection, F&& f, G&& g) {
    if (!connection.is_connected()) {
        return {};
    }


    std::vector<pong::packet::GamePacket> packets;
    bool is_done = false;
    sf::TcpSocket& socket = *connection.get_socket();

    while(!is_done && !g()) {
        auto&&[status, packet] = f(socket);


        if (packet) {
            packets.emplace_back(*packet);
        }


        switch(status) {
            case net::Status::Error: {
                ERROR("Error when using the socket, abording...");
                throw std::runtime_error("Error when using the socket, abording...");
            }

            case net::Status::Done: {
                is_done = true;
                break;
            }

            default: {
                break;
            }
        }
    }

    return packets;
}


std::vector<pong::packet::GamePacket> StateSupervisor::send_packets() {
    return accumulate_packets(connection, [this] (auto& socket) {
        return packet_queue.send(socket);
    },
    [this] {
        return packet_queue.empty();
    });
}

std::vector<pong::packet::GamePacket> StateSupervisor::receive_packets() {
    return accumulate_packets(connection, net::receive, [] { return false; });
}

void StateSupervisor::process_events(float dt) {
    auto app = make_application();

    for(auto window_event : poll_window_events()) {
        auto actions = state->on_window_event(app, window_event);
        process_actions(actions);
    }

    for(auto network_event : send_packets()) {
        auto actions = state->on_send(app, network_event);
        process_actions(actions);
    }

    for(auto& network_event : receive_packets()) {
        auto actions = state->on_receive(app, network_event);
        process_actions(actions);
    }

    {
        auto actions = state->on_update(app, dt);
        process_actions(actions);
    }
}

void StateSupervisor::process_actions(action::Actions& actions) {
    for(auto& action : actions) {
        process_action(action);
    }
}

void StateSupervisor::process_action(action::Action& action) {
    std::visit(Visitor{
        [this] (action::ChangeState&& cs) {
            state = std::move(cs);
        },
        [this] (action::Quit const&) {
            window.close();
        },
        [this] (action::Send const& s) {
            packet_queue.push(s);
        },
        [this] (action::Connect const& c) {
            connection.attempt_to_connect(c.addr, c.port, c.timeout);
        },
        [this] (action::Disconnect const&) {
            connection.stop_connection();
        }
    }, std::move(action));
}

void StateSupervisor::update_gui() {
    state->notify_gui(gui);

    if (!gui.is_up_to_date() && !gui.compute_order()) {
        ERROR("Circular dependencies in the GUI");
        throw std::runtime_error("Circular dependencies in the GUI");
    }

    gui.update_properties();

    state->update_properties(gui);
}

void StateSupervisor::draw() {
    window.clear(sf::Color{ 0x3A, 0x3C, 0x46 });

    window.draw(*state);

    window.display();
}


}