#pragma once

#include <dbg_pckt/TextArea.hpp>

namespace dbg_pckt::gui {

class CommandTextArea : public TextArea {
public:

    CommandTextArea(sf::Font& font, unsigned width, unsigned height, std::string default_message);

    void cycle_prediction();
    void cycle_previous_prediction();
    void write_prediction();

private:

    std::string current_prediction(std::vector<std::string> const& predictions) const;

    TextArea::GeneratedText generate_text() override;

    std::string default_message;
    std::size_t current_prediction_index;

};

}