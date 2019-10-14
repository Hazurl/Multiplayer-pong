#pragma once

#include <algorithm>
#include <vector>
#include <cassert>
#include <tuple>

#include <SFML/Graphics.hpp>

#include <pong/client/gui/constraint/Constraint.hpp>
#include <pong/client/gui/constraint/Set.hpp>

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
            assert(vec.size() >= std::tuple_size_v<Args> && "The constraint's function provided doesn't not have the same number of parameter as the number of dependencies");
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

}