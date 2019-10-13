#pragma once

#include <deque>
#include <cassert>

#include <SFML/Graphics.hpp>

#include <pong/client/gui/Button.hpp>

#include <sftk/eventListener/EventListener.hpp>


namespace pong::client::gui {
    
struct NotificationQueue: sf::Drawable, sftk::EventListener {
private:

    struct Notification {
        Button button;
        float lifetime;
        std::size_t id;
    };

public:

    NotificationQueue(Gui<>& _gui, sf::Font const& _font, float _max_lifetime);


    void push(sf::String text);


    void update(float dt);


    sftk::PropagateEvent on_mouse_button_pressed(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_button_released(sf::Window&, sf::Event::MouseButtonEvent const& b) override;
    sftk::PropagateEvent on_mouse_moved(sf::Window&, sf::Event::MouseMoveEvent const& b) override;


    void draw(sf::RenderTarget &target, sf::RenderStates state) const override;

private:

    Gui<>& gui;
    sf::Font const& font;
    float const max_lifetime;
    std::deque<Notification> notifications;
    std::size_t next_id{ 0 };

};

}