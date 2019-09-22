#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace old_user_details {

    auto make_old_user(std::string user) {
        return pong::packet::OldUser{ user };
    }

}

constexpr auto old_user = 
    wsp::seq<
        many_spaces, 
        string
    > 
        [ wsp::peek<1> ] [ wsp::map<old_user_details::make_old_user> ]
        [ wsp::map_err<details::unwrap<predictions_t>> ];
}