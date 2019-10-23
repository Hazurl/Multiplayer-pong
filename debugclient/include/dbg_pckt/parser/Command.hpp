#pragma once

#include <dbg_pckt/parser/common.hpp>
#include <dbg_pckt/parser/ChangeUsername.hpp>
#include <dbg_pckt/parser/CreateRoom.hpp>
#include <dbg_pckt/parser/EnterRoom.hpp>
#include <dbg_pckt/parser/LeaveRoom.hpp>
#include <dbg_pckt/parser/EnterRoomResponse.hpp>
#include <dbg_pckt/parser/Input.hpp>
#include <dbg_pckt/parser/LobbyInfo.hpp>
#include <dbg_pckt/parser/NewRoom.hpp>
#include <dbg_pckt/parser/NewUser.hpp>
#include <dbg_pckt/parser/OldRoom.hpp>
#include <dbg_pckt/parser/OldUser.hpp>
#include <dbg_pckt/parser/RoomInfo.hpp>
#include <dbg_pckt/parser/ChangeUsernameResponse.hpp>
#include <dbg_pckt/parser/NewPlayer.hpp>
#include <dbg_pckt/parser/OldPlayer.hpp>
#include <dbg_pckt/parser/Abandon.hpp>
#include <dbg_pckt/parser/EnterQueue.hpp>
#include <dbg_pckt/parser/LeaveQueue.hpp>
#include <dbg_pckt/parser/BePlayer.hpp>
#include <dbg_pckt/parser/GameState.hpp>

#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace command_details {

    constexpr std::string_view change_username_str = "changeusername";
    constexpr std::string_view game_state_str = "gamestate";
    constexpr std::string_view create_room_str = "createroom";
    constexpr std::string_view enter_room_str = "enterroom";
    constexpr std::string_view enter_room_response_str = "enterroomresponse";
    constexpr std::string_view input_str = "input";
    constexpr std::string_view leave_room_str = "leaveroom";
    constexpr std::string_view lobby_info_str = "lobbyinfo";
    constexpr std::string_view new_room_str = "newroom";
    constexpr std::string_view new_user_str = "newuser";
    constexpr std::string_view old_room_str = "oldroom";
    constexpr std::string_view old_user_str = "olduser";
    constexpr std::string_view room_info_str = "roominfo";
    constexpr std::string_view username_response_str = "usernameresponse";
    constexpr std::string_view new_player_str = "newplayer";
    constexpr std::string_view old_player_str = "oldplayer";
    constexpr std::string_view be_player_str = "beplayer";
    constexpr std::string_view abandon_str = "abandon";
    constexpr std::string_view enter_queue_str = "enterqueue";
    constexpr std::string_view leave_queue_str = "leavequeue";

    template<auto& str>
    bool is_keyword(std::string ident) {
        std::transform(std::begin(ident), std::end(ident), std::begin(ident), ::tolower);
        return str == ident;
    }

    predictions_t predictions(std::string ident) {
        std::transform(std::begin(ident), std::end(ident), std::begin(ident), ::tolower);
        predictions_t preds;

#define check(str) \
        { \
            if(str.starts_with(ident)) { \
                auto tmp = str; \
                tmp.remove_prefix(ident.size()); \
                preds.emplace_back(tmp); \
            } \
        }

        check(change_username_str)
        check(game_state_str)
        check(create_room_str)
        check(enter_room_str)
        check(enter_room_response_str)
        check(input_str)
        check(leave_room_str)
        check(lobby_info_str)
        check(new_room_str)
        check(new_user_str)
        check(old_room_str)
        check(old_user_str)
        check(room_info_str)
        check(username_response_str)
        check(new_player_str)
        check(old_player_str)
        check(be_player_str)
        check(abandon_str)
        check(enter_queue_str)
        check(leave_queue_str)

#undef check

        return preds;

    }

    predictions_t all_predictions(predictions_t const& preds) {
        if (preds.empty()) {
            return {};
        }

        return predictions("");
    }

    predictions_t predictions_and_no_branch(wsp::Sum<wsp::error::NoBranch<std::string>, predictions_t> err) {
        if (auto b = std::get_if<0>(&err)) {
            return predictions(b->value);
        } else {
            return std::move(std::get<1>(err));
        }
    }

    predictions_t end_predictions(wsp::Sum<predictions_t, char> err) {
        if (std::holds_alternative<char>(err)) {
            return {};
        }

        return std::move(std::get<0>(err));
    }

}

constexpr auto command = 
    wsp::seq<
        many_spaces,
        wsp::switcher<
            ident [ wsp::map_err<command_details::all_predictions> ],

#define br(p) wsp::branchf<command_details::is_keyword<command_details::p ## _str>, p>

            br(change_username),
            br(game_state),
            br(create_room),
            br(enter_room),
            br(enter_room_response),
            br(input),
            br(leave_room),
            br(lobby_info),
            br(new_room),
            br(new_user),
            br(old_room),
            br(old_user),
            br(room_info),
            br(username_response),
            br(new_player),
            br(old_player),
            br(be_player),
            br(abandon),
            br(enter_queue),
            br(leave_queue)

#undef br

        >   [ wsp::map_err<command_details::predictions_and_no_branch> ],
        end
    > 
        [ wsp::peek<1> ]
        [ wsp::map_err<command_details::end_predictions> ];

}