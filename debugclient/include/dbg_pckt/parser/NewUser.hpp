#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace new_user_details {

    auto make_new_user(std::string user) {
        return pong::packet::NewUser{ user };
    }

}

constexpr auto new_user = 
    wsp::seq<
        many_spaces, 
        string
    > 
        [ wsp::peek<1> ] [ wsp::map<new_user_details::make_new_user> ]
        [ wsp::map_err<details::unwrap<predictions_t>> ];
}