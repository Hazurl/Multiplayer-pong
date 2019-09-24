#include <dbg_pckt/parser/common.hpp>
#include <multipong/Packets.hpp>

namespace dbg_pckt::parser {

namespace wsp = wpr;

namespace new_player_details {

    wsp::Result<pong::packet::NewPlayer::Side, predictions_t> validate_result(std::string res) {
        std::transform(std::begin(res), std::end(res), std::begin(res), ::tolower);
        predictions_t preds;

#define check(s, r) \
        { \
            std::string_view str = s; \
            if(str.starts_with(res)) { \
                if (res.size() == str.size()) { \
                    return wsp::success(0, pong::packet::NewPlayer::Side::r); \
                } else { \
                    str.remove_prefix(res.size()); \
                    preds.emplace_back(str); \
                } \
            } \
        }

        check("left", Left)
        check("right", Right)

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

    auto make_new_player(wsp::Product<pong::packet::NewPlayer::Side, std::string> p) {
        auto&&[side, username] = std::move(p.as_tuple());
        return pong::packet::NewPlayer{ side, std::move(username) };
    }
}

constexpr auto new_player = 
    wsp::seq<
        some_spaces [ wsp::map_err<new_player_details::no_spaces_predictions> ], 
        ident [ wsp::handler<new_player_details::empty_string> ] [ wsp::bind<new_player_details::validate_result> ],
        many_spaces,
        string
    > 
        [ wsp::select<1, 3> ] 
        [ wsp::map<new_player_details::make_new_player> ]

        [ wsp::map_err<details::unwrap<predictions_t>> ];

}
