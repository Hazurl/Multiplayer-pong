#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace lobby_info_details {
    auto make_lobby_info(wsp::Product<std::vector<std::string>, std::vector<int>> p) {
        return pong::packet::LobbyInfo{ std::move(std::get<0>(p)), std::move(std::get<1>(p)) };
    }
}

constexpr auto lobby_info = 
    wsp::seq<
        many_spaces, 
        details::list_of<string>,
        many_spaces,
        details::list_of<integer>
    > 
        [ wsp::select<1, 3> ] [ wsp::map<lobby_info_details::make_lobby_info> ]
        [ wsp::map_err<details::unwrap<predictions_t>> ];
}