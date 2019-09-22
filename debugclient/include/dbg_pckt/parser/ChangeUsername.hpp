#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace change_username_details {
    auto make_change_username(std::string&& username) {
        return pong::packet::ChangeUsername{ std::move(username) };
    }
}

constexpr auto change_username = 
    wsp::seq<
        many_spaces, 
        string
    > 
        [ wsp::peek<1> ] [ wsp::map<change_username_details::make_change_username> ]
        [ wsp::map_err<details::unwrap<predictions_t>> ];
}