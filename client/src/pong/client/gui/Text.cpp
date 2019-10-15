#include <pong/client/gui/Text.hpp>

namespace pong::client::gui {


Text::Text(Allocator<> gui, sf::String const& str, sf::Font const& font, unsigned char size)
:   sf::Text(str, font, size)
,   id(gui.allocate_properties(7)) {}

sf::Vector2f Text::getSize() const {
    auto bounds = getLocalBounds();
    return { bounds.width, bounds.height };
}

sf::Vector2f Text::get_offset() const {
    auto bounds = getLocalBounds();
    return { bounds.left, bounds.top };
}


void Text::free_properties(Gui<>& gui) {
    gui.free_properties(id, 7);
}

property_id_t Text::left_offset() const {
    return id + left_offset_id_offset;
} 

property_id_t Text::top_offset() const {
    return id + top_offset_id_offset;
} 

property_id_t Text::left() const {
    return id + left_id_offset;
}

property_id_t Text::top() const {
    return id + top_id_offset;
}

property_id_t Text::width() const {
    return id + width_id_offset;
}

property_id_t Text::height() const {
    return id + height_id_offset;
}

property_id_t Text::size() const {
    return id + size_id_offset;
}

void Text::update_properties(Gui<> const& gui) {
    setPosition({
        gui.get_property(left()),
        gui.get_property(top())
    });

    setCharacterSize(gui.get_property(size()));
}

void Text::notify_gui(Gui<>& gui) const {
    auto position = getPosition();
    auto char_size = getCharacterSize();
    auto bounds = getLocalBounds();

    gui.set_property(left(), position.x);
    gui.set_property(top(), position.y);
    gui.set_property(size(), char_size);

    gui.set_property(left_offset(), bounds.left);
    gui.set_property(top_offset(), bounds.top);
    gui.set_property(width(), bounds.width);
    gui.set_property(height(), bounds.height);
}


void Text::debug_draw(sf::RenderTarget& target, sf::RenderStates states) const {
    auto bounds = getLocalBounds();
    sf::RectangleShape rect{ { bounds.left, bounds.top } };
    rect.setPosition(getPosition());
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineColor(sf::Color::Magenta);
    rect.setOutlineThickness(1);

    sf::RectangleShape offset_rect(rect);
    offset_rect.move(rect.getSize());
    rect.setOutlineColor(sf::Color::Green);
    offset_rect.setSize(getSize());

    target.draw(rect, states);
    target.draw(offset_rect, states);
}


}