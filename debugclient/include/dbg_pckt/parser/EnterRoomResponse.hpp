#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace enter_room_response_details {

    wsp::Result<pong::packet::EnterRoomResponse::Result, predictions_t> validate_result(std::string res) {
        std::transform(std::begin(res), std::end(res), std::begin(res), ::tolower);
        predictions_t preds;

#define check(s, r) \
        { \
            std::string_view str = s; \
            if(str.starts_with(res)) { \
                if (res.size() == str.size()) { \
                    return wsp::success(0, pong::packet::EnterRoomResponse::Result::r); \
                } else { \
                    str.remove_prefix(res.size()); \
                    preds.emplace_back(str); \
                } \
            } \
        }

        check("okay", Okay)
        check("full", Full)
        check("invalidid", InvalidID)

#undef check

        return wsp::fail(0, preds);
    }

    std::string empty_string(predictions_t) {
        return "";
    }

    predictions_t no_spaces_predictions(wsp::Sum<wsp::error::PredicateFailure, wsp::error::EndOfFile> err) {
        if (std::holds_alternative<wsp::error::EndOfFile>(err)) {
            auto preds = validate_result("").error();
            for(auto& p : preds) {
                p.insert(0, 1, ' ');
            }
            return preds;
        }

        return {};
    }

    auto make_enter_room_response(pong::packet::EnterRoomResponse::Result r) {
        return pong::packet::EnterRoomResponse{ r };
    }
}

constexpr auto enter_room_response = 
    wsp::seq<
        some_spaces [ wsp::map_err<enter_room_response_details::no_spaces_predictions> ], 
        ident [ wsp::handler<enter_room_response_details::empty_string> ]
    > 
        [ wsp::map_err<details::unwrap<predictions_t>> ]

        [ wsp::peek<1> ] 
        [ wsp::bind<enter_room_response_details::validate_result> ] 
        [ wsp::map<enter_room_response_details::make_enter_room_response> ]

        [ wsp::map_err<details::unwrap<predictions_t>> ];

}
