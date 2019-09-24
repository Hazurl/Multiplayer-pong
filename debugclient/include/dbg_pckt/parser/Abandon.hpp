#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

constexpr auto abandon = wsp::unit<pong::packet::Abandon{}>;

}