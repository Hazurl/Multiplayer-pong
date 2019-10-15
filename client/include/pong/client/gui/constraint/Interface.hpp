#pragma once

#include <algorithm>
#include <vector>
#include <cassert>
#include <tuple>

#include <SFML/Graphics.hpp>

#include <pong/client/gui/constraint/Constraint.hpp>
#include <pong/client/gui/constraint/Solver.hpp>
#include <pong/client/gui/constraint/Set.hpp>

#include <chrono>

namespace pong::client::gui {


template<typename T = float>
struct Gui {

    struct Constraint {
        property_id_t id;
        std::vector<property_id_t> dependencies;
        constrained_func_t<T> func;
    };

private:

    Set<T> values;
    std::vector<Constraint> constraints;

    std::vector<property_id_t> order;
    bool order_up_to_date = false;

    auto get_constraint_iterator(property_id_t id) {
        return std::lower_bound(std::begin(constraints), std::end(constraints), id, 
            [] (Constraint const& current_constraint, std::size_t const id_to_find) {
                return current_constraint.id < id_to_find;
            });
    }

    auto get_constraint_iterator(property_id_t id) const {
        return std::lower_bound(std::begin(constraints), std::end(constraints), id, 
            [] (Constraint const& current_constraint, std::size_t const id_to_find) {
                return current_constraint.id < id_to_find;
            });
    }

public:

    bool is_up_to_date() const {
        return order_up_to_date;
    }

    bool compute_order() {
        property_graph_t graph;

        for(property_id_t id{ 0 }; id < values.capacity(); ++id) {
            if (values.contains(id)) {
                graph[id] = {};
            }
        }

        for(auto constraint : constraints) {
            for(auto dependency : constraint.dependencies) {
                // Arc dependency -> constraint.prop
                graph[constraint.id].insert(dependency);
            }
        }


        auto res = find_update_order(std::move(graph));

        if (res) {
            order = std::move(*res);
            return order_up_to_date = true;
        }

        return order_up_to_date = false;
    }

    void update_properties() {
        for(auto const& property : order) {   
            auto it = get_constraint_iterator(property);
            if (it != std::end(constraints) && it->id == property) {
                auto& constraint = *it;
                std::vector<T> ts;

                for(auto p : constraint.dependencies) {
                    ts.push_back(get_property(p));
                }
                get_property(property) = constraint.func(ts);
            }
        }
    }

    property_id_t allocate_properties(std::size_t const count) {
        assert(count > 1);

        auto const index = values.push_multiple(count);

        order_up_to_date = false;
        return index;
    }

    void free_properties(property_id_t const index, std::size_t const count) {
        assert(count > 1);

        order_up_to_date = false;
        values.erase_multiple(index, count);
    }

    property_id_t allocate_property(T const& value = T()) {
        auto const index = values.push(value);
        order_up_to_date = false;
        return index;
    }

    void free_property(property_id_t const index) {
        order_up_to_date = false;
        values.erase(index);
    }

    void set_property(property_id_t const id, T const& value) {
        assert(values.contains(id));
        order_up_to_date = false;
        values[id] = value;
    }

    T const& get_property(property_id_t const id) const {
        assert(values.contains(id));
        return values[id];
    }

    T& get_property(property_id_t const id) {
        assert(values.contains(id));
        return values[id];
    }

    template<auto f>
    void set_constraint(property_id_t id, std::vector<property_id_t> dependencies) {
        Constraint constraint{
            id, std::move(dependencies), uncurry<f>()
        };
        auto it = get_constraint_iterator(id);

        // Constraint already exist for this property
        if (it != std::end(constraints) && it->id == id) {
            *it = std::move(constraint);
        } else {
            constraints.insert(it, std::move(constraint));
        }
        order_up_to_date = false;
    }


};


}