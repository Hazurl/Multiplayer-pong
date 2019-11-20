#pragma once

#include <cassert>
#include <vector>
#include <variant>
#include <optional>

#include <SFML/Graphics.hpp>

#include <pong/client/gui/Button.hpp>
#include <pong/client/gui/Text.hpp>
#include <pong/client/gui/Sprite.hpp>

#include <pong/client/Visitor.hpp>
#include <pong/client/Application.hpp>
#include <pong/client/Logger.hpp>

#include <sftk/eventListener/EventListener.hpp>


namespace pong::client::notif {

struct NoButton {};
struct RefreshButton {};
struct GenericButton {
    sf::String message;
};

using Button = std::variant<
    NoButton,
    RefreshButton,
    GenericButton
>;

struct Lifetime {
    std::optional<float> close_after;
    bool close_on_click;
};


struct Notification {
    enum class Category {
        Error,
        Gameplay
    };

    Notification(sf::String message_, Category category_ = Category::Gameplay, Lifetime lifetime_ = Lifetime{ 5.f, true }, Button button_ = NoButton{});

    sf::String message;
    Category category;
    Lifetime lifetime;
    Button button;

    inline Notification& closing_after(float t) { lifetime.close_after = t; return *this; }
    inline Notification& with_no_timeout() { lifetime.close_after = std::nullopt; return *this; }

    inline Notification& with_no_manual_closing() { lifetime.close_on_click = true; return *this; }
    inline Notification& with_manual_closing() { lifetime.close_on_click = false; return *this; }

    inline Notification& with_no_button() { button = NoButton{}; return *this; }
    inline Notification& refreshable() { button = RefreshButton{}; return *this; }
    inline Notification& with_button(sf::String button_message) { button = GenericButton{ std::move(button_message) }; return *this; }

    inline Notification& with_category(Category category_) { category = category_; return *this; }

};







class NotificationMesh : public sf::Drawable, public sf::Transformable {
public:

    enum class Event {
        Background, Button, Refresh
    };

    NotificationMesh(gui::Allocator<> gui, Notification const& notification, sf::Texture const& refresh_texture, sf::Font const& font);

    void update(float dt);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void notify_gui(gui::Gui<>& gui) const;
    void update_properties(gui::Gui<> const& gui);
    void free_properties(gui::Allocator<> gui) const;

    std::optional<Event> on_click(sf::Vector2f const& mouse);
    std::optional<Event> on_hover(sf::Vector2f const& mouse);
    std::optional<Event> on_release_click(sf::Vector2f const& mouse);

private:

    struct ButtonMesh {
        gui::Button button;
        gui::Text label;
    };

    struct RefreshMesh {
        gui::Button button;
        gui::Sprite sprite;
    };

    struct None {};

    gui::Button background;
    gui::Text message;
    std::variant<None, ButtonMesh, RefreshMesh> button_mesh;
};




class Controller;

class Queue {
private:

    static unsigned long next_id;

    struct Object {
        unsigned long id;

        Lifetime lifetime;
        bool has_button;

        bool is_clicked;
        bool is_button_clicked;

        NotificationMesh mesh;

        sftk::Animated<float, sftk::interpolation::Bezier<float>> target_x;
        sftk::Animated<float> target_y;
    };

public:

    Queue();

    void update_animations(Application app, float dt);

    void on_click(sf::Vector2f const& mouse);
    void on_hover(sf::Vector2f const& mouse);
    void on_release_click(sf::Vector2f const& mouse);

    void push(Application app, Notification notification);

    void draw(sf::RenderTarget& target) const;

    void notify_gui(gui::Gui<>& gui) const;
    void update_properties(gui::Gui<> const& gui);
    void free_properties(gui::Allocator<> gui) const;

    Controller make_controller();

private:

    friend Controller;

    std::vector<Object> notifications;
    sf::Texture refresh_texture;

};





class Controller {
private:

    friend Queue;

    std::vector<Queue::Object>* notifications;
    Controller(std::vector<Queue::Object>& notifications_);

public:


    bool is_clicked(unsigned long id) const;
    bool is_refresh_button_clicked(unsigned long id) const;
    bool is_button_clicked(unsigned long id) const;
    bool is_timed_out(unsigned long id) const;

};

}