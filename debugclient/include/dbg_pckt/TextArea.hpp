#pragma once

#include <SFML/Graphics.hpp>
#include <SFML-Toolkit/include/sftk/fancyText/FancyText.hpp>

#include <string>

namespace dbg_pckt::gui {

enum class CursorDirection {
    Left, Right
};

enum class Selection {
    Keep, Reset
};

enum class Step {
    Character, Word
};

class TextArea : public sf::Drawable, public sf::Transformable {
public:

    TextArea(sf::Font& font, unsigned width, unsigned height) ;
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void write(char c);
    void write(std::string const& str);

    std::string copy_selection() const;
    void remove_selection();

    void remove_next_character();
    void remove_previous_character();

    void clear();

    void move_cursor(CursorDirection dir, Selection sel, Step step);

    void set_focus(bool enable);

    std::string const& get_content() const;

protected:

    struct GeneratedText {
        sftk::TextBuilder builder;
        float cursor_x;
        float selection_left;
        float selection_right;
    };

    void force_generation();
    virtual GeneratedText generate_text();

    sf::Font const& get_font() const; 
    float get_font_size() const; 

    bool is_in_selection(std::size_t cursor) const;
    bool is_focus() const;

    std::size_t get_cursor_index() const;
    std::size_t get_left_selection_index() const;
    std::size_t get_right_selection_index() const;

private:

    std::size_t selection_lower_bound() const;
    std::size_t selection_size() const;

    std::string content;

    sf::Font& font;
    sf::RectangleShape box;
    sf::RectangleShape cursor;
    sf::RectangleShape selection;
    sftk::FancyText fancy_text;

    std::size_t cursor_index;
    int selection_offset;

    bool focus_enable;

};

}