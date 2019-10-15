#include <deque>
#include <cassert>

#include <SFML/Graphics.hpp>

#include <pong/client/gui/Button.hpp>
#include <pong/client/gui/NotificationQueue.hpp>

#include <sftk/eventListener/EventListener.hpp>


namespace pong::client::gui {
    
NotificationQueue::NotificationQueue(Gui<>& _gui, sf::Font const& _font, float _max_lifetime) : gui{ _gui }, font{ _font }, max_lifetime(_max_lifetime) {}

void NotificationQueue::push(sf::String text) {
    auto id = next_id++;
    auto callback = [this, id] {
        auto it = std::find_if(std::begin(notifications), std::end(notifications), [id] (auto& n) {
            return n.id == id;
        });
        assert(it != std::end(notifications));
        notifications.erase(it);
    };

    auto const theme = Button::Theme();

    notifications.push_back({ Button{ Allocator{ gui }, callback, theme }, max_lifetime, id });
    auto& button = notifications.back().button;
    button.setSize({ 50, 20 });
    button.set_rounded_radius(3);
    button.set_rounded_precision(2);
}

void NotificationQueue::update(float dt) {
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

sftk::PropagateEvent NotificationQueue::on_mouse_button_pressed(sf::Window& w, sf::Event::MouseButtonEvent const& b) {
    return std::all_of(std::begin(notifications), std::end(notifications), [&w, b] (auto& n) {
        return n.button.on_mouse_button_pressed(w, b);
    });
}

sftk::PropagateEvent NotificationQueue::on_mouse_button_released(sf::Window& w, sf::Event::MouseButtonEvent const& b) {
    return std::all_of(std::begin(notifications), std::end(notifications), [&w, b] (auto& n) {
        return n.button.on_mouse_button_released(w, b);
    });
}

sftk::PropagateEvent NotificationQueue::on_mouse_moved(sf::Window& w, sf::Event::MouseMoveEvent const& b) {
    return std::all_of(std::begin(notifications), std::end(notifications), [&w, b] (auto& n) {
        return n.button.on_mouse_moved(w, b);
    });
}

void NotificationQueue::draw(sf::RenderTarget &target, sf::RenderStates state) const {
    for(auto& n : notifications) {
        target.draw(n.button, state);
    }
}


}