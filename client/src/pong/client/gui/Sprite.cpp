#include <pong/client/gui/Sprite.hpp>

namespace pong::client::gui {


Sprite::Sprite(Gui<>& gui, sf::Texture const& texture)
:   sf::RectangleShape()
,   id(gui.allocate_properties(5)) {
    setTexture(&texture);
}

void Sprite::free_properties(Gui<>& gui) {
    gui.free_properties(id, 5);
}

property_id_t Sprite::left() const {
    return id + left_id_offset;
}

property_id_t Sprite::top() const {
    return id + top_id_offset;
}

property_id_t Sprite::width() const {
    return id + width_id_offset;
}

property_id_t Sprite::height() const {
    return id + height_id_offset;
}

property_id_t Sprite::ratio() const {
    return id + ratio_id_offset;
}

void Sprite::update_properties(Gui<> const& gui) {
    setPosition({
        gui.get_property(left()),
        gui.get_property(top())
    });

    setSize({
        gui.get_property(width()),
        gui.get_property(height())
    });
}

void Sprite::notify_gui(Gui<>& gui) const {
    auto position = getPosition();
    auto size = getSize();

    gui.set_property(left(), position.x);
    gui.set_property(top(), position.y);
    gui.set_property(width(), size.x);
    gui.set_property(height(), size.y);

    if (getTexture()) {
        auto texture_size = getTexture()->getSize();
        gui.set_property(ratio(), static_cast<float>(texture_size.x) / static_cast<float>(texture_size.y));
    } else {
        gui.set_property(ratio(), 1.0f);
    }
}


void Sprite::debug_draw(sf::RenderTarget& target, sf::RenderStates states) const {
    sf::RectangleShape rect{ getSize() };
    rect.setPosition(getPosition());
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineColor(sf::Color::Magenta);
    rect.setOutlineThickness(1);

    target.draw(rect, states);
}


}