#pragma once

#include <algorithm>
#include <vector>
#include <cassert>
#include <tuple>

#include <SFML/Graphics.hpp>

#include <pong/client/gui/Constraint/Constraint.hpp>
#include <pong/client/gui/Constraint/Set.hpp>

#include <chrono>

namespace pong::client::gui {

namespace details {
    template<typename F>
    struct GetArgs2 {};

    template<typename R, typename...Args>
    struct GetArgs2<R(Args...)> {
        using args = std::tuple<Args...>;
        using ret = R;
    };

    template<typename R, typename...Args>
    struct GetArgs2<R(*)(Args...)> {
        using args = std::tuple<Args...>;
        using ret = R;
    };

    template<typename L, typename R, typename...Args>
    struct GetArgs2<R(L::*)(Args...) const> {
        using args = std::tuple<Args...>;
        using ret = R;
    };

    template<typename F>
    struct GetArgs : GetArgs2<decltype(&F::operator())> {
        using GetArgs2<decltype(&F::operator())>::args;
    };

    template<typename R, typename...Args>
    struct GetArgs<R(Args...)> {
        using args = std::tuple<Args...>;
        using ret = R;
    };

    template<typename R, typename...Args>
    struct GetArgs<R(*)(Args...)> {
        using args = std::tuple<Args...>;
        using ret = R;
    };

    template<typename T, typename R>
    static constexpr bool is_last_vector_v = std::is_same_v<std::tuple_element_t<T::count - 1, typename T::args>, std::vector<R>>;

    template<typename T>
    using last_tuple_element_t = std::tuple_element_t<std::tuple_size_v<T> - 1, T>;

    template<typename F>
    using get_args_t = typename GetArgs<F>::args;

    template<typename F>
    using get_ret_t = typename GetArgs<F>::ret;

    template<typename F>
    static constexpr std::size_t count_v = std::tuple_size_v<get_args_t<F>>;

    template<auto f, typename Args, typename R, std::size_t...Is>
    auto uncurry(std::index_sequence<Is...>) {
        return [] (std::vector<R> const& vec) {
            assert(vec.size() >= std::tuple_size_v<Args>);
            if constexpr (std::is_convertible_v<std::vector<R> const&, last_tuple_element_t<Args>>) {
                return f(vec[Is]..., std::vector<R>(std::begin(vec) + (std::tuple_size_v<Args> - 1), std::end(vec)));
            } else {
                return f(vec[Is]...);
            }
        };
    }

}


template<typename T>
using constrained_func_t = T(*)(std::vector<T> const&);

template<auto f, typename R = details::get_ret_t<std::decay_t<decltype(f)>>>
constrained_func_t<R> uncurry() {
    using F = std::decay_t<decltype(f)>;
    using Args = details::get_args_t<F>;
    constexpr std::size_t Size = std::tuple_size_v<Args>;
    constexpr bool is_last_a_vector = std::is_convertible_v<std::vector<float> const&, details::last_tuple_element_t<Args>>;
    return details::uncurry<f, Args, R>(std::make_index_sequence<Size - is_last_a_vector>{});
}

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
    BitSet<> dirty_properties;

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
            return true;
        }

        return false;
    }

    void update_properties() {
        std:size_t i = order.size();
        //auto start = std::chrono::system_clock::now();
    
        for(auto const& property : order) {   
            auto it = get_constraint_iterator(property);
            if (it != std::end(constraints) && it->id == property) {
                auto& constraint = *it;
                std::vector<T> ts;
                //bool is_dirty = dirty_properties.contains(property);

                for(auto p : constraint.dependencies) {
                    /*if (dirty_properties.contains(p)) {
                        is_dirty = true;
                    }*/
                    ts.push_back(get_property(p));
                }
                /*if (is_dirty) {
                    dirty_properties.set(property);
                    --i;*/
                    get_property(property) = constraint.func(ts);/*
                }*/
            }
        }
    
        dirty_properties.clear();
    
        /*auto end = std::chrono::system_clock::now();
        std::cout << "AVOIDED: " << i << '/' << order.size() << '\n';
        std::cout << "TOOK " << std::chrono::duration<double>(end - start).count() << '\n';*/
    }

    property_id_t allocate_properties(std::size_t const count) {
        assert(count > 1);

        auto const index = values.push_multiple(count);
        dirty_properties.set_multiple(index, count);

        return index;
    }

    void free_properties(property_id_t const index, std::size_t const count) {
        assert(count > 1);

        values.erase_multiple(index, count);
    }

    property_id_t allocate_property(T const& value = T()) {
        auto const index = values.push(value);
        dirty_properties.set(index);
        return index;
    }

    void free_property(std::size_t const index) {
        values.erase(index);
    }

    void set_property(property_id_t const id, T const& value) {
        assert(values.contains(id));
        dirty_properties.set(id);
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

        dirty_properties.set(id);
    }


};


}