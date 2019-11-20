#include <pong/client/Notification.hpp>

namespace pong::client::notif {

Notification::Notification(sf::String message_, Category category_, Lifetime lifetime_, Button button_)
:   message{ std::move(message_) }
,   category{ category_ }
,   lifetime{ std::move(lifetime_) }
,   button{ std::move(button_) }
{}




NotificationMesh::NotificationMesh(gui::Allocator<> gui, Notification const& notification, sf::Texture const& refresh_texture, sf::Font const& font) 
:   background(
        gui,
        [] {},
        gui::Button::Theme{
            sf::Color{ 0xBC, 0x31, 0x31, 0xA0 }, // idle  
            sf::Color{ 0xBC, 0x31, 0x31, 0xA0 }, // hover  
            sf::Color{ 0xBC, 0x31, 0x31, 0xA0 }  // click 
        })
,   message(gui, notification.message, font, 16)
{

    if(std::holds_alternative<GenericButton>(notification.button)) {
        gui::Button::Theme theme{
            sf::Color{ 0xBC, 0x31, 0x31, 0x00 }, // idle  
            sf::Color{ 0xBC, 0x31, 0x31, 0xA0 }, // hover  
            sf::Color{ 0xBC, 0x31, 0x31, 0xFF }  // click 
        };
        
        NotificationMesh::ButtonMesh mesh {
            gui::Button(gui, []{}, theme),
            gui::Text(gui, std::get<GenericButton>(notification.button).message, font, 24)
        };

        button_mesh = std::move(mesh);

        auto const message_bounds = message.getLocalBounds();
        auto const message_size = sf::Vector2f{ message_bounds.width, message_bounds.height };
        auto const background_size = sf::Vector2f{ 300, 25 };

        background.setSize(background_size);
        message.setPosition((background_size - message_size) / 2.f);
    }
    
    else if(std::holds_alternative<RefreshButton>(notification.button)) {
        gui::Button::Theme theme{
            sf::Color{ 0xBC, 0x31, 0x31, 0x00 }, // idle  
            sf::Color{ 0xBC, 0x31, 0x31, 0xA0 }, // hover  
            sf::Color{ 0xBC, 0x31, 0x31, 0xFF }  // click 
        };
        
        NotificationMesh::RefreshMesh mesh {
            gui::Button(gui, []{}, theme),
            gui::Sprite(gui, refresh_texture)
        };

        auto const message_bounds = message.getLocalBounds();
        auto const message_size = sf::Vector2f{ message_bounds.width, message_bounds.height };
        auto const background_size = sf::Vector2f{ 300, 25 };

        mesh.sprite.setSize({ 17.f, 17.f });
        mesh.sprite.setPosition(background_size.x - 17.f /* sprite size */ - 4.f /* padding */, 4.f /* padding */);

        mesh.button.setPosition(background_size.x - 21.f /* button size */ - 2.f /* padding */, 2.f /* padding */);
        mesh.button.setSize({ 21.f, 21.f });

        button_mesh = std::move(mesh);

        background.setSize(background_size);
        message.setPosition((background_size - message_size) / 2.f);
    }

    else { // Simple message
        auto const message_bounds = message.getLocalBounds();
        auto const message_offset = sf::Vector2f{ message_bounds.left, message_bounds.top };
        auto const message_size = sf::Vector2f{ message_bounds.width, message_bounds.height };
        auto const background_size = sf::Vector2f{ 300, 25 };

        background.setSize(background_size);
        message.setPosition((background_size - message_size) / 2.f - message_offset);
    }
}

void NotificationMesh::update(float dt) {
    background.update(dt);
    std::visit(Visitor{
        [&] (NotificationMesh::None) {},
        [&] (auto& m) {
            m.button.update(dt);
        }
    }, button_mesh);
}

void NotificationMesh::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(background, states);
    target.draw(message, states);

    std::visit(Visitor{
        [&] (NotificationMesh::ButtonMesh const& m) {
            target.draw(m.button, states);
            target.draw(m.label, states);
        },
        [&] (NotificationMesh::RefreshMesh const& m) {
            target.draw(m.button, states);
            target.draw(m.sprite, states);
        },
        [] (auto const&) {}
    }, button_mesh);
}

void NotificationMesh::notify_gui(gui::Gui<>& gui) const {
    background.notify_gui(gui);
    message.notify_gui(gui);
    std::visit(Visitor{
        [&] (NotificationMesh::ButtonMesh const& m) {
            m.button.notify_gui(gui);
            m.label.notify_gui(gui);
        },
        [&] (NotificationMesh::RefreshMesh const& m) {
            m.button.notify_gui(gui);
            m.sprite.notify_gui(gui);
        },
        [] (auto const&) {}
    }, button_mesh);
}

void NotificationMesh::update_properties(gui::Gui<> const& gui) {
    background.update_properties(gui);
    message.update_properties(gui);
    std::visit(Visitor{
        [&] (NotificationMesh::ButtonMesh& m) {
            m.button.update_properties(gui);
            m.label.update_properties(gui);
        },
        [&] (NotificationMesh::RefreshMesh& m) {
            m.button.update_properties(gui);
            m.sprite.update_properties(gui);
        },
        [] (auto const&) {}
    }, button_mesh);
}

void NotificationMesh::free_properties(gui::Allocator<> gui) const {
    background.free_properties(gui);
    message.free_properties(gui);
    std::visit(Visitor{
        [&] (NotificationMesh::ButtonMesh const& m) {
            m.button.free_properties(gui);
            m.label.free_properties(gui);
        },
        [&] (NotificationMesh::RefreshMesh const& m) {
            m.button.free_properties(gui);
            m.sprite.free_properties(gui);
        },
        [] (auto const&) {}
    }, button_mesh);
}

std::optional<NotificationMesh::Event> NotificationMesh::on_click(sf::Vector2f const& mouse) {
    std::optional<NotificationMesh::Event> event;

    if (background.on_click(mouse)) {
        event = NotificationMesh::Event::Background;
    }

    std::visit(Visitor{
        [&] (NotificationMesh::ButtonMesh& m) {
            if (m.button.on_click(mouse)) {
                event = NotificationMesh::Event::Button;
            }
        },
        [&] (NotificationMesh::RefreshMesh& m) {
            if (m.button.on_click(mouse)) {
                event = NotificationMesh::Event::Refresh;
            }
        },
        [] (auto const&) {}
    }, button_mesh);

    return event;
}

std::optional<NotificationMesh::Event> NotificationMesh::on_hover(sf::Vector2f const& mouse) {
    std::optional<NotificationMesh::Event> event;

    if (background.on_hover(mouse)) {
        event = NotificationMesh::Event::Background;
    }

    std::visit(Visitor{
        [&] (NotificationMesh::ButtonMesh& m) {
            if (m.button.on_hover(mouse)) {
                event = NotificationMesh::Event::Button;
            }
        },
        [&] (NotificationMesh::RefreshMesh& m) {
            if (m.button.on_hover(mouse)) {
                event = NotificationMesh::Event::Refresh;
            }
        },
        [] (auto const&) {}
    }, button_mesh);

    return event;
}

std::optional<NotificationMesh::Event> NotificationMesh::on_release_click(sf::Vector2f const& mouse) {
    std::optional<NotificationMesh::Event> event;

    if (background.on_release_click(mouse)) {
        event = NotificationMesh::Event::Background;
    }

    std::visit(Visitor{
        [&] (NotificationMesh::ButtonMesh& m) {
            if (m.button.on_release_click(mouse)) {
                event = NotificationMesh::Event::Button;
            }
        },
        [&] (NotificationMesh::RefreshMesh& m) {
            if (m.button.on_release_click(mouse)) {
                event = NotificationMesh::Event::Refresh;
            }
        },
        [] (auto const&) {}
    }, button_mesh);

    return event;
}




unsigned long Queue::next_id{ 0 };

Queue::Queue() {
    if(!refresh_texture.loadFromFile("../assets/refresh.png")) {
        std::cerr << "Couldn't load texture '../assets/refresh.png'\n";
        throw std::runtime_error("Couldn'l load texture '../assets/refresh.png'");
    }
}

void Queue::update_animations(Application app, float dt) {
    notifications.erase(std::remove_if(std::begin(notifications), std::end(notifications), [&] (auto& n) {
        /*
            Start animation if lifetime.close_after has passed
            of if the notication is clicked and lifetime.close_on_click is true
        */
        if (n.lifetime.close_after) {
            float rem = *n.lifetime.close_after -= dt;
            if (rem < 0.f && n.target_x.get_target() < app.width()) {
                n.target_x.animate(app.width(), 0.5f);
            }
        }

        if (n.lifetime.close_on_click && n.is_clicked) {
            if (n.target_x.get_target() < app.width()) {
                n.target_x.animate(app.width(), 0.5f);
            }
        }

        /*
            Erase notification if the animation is done
        */

        if (n.target_x.get_target() >= app.width()) {
            return !n.target_x.is_animated();
        }

        return false;
    }), std::end(notifications));
    
    std::size_t position_step = 0;
    for(auto& n : notifications) {
        n.mesh.update(dt);


        auto height = static_cast<float>(app.height()) - ++position_step * 27.f /* notification height */ - 5 /* padding */;
        n.target_y.animate(height, std::abs(height - n.target_y.current()) / 200.f);

        n.target_x.update(dt);
        n.target_y.update(dt);
    }
}

void Queue::on_click(sf::Vector2f const& mouse) {
    for(auto& n : notifications) {
        n.mesh.on_click(mouse - sf::Vector2f{ n.target_x.current(), n.target_y.current() });
    }
}

void Queue::on_hover(sf::Vector2f const& mouse) {
    for(auto& n : notifications) {
        n.mesh.on_hover(mouse - sf::Vector2f{ n.target_x.current(), n.target_y.current() });
    }
}

void Queue::on_release_click(sf::Vector2f const& mouse) {
    for(auto& n : notifications) {
        auto event = n.mesh.on_release_click(mouse - sf::Vector2f{ n.target_x.current(), n.target_y.current() });
        if (event) {
            n.is_clicked = *event == NotificationMesh::Event::Background;
            n.is_button_clicked = 
                *event == NotificationMesh::Event::Button
            ||  *event == NotificationMesh::Event::Refresh;
        }
    }
}

void Queue::push(Application app, Notification notification) {

    float start = app.width();
    float end = app.width() - 300 /* notification width */ - 5 /* padding */;
    sftk::Animated<float, sftk::interpolation::Bezier<float>> target_x(
        sftk::interpolation::Bezier<float>{ end, end }, start
    );
    target_x.animate(end, 0.5f);

    float starting_height = notifications.empty() ? 
        app.height() - 27.f /* notification height */ - 5 /* padding */
    :   notifications.back().target_y.current() - 27.f;

    notifications.push_back({
        next_id++,
        notification.lifetime,
        std::holds_alternative<NoButton>(notification.button),
        false, false,
        NotificationMesh(app.gui_allocator(), notification, refresh_texture, app.get_font()),
        target_x,
        sftk::Animated<float>{ sftk::interpolation::linear<float>, starting_height }, 
    });
}

void Queue::draw(sf::RenderTarget& target) const {
    for(auto const& n : notifications) {
        sf::Transform tf;
        tf.translate(n.target_x, n.target_y);
        target.draw(n.mesh, tf);
    }
}

void Queue::notify_gui(gui::Gui<>& gui) const {
    for(auto const& n : notifications) {
        n.mesh.notify_gui(gui);
    }
}

void Queue::update_properties(gui::Gui<> const& gui) {
    for(auto& n : notifications) {
        n.mesh.update_properties(gui);
    }
}

void Queue::free_properties(gui::Allocator<> gui) const {
    for(auto const& n : notifications) {
        n.mesh.free_properties(gui);
    }
}

Controller Queue::make_controller() {
    return Controller{ notifications };
}





Controller::Controller(std::vector<Queue::Object>& notifications_) : notifications{ &notifications_ } {}

bool Controller::is_clicked(unsigned long id) const {
    for(auto const& n : *notifications) {
        if(id == n.id) return n.is_clicked;
    }
    return false;
}

bool Controller::is_refresh_button_clicked(unsigned long id) const {
    return is_button_clicked(id);
}

bool Controller::is_button_clicked(unsigned long id) const {
    for(auto const& n : *notifications) {
        if(id == n.id) return n.is_button_clicked;
    }
    return false;
}

bool Controller::is_timed_out(unsigned long id) const {
    // Notification is timed out iff it has been deleted, or the lifetime is over
    for(auto const& n : *notifications) {
        if(id == n.id) {
            return n.lifetime.close_after && n.lifetime.close_after < 0;
        }
    }
    return true;
}

}