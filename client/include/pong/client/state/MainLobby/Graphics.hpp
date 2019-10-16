#pragma once

#include <SFML/Graphics.hpp>

#include <pong/client/gui/Button.hpp>
#include <pong/client/gui/Text.hpp>
#include <pong/client/Application.hpp>

#include <sftk/animated/Animated.hpp>

#include <optional>
#include <string>

namespace pong::client::state::mainlobby {

class Graphics : gui::Element {
private:

    gui::Text title_txt;
    gui::Text by_hazurl_txt;
    gui::Text quit_txt;
    gui::Button quit_button;
    gui::Text create_room_txt;
    gui::Button create_room_button;

public:

    enum class Button {
        Quit, CreateRoom
    };

    Graphics(Application app);
    void free_properties(gui::Allocator<> gui) const;
    

    std::optional<Button> on_click(sf::Vector2f const& position);
    std::optional<Button> on_release_click(sf::Vector2f const& position);
    void on_hover(sf::Vector2f const& position);


    void update_animations(Application app, float dt);

    void notify_gui(gui::Gui<>& gui) const override;
    void update_properties(gui::Gui<> const& gui) override;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

};

}