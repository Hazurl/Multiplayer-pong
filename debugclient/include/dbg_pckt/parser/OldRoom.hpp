#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace old_room_details {
    auto make_old_room(int id) {
        return pong::packet::OldRoom{ id };
    }

    predictions_t no_spaces_predictions(wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            return {{ " 0" }};
        }

        return {};
    }

}

constexpr auto old_room = 
    wsp::seq<
        some_spaces [ wsp::map_err<old_room_details::no_spaces_predictions> ], 
        integer
    > 
        [ wsp::peek<1> ] [ wsp::map<old_room_details::make_old_room> ]
        [ wsp::map_err<details::unwrap<predictions_t>> ];
}