#pragma once

#include <wpr/Containers.hpp>
#include <wpr/Parsers.hpp>
#include <wpr/Details.hpp>

#include <variant>
#include <cmath>
#include <string>

namespace dbg_pckt::parser {

namespace wsp = wpr;

using prediction_t = std::string;
using predictions_t = std::vector<prediction_t>;

namespace details {

    template<char n>
    bool is_not(char c) {
        return c != n;
    }

    template<typename T>
    T unwrap(wsp::Sum<T> sum) {
        return std::move(std::get<0>(static_cast<std::variant<T>&&>(sum)));
    }

}






namespace string_details {

    char from_escaped_characters(char c) {
        switch(c) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
        }

        return c;
    }

    predictions_t join_predictions(wsp::Product<prediction_t, prediction_t> p) {
        auto& p0 = std::get<0>(p);
        auto& p1 = std::get<1>(p);

        if (p0.empty()     && p1.empty()) { return {}; }
        if (p0.size() == 1 || p1.empty()) { return { std::move(p0) }; }
        if (p1.size() == 1 || p0.empty()) { return { std::move(p1) }; }

        return { std::move(p0), std::move(p1) };
    }

    template<char V, bool is_begin>
    prediction_t quote_err_to_predictions(wsp::Sum<wsp::error::NotMatching<V>, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return prediction_t((is_begin ? 2 : 1), V);
        }

        return "";
    }

    constexpr auto escaped_characters = wsp::seq<wsp::ch<'\\'>, wsp::nextc> [ wsp::peek<1> ] [ wsp::map<from_escaped_characters> ];

    template<char quote>
    constexpr auto character_not_quote = wsp::first<escaped_characters, wsp::nextc [ wsp::filter<details::is_not<quote>> ]> [ wsp::map<details::unwrap<char>> ];

    template<char quote>
    constexpr auto many_character_not_quote = wsp::many<character_not_quote<quote>, std::basic_string>;

    // Parser<char, predictions_t>
    template<char V, bool is_begin>
    constexpr auto quote = wsp::ch<V> [ wsp::map_err<quote_err_to_predictions<V, is_begin>> ];

    template<char V>
    constexpr auto begin_quote = quote<V, true>;

    template<char V>
    constexpr auto end_quote = quote<V, false>;

    template<char V>
    constexpr auto quoted_string = 
        wsp::seq<
            begin_quote<V>,
            many_character_not_quote<V>,
            end_quote<V>
        > [ wsp::peek<1> ] [ wsp::map_err<details::unwrap<prediction_t>> ];

}

constexpr auto string = 
    wsp::first<
        string_details::quoted_string<'"'>, 
        string_details::quoted_string<'\''>
    > 
        [ wsp::map<details::unwrap<std::string>> ] 
        [ wsp::map_err<string_details::join_predictions> ];





namespace number_details {
    bool is_digit(char c) { return c >= '0' && c <= '9'; }

    int digits_to_integer(std::string s) {
        int n{ 0 };
        for(auto c : s) {
            n = n*10 + (c - '0');
        }

        return n;
    }

    float digits_to_decimal(std::string s) {
        int size = s.size();
        int n = digits_to_integer(std::move(s));

        return static_cast<float>(n) / std::pow(10, size);
    }

    float add_parts(wsp::Product<int, float> p) {
        return std::get<0>(p) + std::get<1>(p);
    }

    int point_to_zero(char) {
        return 0;
    }

    predictions_t predictions(wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return { "0" };
        }

        return {};
    }

    wsp::Result<float, predictions_t> handle_decimal_err(wsp::Sum<wsp::Sum<wsp::error::NotMatching<'.'>, wsp::error::EndOfFile>, predictions_t> err) {
        if (err.index() == 0) {
            return wsp::success(0, 0.0f);
        } else {
            return wsp::fail(1, std::get<1>(std::move(err)));
        }
    }

    constexpr auto digit = wsp::nextc [ wsp::filter<is_digit> ];
    constexpr auto digits = wsp::some<digit, std::basic_string> [ wsp::map_err<number_details::predictions> ];

    constexpr auto decimal_part_unhandled = 
        wsp::seq<
            wsp::ch<'.'>,                           // Result<char, Sum<NotMatching<'.'>, EOF>>
            digits [ wsp::map<digits_to_decimal> ]  // Result<float, predictions_t>
        > [ wsp::peek<1> ];                         // Result<float, Sum<Sum<NotMatching<'.'>, EOF>, predictions_t>>

    constexpr auto decimal_part = decimal_part_unhandled [ wsp::bind_err<handle_decimal_err> ];
}

constexpr auto integer = number_details::digits [ wsp::map<number_details::digits_to_integer> ];

constexpr auto number = 
    wsp::seq<
        integer, 
        number_details::decimal_part
    > [ wsp::map<number_details::add_parts> ] [ wsp::map_err<details::unwrap<predictions_t>> ];





namespace ident_details {

    bool is_alpha(char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
    }

    bool is_alpha_numeric(char c) {
        return (c >= '0' && c <= '9') || is_alpha(c);
    }

    predictions_t predictions(wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return { "_" };
        }
        return {};
    }

    std::string join_success(wsp::Product<char, std::string> p) {
        return std::get<0>(p) + std::move(std::get<1>(p));
    }

    constexpr auto alpha_character = wsp::nextc [ wsp::filter<is_alpha> ] [ wsp::map_err<predictions> ];
    constexpr auto alpha_numeric_character = wsp::nextc [ wsp::filter<is_alpha_numeric> ];

}

constexpr auto ident = 
    wsp::seq<
        ident_details::alpha_character,
        wsp::many<ident_details::alpha_numeric_character, std::basic_string>
    > [ wsp::map<ident_details::join_success> ] [ wsp::map_err<details::unwrap<predictions_t>> ];





namespace space_details {
    bool is_space(char c) {
        return c == ' ' || c == '\n' || c == '\r' || c == '\t';
    }

    predictions_t predictions(wsp::error::PredicateFailure) {
        return {{ "" }};
    }
}

// Parser<char, wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile>>
constexpr auto space = wsp::nextc [ wsp::filter<space_details::is_space> ];
// Parser<std::vector<char>>
constexpr auto many_spaces = wsp::many<space>;
// Parser<std::vector<char>, wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile>>
constexpr auto some_spaces = wsp::some<space>;
// Parser<wsp::error::EndOfFile, char>
constexpr auto end = wsp::seq<many_spaces, wsp::inverse<wsp::nextc>> [ wsp::peek<1> ] [ wsp::map_err<details::unwrap<char>> ];






namespace details {

    template<typename T>
    std::vector<T> push_front(wsp::Product<T, std::vector<T>> p) {
        auto v = std::move(std::get<1>(p));
        v.emplace(std::begin(v), std::move(std::get<0>(p)));
        return v;
    }

    template<typename T>
    std::vector<T> empty_vector() {
        return {};
    }

    predictions_t map_err_list_of(
        wsp::Sum<
            wsp::Sum<wsp::error::NotMatching<'['>, wsp::error::EndOfFile>, 
            wsp::Product<
                wsp::Sum<wsp::error::NotMatching<']'>, wsp::error::EndOfFile>,
                wsp::Sum<
                    wsp::Sum<wsp::error::NotMatching<','>, wsp::error::EndOfFile>,
                    predictions_t
                >
            >
        > err) 
    {
        // Handle error for `wsp::ch<'['>`
        if (err.index() == 0) {
            auto err0 = std::move(std::get<0>(err));
            if (std::holds_alternative<wsp::error::EndOfFile>(err0)) {
                return {{ "[]" }};
            } else {
                return {};
            }
        }

        /*
            err1 ::
                wsp::Product<
                    wsp::Sum<wsp::error::NotMatching<']'>, wsp::error::EndOfFile>,
                    wsp::Sum<
                        wsp::Sum<wsp::error::NotMatching<','>, wsp::error::EndOfFile>,
                        predictions_t
                    >
                >
        */
        auto err1 = std::move(std::get<1>(err));
        // Handle error for `wsp::ch<']'>`
        if (std::holds_alternative<wsp::error::EndOfFile>(std::get<0>(err1))) {
            return {{ "]" }};
        }

        // Handle error for `wsp::ch<','>`
        // Cannot be EOF or the previous if would have triggered
        if (std::holds_alternative<wsp::Sum<wsp::error::NotMatching<','>, wsp::error::EndOfFile>>(std::get<1>(err1))) {
            return {};
        }

        return std::get<1>(std::get<1>(std::move(err1)));
    }

    template<auto P, char sep = ','>
    constexpr auto separated_by = 
        wsp::seq<
            P,
            wsp::many<
                wsp::seq<
                    many_spaces,
                    wsp::ch<sep>, 
                    many_spaces,
                    P
                > [ wsp::peek<3> ]
            >
        > 
            [ wsp::map<push_front<wsp::details::parsed_type_t<decltype(P)>>> ]
            [ wsp::or_elsef<empty_vector<wsp::details::parsed_type_t<decltype(P)>>> ];


    template<auto P>
    constexpr auto list_of = 
        wsp::seq<
            wsp::ch<'['>,
            many_spaces,
            separated_by<P>,
            many_spaces,
            wsp::first<wsp::ch<']'>, wsp::seq<wsp::ch<','>, many_spaces, P>>
        > 
            [ wsp::peek<2> ]
            [ wsp::map_err<map_err_list_of> ];

}

}