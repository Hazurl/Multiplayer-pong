#pragma once

#include <SFML/Graphics.hpp>

#include <pong/client/gui/Button.hpp>
#include <pong/client/gui/Text.hpp>
#include <pong/client/Application.hpp>

#include <optional>
#include <string>

namespace pong::client::state::login {

class Graphics : public sf::Drawable, gui::Element {
private:

    gui::Text quit_txt;
    gui::Button quit_button;

public:

    enum class Button {
        Quit
    };

    Graphics(Application& app);


    void on_connection();
    void on_disconnection();

    void cursor_left();
    void cursor_right();
    void on_character_entered(char c);
    std::optional<std::string> validate_username();


    std::optional<Button> on_click(sf::Vector2f const& position);
    std::optional<Button> on_release_click(sf::Vector2f const& position);
    void on_hover(sf::Vector2f const& position);


    void update_animations(float dt);

    void notify_gui(gui::Gui<>& gui) const override;
    void update_properties(gui::Gui<> const& gui) override;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

};

}