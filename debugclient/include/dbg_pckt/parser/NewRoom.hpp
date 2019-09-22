#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace new_room_details {

    auto make_new_room(int id) {
        return pong::packet::NewRoom{ id };
    }

    predictions_t no_spaces_predictions(wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return {{ " 0" }};
        }

        return {};
    }

}

constexpr auto new_room = 
    wsp::seq<
        some_spaces [ wsp::map_err<new_room_details::no_spaces_predictions> ], 
        integer
    > 
        [ wsp::peek<1> ] [ wsp::map<new_room_details::make_new_room> ]
        [ wsp::map_err<details::unwrap<predictions_t>> ];
}