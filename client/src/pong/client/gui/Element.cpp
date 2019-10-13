#include <pong/client/gui/constraint/Interface.hpp>
#include <pong/client/gui/Element.hpp>

namespace pong::client::gui {

void Element::update_properties(Gui<> const& gui) {}
void Element::notify_gui(Gui<>& gui) const {}

RectProperties::RectProperties(Gui<>& gui) : id{ gui.allocate_properties(4) } {}

void RectProperties::free_properties(Gui<>& gui) {
    gui.free_properties(id, 4);
}

property_id_t RectProperties::left() const { return id + left_offset; }
property_id_t RectProperties::top() const { return id + top_offset; }
property_id_t RectProperties::width() const { return id + width_offset; }
property_id_t RectProperties::height() const { return id + height_offset; }

}