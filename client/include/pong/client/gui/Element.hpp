#pragma once

#include <pong/client/gui/constraint/Interface.hpp>
#include <pong/client/gui/constraint/Allocator.hpp>

namespace pong::client::gui {

struct Element {

    virtual ~Element() = default;

    virtual void update_properties(Gui<> const& gui);
    virtual void notify_gui(Gui<>& gui) const;

};

struct RectProperties {
private:

    static constexpr std::size_t left_offset = 0;
    static constexpr std::size_t top_offset = 1;
    static constexpr std::size_t width_offset = 2;
    static constexpr std::size_t height_offset = 3;


    pong::client::gui::property_id_t id;

public:

    RectProperties(Allocator<> gui);

    void free_properties(Allocator<> gui);

    property_id_t left() const;
    property_id_t top() const;
    property_id_t width() const;
    property_id_t height() const;

};

template<typename T>
struct RectElement : Element, RectProperties {

    RectElement(Allocator<>& gui) : RectProperties(gui) {}

    void update_properties(Gui<> const& gui) override {
        static_cast<T*>(this)->setPosition({
            gui.get_property(left()),
            gui.get_property(top())
        });
        static_cast<T*>(this)->setSize({
            gui.get_property(width()),
            gui.get_property(height())
        });
    }

    void notify_gui(Gui<>& gui) const override {
        auto const position = static_cast<T const*>(this)->getPosition();
        auto const size = static_cast<T const*>(this)->getSize();
        gui.set_property(left(), position.x);
        gui.set_property(top(), position.y);
        gui.set_property(width(), size.x);
        gui.set_property(height(), size.y);
    }


};

}