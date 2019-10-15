#pragma once

#include <pong/client/net/PacketQueue.hpp>
#include <pong/client/net/Receiver.hpp>
#include <pong/client/net/Status.hpp>
#include <pong/client/net/Connection.hpp>

#include <pong/client/Application.hpp>
#include <pong/client/Action.hpp>
#include <pong/client/Logger.hpp>
#include <pong/client/WindowEvent.hpp>

#include <pong/client/state/State.hpp>

#include <pong/client/gui/constraint/Interface.hpp>

#include <multipong/Packets.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <memory>
#include <vector>

namespace pong::client {

class StateSupervisor {
public:

    template<typename S>
    struct Tag{};

    template<typename InitialState, typename...Args>
    StateSupervisor(Tag<InitialState>, sf::VideoMode video_mode, sf::String const& title, Args&&...args) 
    :   window(video_mode, title)
    ,   connection{}
    ,   packet_queue{}
    ,   gui{}
    ,   window_properties(gui)
    ,   font{}
    ,   state{ nullptr }
    {
        auto window_size = window.getSize();
        gui.set_property(window_properties.left(), 0);
        gui.set_property(window_properties.top(), 0);
        gui.set_property(window_properties.width(), window_size.x);
        gui.set_property(window_properties.height(), window_size.y);

        load_fonts();

        auto app = make_application();
        state = std::make_unique<InitialState>(app, std::forward<Args>(args)...);
    }

    void loop();

private:

    void load_fonts();

    Application make_application();
    
    std::vector<WindowEvent> poll_window_events();
    std::vector<pong::packet::GamePacket> send_packets();
    std::vector<pong::packet::GamePacket> receive_packets();

    void process_events(float dt);
    void process_actions(action::Actions actions);
    void process_action(action::Action action);

    void update_gui();
    void draw();


    sf::RenderWindow window;

    net::Connection connection;
    net::PacketQueue packet_queue;

    gui::Gui<> gui;
    gui::RectProperties window_properties;

    sf::Font font;

    std::unique_ptr<state::State> state;

};

template<typename InitialState, typename...Args>
StateSupervisor make_supervisor(sf::VideoMode video_mode, sf::String const& title, Args&&...args) {
    return StateSupervisor(StateSupervisor::Tag<InitialState>{}, video_mode, title, std::forward<Args>(args)...);
}


}