#pragma once

#include <pong/client/gui/constraint/Interface.hpp>

namespace pong::client::gui {

template<typename T = float>
class Allocator {
public:

    Allocator(Gui<T>& _gui) : gui{ &_gui } {}

    property_id_t allocate_property(T const& value = T{}) {
        return gui->allocate_property(value);
    }

    property_id_t allocate_properties(std::size_t const count) {
        return gui->allocate_properties(count);
    }

    void free_property(property_id_t const index) {
        return gui->free_property(index);
    }

    void free_properties(property_id_t const index, std::size_t const count) {
        return gui->free_properties(index, count);
    }

    template<auto f>
    void set_constraint(property_id_t id, std::vector<property_id_t> dependencies) {
        return gui->template set_constraint<f>(id, std::move(dependencies));
    }

private:

    Gui<T>* gui;

};

}