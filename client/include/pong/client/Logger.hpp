#pragma once

#include <sftk/print/Printer.hpp>

namespace tinge {
    using sftk::operator<<;
}

#include <tinge/tinge.hpp>

#define _LOGGER_PREFIX ::tinge::fg::reset, ::tinge::fg::white, '[', __FILE__, ':', __LINE__, ", ", ::tinge::fg::blue, __PRETTY_FUNCTION__, ::tinge::fg::white, "] ", ::tinge::fg::reset, ::tinge::style::reset, '\n', '\t'

#define NOTICE(...) (::tinge::println(::tinge::style::bold, ::tinge::fg::white, tinge::detail::symbol::notice, _LOGGER_PREFIX, __VA_ARGS__))
#define WARN(...) (::tinge::errln(::tinge::style::bold, ::tinge::fg::yellow, tinge::detail::symbol::warn, _LOGGER_PREFIX, __VA_ARGS__))
#define ERROR(...) (::tinge::errln(::tinge::style::bold, ::tinge::fg::red, tinge::detail::symbol::error, _LOGGER_PREFIX, __VA_ARGS__))
#define SUCCESS(...) (::tinge::println(::tinge::style::bold, ::tinge::fg::green, tinge::detail::symbol::success, _LOGGER_PREFIX, __VA_ARGS__))
