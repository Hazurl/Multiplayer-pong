#pragma once

#include <SFML/Graphics.hpp>

#include <pong/client/gui/Button.hpp>
#include <pong/client/gui/Text.hpp>
#include <pong/client/gui/Sprite.hpp>
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

    gui::Text spectator_count_txt;
    sf::Texture spectator_texture;
    gui::Sprite spectator_spr;

    gui::Text join_queue_txt;
    gui::Button join_queue_button;

    gui::Text leave_queue_txt;
    gui::Button leave_queue_button;

    gui::Text abandon_txt;
    gui::Button abandon_button;

    gui::Text versus_txt;
    gui::Text left_versus_txt;
    gui::Text right_versus_txt;

    gui::Text score_dash_txt;
    gui::Text left_score_txt;
    gui::Text right_score_txt;

    sf::RectangleShape left_pad;
    sf::RectangleShape right_pad;
    sf::RectangleShape ball;

public:

    enum class Button {
        Quit, JoinQueue, LeaveQueue, Abandon
    };

    Graphics(Application app);
    void free_properties(gui::Allocator<> gui) const;
    

    std::optional<Button> on_click(Game::Role role, sf::Vector2f const& position);
    std::optional<Button> on_release_click(Game::Role role, sf::Vector2f const& position);
    void on_hover(sf::Vector2f const& position);


    void update_animations(Application app, float dt);
    void update_game(Application app, Game const& game);
    void update_score(Application app, unsigned left, unsigned right);

    
    void set_spectator_count(unsigned spectator_count);
    void set_left_player(std::string username);
    void set_right_player(std::string username);

    void unhighlight_left_player();
    void unhighlight_right_player();
    void unhighlight_spectator_count();

    void highlight_left_player();
    void highlight_right_player();
    void highlight_spectator_count();


    void notify_gui(gui::Gui<>& gui) const override;
    void update_properties(gui::Gui<> const& gui) override;

    void draw(bool left_player_present, bool right_player_present, Game::Role role, sf::RenderTarget& target, sf::RenderStates states) const;

};

}