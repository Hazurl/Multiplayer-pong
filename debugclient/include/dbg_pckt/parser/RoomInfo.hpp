#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace room_info_details {

    auto make_room_info(wsp::Product<std::string, std::string, std::vector<std::string>> p) {
        return pong::packet::RoomInfo{ std::move(std::get<0>(p)), std::move(std::get<1>(p)), std::move(std::get<2>(p)) };
    }

}

constexpr auto room_info = 
    wsp::seq<
        many_spaces, 
        string,
        many_spaces, 
        string,
        many_spaces, 
        details::list_of<string>
    > 
        [ wsp::select<1, 3, 5> ] [ wsp::map<room_info_details::make_room_info> ]
        [ wsp::map_err<details::unwrap<predictions_t>> ];
}