#include <dbg_pckt/CommandTextArea.hpp>

#include <dbg_pckt/Parser.hpp>

namespace dbg_pckt::gui {

CommandTextArea::CommandTextArea(sf::Font& font, unsigned width, unsigned height, std::string _default_message) 
:   TextArea(font, width, height)
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
    auto const& content = get_content();
    auto res = parser::parse_command(content);
    auto prediction = res.is_error() ? current_prediction(res.error()) : "";
    write(prediction);
}

std::string CommandTextArea::current_prediction(std::vector<std::string> const& predictions) const {
    return predictions.empty() ? "" : predictions[ current_prediction_index % predictions.size() ]; 
}

sftk::TextBuilder CommandTextArea::generate_text() {
    auto const& content = get_content();
    
    if(content.empty() && !is_focus()) {
        return 
            sftk::TextBuilder{ get_font() }
                << sftk::txt::size(get_font_size())
                << sf::Color{ 100, 100, 100 }
                << default_message;
    }

    auto res = parser::parse_command(content);
    auto builder = sftk::TextBuilder{ get_font() } << sftk::txt::size(get_font_size());
    auto prediction = res.is_error() ? current_prediction(res.error()) : "";

    //bool is_red{ false };
    bool is_selected{ false };
/*
    float cursor_x{ 3 };
    float selection_lower_bound_x{ 3 };
    float selection_upper_bound_x{ 3 };
*/
    for(std::size_t i{ 0 }; i < content.size(); ++i) {
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
        builder.append(content[i]);
/*
        if(i + 1 == cursor_index) {
            cursor_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
        }

        if(i + 1 == selection_lower_bound) {
            selection_lower_bound_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
        }

        if(i + 1 == selection_upper_bound) {
            selection_upper_bound_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
        }*/
    }
/*
    if (cursor_index >= content.size()) {
        cursor_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
        cursor_index = content.size();
    } 

    if (selection_lower_bound >= content.size()) {
        selection_lower_bound_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
    } 

    if (selection_upper_bound >= content.size()) {
        selection_upper_bound_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
    } 
*/
    return std::move(builder)
        << sf::Color{ 100, 100, 100 }
        << prediction;
/*
    cursor.setPosition(cursor_x, 17);
    selection.setSize({ selection_upper_bound_x == selection_lower_bound_x ? 0 : selection_upper_bound_x - selection_lower_bound_x + 1, selection.getSize().y });
    selection.setPosition(selection_lower_bound_x - 1, 0);*/
}


}