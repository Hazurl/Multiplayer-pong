#include <dbg_pckt/CommandTextArea.hpp>

#include <dbg_pckt/Parser.hpp>

#include <cassert>

namespace dbg_pckt::gui {

CommandTextArea::CommandTextArea(sf::Font& _font, unsigned width, unsigned height, std::string _default_message) 
:   TextArea(_font, width, height)
,   default_message{ _default_message }
,   current_prediction_index{ 0 }
{
    force_generation();
}

void CommandTextArea::cycle_prediction() {
    ++current_prediction_index;
    force_generation();
}

void CommandTextArea::cycle_previous_prediction() {
    --current_prediction_index;
    force_generation();
}

void CommandTextArea::write_prediction() {
    auto const& text_content = get_content();
    auto res = parser::parse_command(text_content);
    auto prediction = res.is_error() ? current_prediction(res.error()) : "";
    write(prediction);
    current_prediction_index = 0;
}

std::string CommandTextArea::current_prediction(std::vector<std::string> const& predictions) const {
    return predictions.empty() ? "" : predictions[ current_prediction_index % predictions.size() ]; 
}

TextArea::GeneratedText CommandTextArea::generate_text() {
    auto const& text_content = get_content();

    TextArea::GeneratedText generated_text{
        sftk::TextBuilder{ get_font() } << sftk::txt::size(get_font_size()),
        0, 0, 0
    };

    auto& builder = generated_text.builder;
    
    if(text_content.empty() && !is_focus()) {
        builder << sf::Color{ 100, 100, 100 }
                << default_message;

        return generated_text;
    }

    auto res = parser::parse_command(text_content);
    auto prediction = res.is_error() ? current_prediction(res.error()) : "";

    //bool is_red{ false };
    bool is_selected{ false };

    assert(get_cursor_index() <= text_content.size());
    assert(get_left_selection_index() <= text_content.size());
    assert(get_right_selection_index() <= text_content.size());

    for(std::size_t i{ 0 }; i < text_content.size(); ++i) {
        bool is_i_in_error = false;
        bool is_i_in_selection = is_in_selection(i);
/*
        if (is_in_error && !is_red) {
            builder.set_fill_color(sf::Color::Red);
            is_red = true;
        } 
        else if (!is_in_error && is_red) {
            builder.set_fill_color(is_in_selection ? sf::Color::Black : sf::Color::White);
            is_red = false;
        }
        */
        if (is_i_in_selection && !is_selected) {
            builder.set_fill_color(is_i_in_error ? sf::Color::Red : sf::Color::Black);
            is_selected = true;
        } 
        else if (!is_i_in_selection && is_selected) {
            builder.set_fill_color(is_i_in_error ? sf::Color::Red : sf::Color::White);
            is_selected = false;
        }
        builder.append(text_content[i]);

        if (get_cursor_index() == i + 1) {
            generated_text.cursor_x = builder.get_current_position().x;
        }

        if (get_left_selection_index() == i + 1) {
            generated_text.selection_left = builder.get_current_position().x;
        }

        if (get_right_selection_index() == i + 1) {
            generated_text.selection_right = builder.get_current_position().x;
        }
    }

    builder << sf::Color{ 100, 100, 100 }
            << prediction;

    return generated_text;
}


}