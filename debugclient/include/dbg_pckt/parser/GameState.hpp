#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace game_state_details {
    auto make_game_state(wsp::Product<pong::Ball, pong::Pad, pong::Pad> p) {
        return pong::packet::GameState{ 
            std::move(std::get<0>(p)), 
            std::move(std::get<1>(p)), 
            std::move(std::get<2>(p)) 
        };
    }

    sf::Vector2f make_vector2f(wsp::Product<float, float> p) {
        return { std::get<0>(p), std::get<1>(p) };
    }

    pong::Ball make_ball(wsp::Product<sf::Vector2f, sf::Vector2f> p) {
        return { std::get<0>(p), std::get<1>(p) };
    }

    pong::Pad make_pad(wsp::Product<float, float> p) {
        return { std::get<0>(p), std::get<1>(p) };
    }

    template<char C>
    predictions_t predictions_of_char(wsp::Sum<wsp::error::NotMatching<C>, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return { std::string(1, C) };
        }

        return {};
    }

    template<char C>
    constexpr auto ch_with_predicitons = wsp::ch<C> [ wsp::map_err<predictions_of_char<C>> ];

    constexpr auto vector2f = 
        wsp::seq<
            ch_with_predicitons<'('>,
            many_spaces,
            number,
            many_spaces,
            ch_with_predicitons<','>,
            many_spaces,
            number,
            many_spaces,
            ch_with_predicitons<')'>
        > [ wsp::select<2, 6> ] [ wsp::map<make_vector2f> ] [ wsp::map_err<details::unwrap<predictions_t>> ];

    template<auto& str>
    predictions_t ident_default_predictions(predictions_t ident_preds) {
        if (ident_preds.empty()) { 
            return {}; 
        }

        return { std::string{ str }};
    }

    template<auto& str>
    wsp::Result<std::string, predictions_t> validate_ident(std::string ident) {
        std::transform(std::begin(ident), std::end(ident), std::begin(ident), ::tolower);
        std::string_view str_mut = str;
        if(str_mut.starts_with(ident) || ident.empty()) {
            if (ident.size() == str_mut.size()) {
                return wsp::success(0, ident);
            } else {
                str_mut.remove_prefix(ident.size());
                return wsp::fail(0, predictions_t{ std::string{ str_mut }});
            }
        }
        return wsp::fail(0, predictions_t{});
    }

    predictions_t in_pad_no_spaces_predictions(wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return {{ " 0" }};
        }

        return {};
    }

    predictions_t before_pad_no_spaces_predictions(wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return {{ " pad" }};
        }

        return {};
    }

    predictions_t before_ball_no_spaces_predictions(wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return {{ " ball" }};
        }

        return {};
    }


    constexpr std::string_view ball_ident = "ball";
    constexpr std::string_view pad_ident = "pad";

    constexpr auto ball = 
        wsp::seq<
            ident [ wsp::map_err<ident_default_predictions<ball_ident>> ][ wsp::bind<validate_ident<ball_ident>> ] [ wsp::map_err<details::unwrap<predictions_t>> ],
            many_spaces,
            ch_with_predicitons<':'>,
            many_spaces,
            vector2f,
            many_spaces,
            vector2f
        > [ wsp::select<4, 6> ] [ wsp::map<make_ball> ] [ wsp::map_err<details::unwrap<predictions_t>> ];

    constexpr auto pad = 
        wsp::seq<
            ident [ wsp::map_err<ident_default_predictions<pad_ident>> ][ wsp::bind<validate_ident<pad_ident>> ] [ wsp::map_err<details::unwrap<predictions_t>> ],
            many_spaces,
            ch_with_predicitons<':'>,
            many_spaces,
            number,
            some_spaces [ wsp::map_err<game_state_details::in_pad_no_spaces_predictions> ],
            number
        > [ wsp::select<4, 6> ] [ wsp::map<make_pad> ] [ wsp::map_err<details::unwrap<predictions_t>> ];

}

constexpr auto game_state = 
    wsp::seq<
        some_spaces [ wsp::map_err<game_state_details::before_ball_no_spaces_predictions> ],
        game_state_details::ball,
        many_spaces, 
        game_state_details::pad,
        some_spaces [ wsp::map_err<game_state_details::before_pad_no_spaces_predictions> ], 
        game_state_details::pad
    > 
        [ wsp::select<1, 3, 5> ] [ wsp::map<game_state_details::make_game_state> ]
        [ wsp::map_err<details::unwrap<predictions_t>> ];
}
