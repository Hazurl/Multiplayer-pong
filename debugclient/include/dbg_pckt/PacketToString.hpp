#pragma once

#include <multipong/Packets.hpp>

#include <wpr/Details.hpp>

#include <string>

namespace dbg_pckt {

std::string packet_to_string(pong::packet::ChangeUsername const& packet);
std::string packet_to_string(pong::packet::GameState const& packet);
std::string packet_to_string(pong::packet::CreateRoom const& packet);
std::string packet_to_string(pong::packet::EnterRoom const& packet);
std::string packet_to_string(pong::packet::EnterRoomResponse const& packet);
std::string packet_to_string(pong::packet::Input const& packet);
std::string packet_to_string(pong::packet::LeaveRoom const& packet);
std::string packet_to_string(pong::packet::LobbyInfo const& packet);
std::string packet_to_string(pong::packet::NewRoom const& packet);
std::string packet_to_string(pong::packet::NewUser const& packet);
std::string packet_to_string(pong::packet::OldRoom const& packet);
std::string packet_to_string(pong::packet::OldUser const& packet);
std::string packet_to_string(pong::packet::RoomInfo const& packet);
std::string packet_to_string(pong::packet::ChangeUsernameResponse const& packet);
std::string packet_to_string(pong::packet::NewPlayer const& packet);
std::string packet_to_string(pong::packet::OldPlayer const& packet);
std::string packet_to_string(pong::packet::BePlayer const& packet);
std::string packet_to_string(pong::packet::Abandon const& packet);
std::string packet_to_string(pong::packet::EnterQueue const& packet); 
std::string packet_to_string(pong::packet::LeaveQueue const& packet);

}

namespace wpr {

template<typename T>
struct Describe<T, std::void_t<
    decltype(::dbg_pckt::packet_to_string(std::declval<T>()))
>> {
    std::string operator()(T const& t) {
        return ::dbg_pckt::packet_to_string(t);
    }
};

}