#pragma once

#include <SFML/Graphics.hpp>

#include <pong/client/gui/Button.hpp>
#include <pong/client/gui/Text.hpp>
#include <pong/client/Application.hpp>

#include <sftk/animated/Animated.hpp>

#include <pong/client/state/Room/Game.hpp>

#include <optional>
#include <string>

namespace pong::client::state::room {

class Graphics : gui::Element {
private:

    gui::Text quit_txt;
    gui::Button quit_button;

    sf::RectangleShape left_pad;
    sf::RectangleShape right_pad;
    sf::RectangleShape ball;

public:

    enum class Button {
        Quit, JoinQueue, LeaveQueue, Abandon
    };

    Graphics(Application app);
    void free_properties(gui::Allocator<> gui) const;
    

    std::optional<Button> on_click(sf::Vector2f const& position);
    std::optional<Button> on_release_click(sf::Vector2f const& position);
    void on_hover(sf::Vector2f const& position);


    void update_animations(Application app, float dt);
    void update_game(Application app, Game const& game);


    void notify_gui(gui::Gui<>& gui) const override;
    void update_properties(gui::Gui<> const& gui) override;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

};

}