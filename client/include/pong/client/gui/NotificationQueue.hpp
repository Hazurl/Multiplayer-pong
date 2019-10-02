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

    NotificationQueue(sf::Font const& _font, float _max_lifetime) : font{ _font }, max_lifetime(_max_lifetime) {}

    void push(sf::String text) {
        auto id = next_id++;
        auto callback = [this, id] {
            auto it = std::find_if(std::begin(notifications), std::end(notifications), [id] (auto& n) {
                return n.id == id;
            });
            assert(it != std::end(notifications));
            notifications.erase(it);
        };

        auto const colors = Button::Customization();

        notifications.push_back({ Button{ callback, font, std::move(text), 15, 5, colors }, max_lifetime, id });
    }

    void update(float dt) {
        for(auto& n : notifications) {
            n.lifetime -= dt; 
            n.button.update(dt);
        }

        while(!notifications.empty() && notifications.front().lifetime <= 0.0f) {
            notifications.pop_front();
        }

        float y{ 600 /* window's height */ - 10 - 15 - 5*2 };
        for(auto it = std::rbegin(notifications); it != std::rend(notifications); ++it) {
            auto& b = it->button;
            b.setPosition(300, y);
            y -= 5 * 2 + 15 + 5;
        }
    }

    sftk::PropagateEvent on_mouse_button_pressed(sf::Window&, sf::Event::MouseButtonEvent const& b) override {
        return std::all_of(std::begin(notifications), std::end(notifications), [b] (auto& n) {
            return n.button.on_mouse_button_pressed(b);
        });
    }

    sftk::PropagateEvent on_mouse_button_released(sf::Window&, sf::Event::MouseButtonEvent const& b) override {
        return std::all_of(std::begin(notifications), std::end(notifications), [b] (auto& n) {
            return n.button.on_mouse_button_released(b);
        });
    }

    sftk::PropagateEvent on_mouse_moved(sf::Window&, sf::Event::MouseMoveEvent const& b) override {
        return std::all_of(std::begin(notifications), std::end(notifications), [b] (auto& n) {
            return n.button.on_mouse_moved(b);
        });
    }

    void draw(sf::RenderTarget &target, sf::RenderStates state) const override {
        for(auto& n : notifications) {
            target.draw(n.button, state);
        }
    }

private:

    sf::Font const& font;
    float const max_lifetime;
    std::deque<Notification> notifications;
    std::size_t next_id{ 0 };

};

}