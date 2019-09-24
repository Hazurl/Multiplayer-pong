#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

constexpr auto leave_queue = wsp::unit<pong::packet::LeaveQueue{}>;

}