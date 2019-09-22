#pragma once

#include <wpr/Containers.hpp>
#include <wpr/Details.hpp>

#include <multipong/Packets.hpp>

#include <string>
#include <vector>

namespace dbg_pckt::parser {

using ParserResult = wpr::Result<wpr::Sum<
    pong::packet::ChangeUsername,
    pong::packet::GameState,
    pong::packet::CreateRoom,
    pong::packet::EnterRoom,
    pong::packet::EnterRoomResponse,
    pong::packet::Input,
    pong::packet::LeaveRoom,
    pong::packet::LobbyInfo,
    pong::packet::NewRoom,
    pong::packet::NewUser,
    pong::packet::OldRoom,
    pong::packet::OldUser,
    pong::packet::RoomInfo,
    pong::packet::UsernameResponse
>, std::vector<std::string>>;


ParserResult parse_command(std::string const& str);
bool test_parsers();

}
