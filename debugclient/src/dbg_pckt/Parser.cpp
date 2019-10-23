#include <dbg_pckt/parser/Command.hpp>
#include <dbg_pckt/Parser.hpp>
#include <dbg_pckt/PacketToString.hpp>

#include <wpr/Readers.hpp>
#include <wpr/Test.hpp>

namespace dbg_pckt::parser {

ParserResult parse_command(std::string const& str) {
    wpr::BoundedReader reader(str);
    return decltype(dbg_pckt::parser::command)::parse(reader);
}

bool test_parsers() {
    using namespace wpr::test;

    Tester tester;

/*
    test_parser<string_parser_t>("string_parser_t", "");
    test_parser<string_parser_t>("string_parser_t", "a");
    test_parser<string_parser_t>("string_parser_t", "'");
    test_parser<string_parser_t>("string_parser_t", "\"");
    test_parser<string_parser_t>("string_parser_t", "'ok");
    test_parser<string_parser_t>("string_parser_t", "\"ok");
    test_parser<string_parser_t>("string_parser_t", "'ok'");
    test_parser<string_parser_t>("string_parser_t", "'\\'ok\\''");
    test_parser<string_parser_t>("string_parser_t", "\"ok\"");
    test_parser<string_parser_t>("string_parser_t", "\"\\\"ok\\\"\"");
*/

    tester += it("String", string) 
        > should_be(std::string{ "ok" })
            >> on("'ok'")
            >> on("\"ok\"")

        > should_be(std::string{ "o'k" })
            >> on("'o\\'k'")
            >> on("\"o'k\"")

        > should_be(std::string{ "o\"k" })
            >> on("'o\"k'")
            >> on("\"o\\\"k\"")

        > should_fail(predictions_t{{"\"\""}, {"''"}})
            >> on("")

        > should_fail(predictions_t{{"\""}})
            >> on("\"")
            >> on("\"abc")
            >> on("\"abc\\\"")

        > should_fail(predictions_t{{"'"}})
            >> on("'")
            >> on("'abc")
            >> on("'abc\\'")

        > should_fail(predictions_t{})
            >> on("a", 0)
    ;

/*
    test_parser<number_parser_t>("number_parser_t", "");
    test_parser<number_parser_t>("number_parser_t", "a");
    test_parser<number_parser_t>("number_parser_t", "123456");
    test_parser<number_parser_t>("number_parser_t", "123.");
    test_parser<number_parser_t>("number_parser_t", "123.456");
*/

    tester += it("Number", number) 
        > should_be(123.f)
            >> on("123")
            >> on("123.0")

        > should_be(123.456f)
            >> on("123.456")

        > should_fail(predictions_t{{"0"}})
            >> on("123.")
            >> on("")

        > should_fail(predictions_t{})
            >> on("a", 0)
            >> on(".", 0)
    ;

/*
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "]");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[]");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[   ]");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[,]");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[[");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[123");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[123,");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[123]");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[123,]");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[123,456,789]");
    test_parser<list_of_integer_parser_t>("list_of_integer_parser_t", "[ 123 , 456 , 789 ]");
*/

    tester += it("List of integers", dbg_pckt::parser::details::list_of<dbg_pckt::parser::integer>) 
        > should_be(std::vector<int>{})
            >> on("[]")
            >> on("[ \t]")

        > should_be(std::vector<int>{ 123 })
            >> on("[123]")
            >> on("[\t 123 \t]")

        > should_be(std::vector<int>{ 123, 256 })
            >> on("[123, 256]")
            >> on("[\t 123 \t,   256 ]")

        > should_fail(predictions_t{"[]"})
            >> on("")

        > should_fail(predictions_t{"0"})
            >> on("[123,")

        > should_fail(predictions_t{"]"})
            >> on("[")
            >> on("[123")
    ;
/*
    test_parser<ident_parser_t>("ident_parser_t", "");
    test_parser<ident_parser_t>("ident_parser_t", "1");
    test_parser<ident_parser_t>("ident_parser_t", "a");
    test_parser<ident_parser_t>("ident_parser_t", "abc_123");
*/

    tester += it("Identifier", ident) 
        > should_be(std::string{ "abc_123" })
            >> on("abc_123")

        > should_fail(predictions_t{})
            >> on("1", 0)
            >> on("[", 0)

        > should_fail(predictions_t{"_"})
            >> on("")
    ;

/*
    test_parser<change_username_parser_t>("change_username_parser_t", "");
    test_parser<change_username_parser_t>("change_username_parser_t", "\ta");
    test_parser<change_username_parser_t>("change_username_parser_t", "\t");
    test_parser<change_username_parser_t>("change_username_parser_t", "'");
    test_parser<change_username_parser_t>("change_username_parser_t", "\t'");
    test_parser<change_username_parser_t>("change_username_parser_t", "\"");
    test_parser<change_username_parser_t>("change_username_parser_t", "\t\"");
    test_parser<change_username_parser_t>("change_username_parser_t", "'something'");
    test_parser<change_username_parser_t>("change_username_parser_t", "\t'something'");
*/

    tester += it("Change username", change_username) 
        > should_be(pong::packet::ChangeUsername{ "abc_123" })
            >> on("'abc_123'")
            >> on(" 'abc_123'")
            >> on(" \"abc_123\"")

        > should_fail(predictions_t{})
            >> on("1", 0)
            >> on("[", 0)
            >> on(" 1", 1)
            >> on(" [", 1)

        > should_fail(predictions_t{{"\"\""}, {"''"}})
            >> on(" ")
            >> on("")
    ;


/*
    test_parser<old_user_parser_t>("old_user_parser_t", "");
    test_parser<old_user_parser_t>("old_user_parser_t", "\ta");
    test_parser<old_user_parser_t>("old_user_parser_t", "\t'");
    test_parser<old_user_parser_t>("old_user_parser_t", "\t'something'");
*/

    tester += it("Old user", old_user) 
        > should_be(pong::packet::OldUser{ "abc_123" })
            >> on("'abc_123'")
            >> on(" 'abc_123'")
            >> on(" \"abc_123\"")

        > should_fail(predictions_t{})
            >> on("1", 0)
            >> on("[", 0)
            >> on(" 1", 1)
            >> on(" [", 1)

        > should_fail(predictions_t{{"\"\""}, {"''"}})
            >> on(" ")
            >> on("")
    ;

/*
    test_parser<new_user_parser_t>("new_user_parser_t", "");
    test_parser<new_user_parser_t>("new_user_parser_t", "\ta");
    test_parser<new_user_parser_t>("new_user_parser_t", "\t'");
    test_parser<new_user_parser_t>("new_user_parser_t", "\t'something'");
*/
    tester += it("New user", new_user) 
        > should_be(pong::packet::NewUser{ "abc_123" })
            >> on("'abc_123'")
            >> on(" 'abc_123'")
            >> on(" \"abc_123\"")

        > should_fail(predictions_t{})
            >> on("1", 0)
            >> on("[", 0)
            >> on(" 1", 1)
            >> on(" [", 1)

        > should_fail(predictions_t{{"\"\""}, {"''"}})
            >> on(" ")
            >> on("")
    ;

/*
    test_parser<room_info_parser_t>("room_info_parser_t", "");
    test_parser<room_info_parser_t>("room_info_parser_t", "\ta");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something'");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' @");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'ok");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'ok\\'");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'name'");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'name' [");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'name' ]");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'name' []");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'name' [\"test\"");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'name' [\"test\"]");
    test_parser<room_info_parser_t>("room_info_parser_t", "\t'something' 'name' [\"test\", 'okay :)']");
*/

    tester += it("Room info", room_info) 
        > should_be(pong::packet::RoomInfo{ "abc_123", "something", { "blabla" } })
            >> on("'abc_123' 'something' ['blabla']")
            >> on(" 'abc_123' 'something' ['blabla']")
            >> on("\t\t'abc_123' \"something\" [ 'blabla'    \t]")

        > should_fail(predictions_t{})
            >> on("1", 0)
            >> on("[", 0)
            >> on(" 1", 1)
            >> on(" [", 1)

        > should_fail(predictions_t{{ "[]" }})
            >> on(" 'abc_123' 'something' ")

        > should_fail(predictions_t{{ "]" }})
            >> on(" 'abc_123' 'something' [")

        > should_fail(predictions_t{{"\"\""}, {"''"}})
            >> on(" ")
            >> on("'abc_123'")
            >> on(" 'abc_123'")
            >> on(" 'abc_123' 'abc_123' ['blabla', ")
            >> on("")
    ;

/*
    test_parser<create_room_parser_t>("create_room_parser_t", "");
    test_parser<create_room_parser_t>("create_room_parser_t", "\ta");
    test_parser<create_room_parser_t>("create_room_parser_t", "\t");
*/

    tester += it("Create room", create_room) 
        > should_be(pong::packet::CreateRoom{})
            >> on("")
            >> on(" \t", 0)
    ;

/*
    test_parser<leave_room_parser_t>("leave_room_parser_t", "");
    test_parser<leave_room_parser_t>("leave_room_parser_t", "\ta");
    test_parser<leave_room_parser_t>("leave_room_parser_t", "\t");
*/
    tester += it("Leave room", leave_room) 
        > should_be(pong::packet::LeaveRoom{})
            >> on("")
            >> on(" \t", 0)
    ;

/*
    test_parser<enter_room_parser_t>("enter_room_parser_t", "");
    test_parser<enter_room_parser_t>("enter_room_parser_t", "\ta");
    test_parser<enter_room_parser_t>("enter_room_parser_t", "\t");
    test_parser<enter_room_parser_t>("enter_room_parser_t", "123");
    test_parser<enter_room_parser_t>("enter_room_parser_t", "\t123");
*/

    tester += it("Enter room", enter_room) 
        > should_be(pong::packet::EnterRoom{ 123 })
            >> on(" 123")

        > should_fail(predictions_t{{"0"}})
            >> on(" ")

        > should_fail(predictions_t{{" 0"}})
            >> on("")

        > should_fail(predictions_t{})
            >> on("a", 0)
            >> on(".", 0)
    ;

/*
    test_parser<new_room_parser_t>("new_room_parser_t", "");
    test_parser<new_room_parser_t>("new_room_parser_t", "\ta");
    test_parser<new_room_parser_t>("new_room_parser_t", "\t");
    test_parser<new_room_parser_t>("new_room_parser_t", "123");
    test_parser<new_room_parser_t>("new_room_parser_t", "\t123");
*/

    tester += it("New room", new_room) 
        > should_be(pong::packet::NewRoom{ 123 })
            >> on(" 123")

        > should_fail(predictions_t{{"0"}})
            >> on(" ")

        > should_fail(predictions_t{{" 0"}})
            >> on("")

        > should_fail(predictions_t{})
            >> on("a", 0)
            >> on(".", 0)
    ;

/*
    test_parser<old_room_parser_t>("old_room_parser_t", "");
    test_parser<old_room_parser_t>("old_room_parser_t", "\ta");
    test_parser<old_room_parser_t>("old_room_parser_t", "\t");
    test_parser<old_room_parser_t>("old_room_parser_t", "123");
    test_parser<old_room_parser_t>("old_room_parser_t", "\t123");
*/
    tester += it("Old room", old_room) 
        > should_be(pong::packet::OldRoom{ 123 })
            >> on(" 123")

        > should_fail(predictions_t{{"0"}})
            >> on(" ")

        > should_fail(predictions_t{{" 0"}})
            >> on("")

        > should_fail(predictions_t{})
            >> on("a", 0)
            >> on(".", 0)
    ;


/*
    test_parser<enter_room_response_parser_t>("enter_room_response_parser_t", "");
    test_parser<enter_room_response_parser_t>("enter_room_response_parser_t", "\t");
    test_parser<enter_room_response_parser_t>("enter_room_response_parser_t", "okay");
    test_parser<enter_room_response_parser_t>("enter_room_response_parser_t", "\ta");
    test_parser<enter_room_response_parser_t>("enter_room_response_parser_t", "\tokay");
    test_parser<enter_room_response_parser_t>("enter_room_response_parser_t", "\tfull");
    test_parser<enter_room_response_parser_t>("enter_room_response_parser_t", "\tinvalidid");
    test_parser<enter_room_response_parser_t>("enter_room_response_parser_t", "\toKAy");
*/

    tester += it("Enter room repsonse", enter_room_response) 
        > should_be(pong::packet::EnterRoomResponse{ pong::packet::EnterRoomResponse::Okay })
            >> on(" okay")

        > should_be(pong::packet::EnterRoomResponse{ pong::packet::EnterRoomResponse::Full })
            >> on(" fULL")

        > should_be(pong::packet::EnterRoomResponse{ pong::packet::EnterRoomResponse::InvalidID })
            >> on("\tInvalidID")

        > should_fail(predictions_t{})
            >> on("a", 0)
            >> on(".", 0)

        > should_fail(predictions_t{{ "okay" }, { "full" }, { "invalidid" }})
            >> on(" ")
            >> on("\t")

        > should_fail(predictions_t{{ " okay" }, { " full" }, { " invalidid" }})
            >> on("")
    ;

/*
    test_parser<username_response_parser_t>("username_response_parser_t", "");
    test_parser<username_response_parser_t>("username_response_parser_t", "\t");
    test_parser<username_response_parser_t>("username_response_parser_t", "okay");
    test_parser<username_response_parser_t>("username_response_parser_t", "\ta");
    test_parser<username_response_parser_t>("username_response_parser_t", "\tokay");
    test_parser<username_response_parser_t>("username_response_parser_t", "\tinvalIDCharacterS");
    test_parser<username_response_parser_t>("username_response_parser_t", "\tTOOLONG");
    test_parser<username_response_parser_t>("username_response_parser_t", "\ttooshort");
*/

    tester += it("Username response", username_response) 
        > should_be(pong::packet::ChangeUsernameResponse{ pong::packet::ChangeUsernameResponse::Okay })
            >> on(" okay")

        > should_be(pong::packet::ChangeUsernameResponse{ pong::packet::ChangeUsernameResponse::InvalidCharacters })
            >> on(" invalidCharacters")

        > should_be(pong::packet::ChangeUsernameResponse{ pong::packet::ChangeUsernameResponse::TooShort })
            >> on("\tTooShort")

        > should_be(pong::packet::ChangeUsernameResponse{ pong::packet::ChangeUsernameResponse::TooLong })
            >> on("\tTooLong")

        > should_fail(predictions_t{})
            >> on("a", 0)
            >> on(".", 0)

        > should_fail(predictions_t{{ "okay" }, { "invalidcharacters" }, { "tooshort" }, { "toolong" }})
            >> on(" ")
            >> on("\t")

        > should_fail(predictions_t{{ " okay" }, { " invalidcharacters" }, { " tooshort" }, { " toolong" }})
            >> on("")
    ;

/*
    test_parser<input_parser_t>("input_parser_t", "");
    test_parser<input_parser_t>("input_parser_t", "\t");
    test_parser<input_parser_t>("input_parser_t", "up");
    test_parser<input_parser_t>("input_parser_t", "\ta");
    test_parser<input_parser_t>("input_parser_t", "\tuP");
    test_parser<input_parser_t>("input_parser_t", "\tDowN");
    test_parser<input_parser_t>("input_parser_t", "\tIDLE");
*/

    tester += it("Input", input) 
        > should_be(pong::packet::Input{ pong::Input::Idle })
            >> on(" idle")

        > should_be(pong::packet::Input{ pong::Input::Up })
            >> on(" uP")

        > should_be(pong::packet::Input{ pong::Input::Down })
            >> on("\tDOWN")

        > should_fail(predictions_t{})
            >> on("a", 0)
            >> on(".", 0)

        > should_fail(predictions_t{{ "idle" }, { "up" }, { "down" }})
            >> on(" ")
            >> on("\t")

        > should_fail(predictions_t{{ " idle" }, { " up" }, { " down" }})
            >> on("")
    ;

/*
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "");
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "\t");
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "\t[][]");
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "\t[]");
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "\t[");
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "\t[][");
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "\t[   ]   [   ]");
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "\t['Zoid'][123]");
    test_parser<lobby_info_parser_t>("lobby_info_parser_t", "\t['Zoid', 'Hazurl'][123, 456, 789]");
*/

    tester += it("Lobby info", lobby_info) 
        > should_be(pong::packet::LobbyInfo{ { "blabla", "something" }, { 123, 456 } })
            >> on("['blabla', 'something'][123,456]")
            >> on(" ['blabla', 'something'][123,456]")
            >> on("\t\t[ \"blabla\"\t, \t \t'something'   ]\t\t [\t 123\t,  456 ] ", -1)

        > should_fail(predictions_t{})
            >> on("1", 0)
            >> on("a", 0)
            >> on(" 1", 1)
            >> on(" a", 1)

        > should_fail(predictions_t{{ "[]" }})
            >> on("")
            >> on(" ")
            >> on(" []")
            >> on("['something']")
            >> on("[] ")

        > should_fail(predictions_t{{ "]" }})
            >> on(" [")
            >> on("['something'")
            >> on("[][")
            >> on("['something'][123")
            >> on(" ['something'][123,456")

        > should_fail(predictions_t{{"\"\""}, {"''"}})
            >> on("['blabla', ")
    ;

/*
    test_parser<game_state_parser_t>("game_state_parser_t", "ball");
    test_parser<game_state_parser_t>("game_state_parser_t", "b");
    test_parser<game_state_parser_t>("game_state_parser_t", " ");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tbALl:");
    test_parser<game_state_parser_t>("game_state_parser_t", "");
    test_parser<game_state_parser_t>("game_state_parser_t", "\ta");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:(");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:()");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:)");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:(,");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:(1.2");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:(1.2,");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:(1.");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8 ");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8)");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) ");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) @");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) 0");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) Pad : 0");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) Pad : 0.");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) Pad : 1 ");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) Pad : 1 @");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) Pad : 1 5");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) Pad : 1 0 Pad : 0 0");
    test_parser<game_state_parser_t>("game_state_parser_t", "\tball:( 1.0 , 0.8) (0,0) Pad : 1 0.0 Pad    :    1.5 25.888");
*/

    tester += it("Ball", game_state_details::ball)
        > should_be(pong::Ball{{ 1, 2 },{ 3, 4 }})
            >> on("ball:(1, 2)(3, 4)")
            >> on("ball:(1.0, 2.0)(3.0, 4.0)")
            >> on("ball\t :\t (\t 1\t ,\t  2\t )\t (\t 3\t ,\t  4\t )\t ")

        > should_fail(predictions_t{{ "all" }})
            >> on ("B")
            >> on ("b")

        > should_fail(predictions_t{{ "ball" }})
            >> on ("")

        > should_fail(predictions_t{{ "l" }})
            >> on ("bal")

        > should_fail(predictions_t{{ ":" }})
            >> on ("ball")
            >> on ("ball \t ")

        > should_fail(predictions_t{{ "(" }})
            >> on ("ball:")
            >> on ("ball: \t ")
            >> on ("ball:(1,2)")
            >> on ("ball:(1,2) \t ")

        > should_fail(predictions_t{{ ")" }})
            >> on ("ball:(1,2")
            >> on ("ball:(1,2 \t ")
            >> on ("ball:(1,2)(3,4")
            >> on ("ball:(1,2)(3,4 \t ")

        > should_fail(predictions_t{{ "0" }})
            >> on ("ball:(")
            >> on ("ball:( \t ")
            >> on ("ball:(1,")
            >> on ("ball:(1,2)(")
            >> on ("ball:(1,2)(3,")
            >> on ("ball:(0.")
            >> on ("ball:( \t 0.")
            >> on ("ball:(1,0.")
            >> on ("ball:(1,2)(0.")
            >> on ("ball:(1,2)(3,0.")
    ;

    tester += it("Game state", game_state) 
        > should_be(pong::packet::GameState{ {{ 1, 2 }, { 3, 4 }}, { 5, 6 }, { 7, 8 }})
            >> on(" ball:(1, 2.0) (3.0, 4) pad: 5 6 pad: 7.0 8.0")
            >> on("\t\tball\t:\t(\t1\t,\t 2.0\t)\t (\t3.0\t,\t 4\t)\t pad\t:\t 5\t 6 \tpad\t: \t7.0\t 8.0\t")
            >> on("  BaLL   :   ( 001.000, 2.000) (03,4.000) pAD: 5.000 06 PAD:  07.0 8")

        > should_fail(predictions_t{ " ball" })
            >> on("")

        > should_fail(predictions_t{ "ball" })
            >> on(" ")

        > should_fail(predictions_t{ "ll" })
            >> on(" ba")
            >> on(" BA")
            >> on(" bA")

        > should_fail(predictions_t{ "pad" })
            >> on(" ball:(1, 2.0) (3.0, 4)")
            >> on(" ball:(1, 2.0) (3.0, 4)\t \t  ")
            >> on(" ball:(1, 2.0) (3.0, 4) pad: 5 6 \t ")

        > should_fail(predictions_t{ "ad" })
            >> on(" ball:(1, 2.0) (3.0, 4)p")
            >> on(" ball:(1, 2.0) (3.0, 4)\t \t  p")
            >> on(" ball:(1, 2.0) (3.0, 4) pad: 5 6 \t p")

        > should_fail(predictions_t{ ":" })
            >> on(" ball")
            >> on(" ball:(1, 2.0) (3.0, 4) pad")
            >> on(" ball:(1, 2.0) (3.0, 4) pad: 5 6 pad")

        > should_fail(predictions_t{ "(" })
            >> on(" ball:")
            >> on(" ball:\t ")
            >> on(" ball:(1, 2.0)")
            >> on(" ball:(1, 2.0) \t")

        > should_fail(predictions_t{ "0" })
            >> on(" ball:(")
            >> on(" ball:(0,")
            >> on(" ball:(1, 2.0)(")
            >> on(" ball:(1, 2.0) (")
            >> on(" ball:(1, 2.0) (3.0, 4) pad:")
            >> on(" ball:(1, 2.0) (3.0, 4) pad:0 ")
            >> on(" ball:(1, 2.0) (3.0, 4) pad: \t 0.0 0.")
            >> on(" ball:(1, 2.0) (3.0, 4) pad: 5 6 pad:")
            >> on(" ball:(1, 2.0) (3.0, 4) pad: 5 6 pad:0 ")
            >> on(" ball:(1, 2.0) (3.0, 4) pad: 5 6 pad: \t 0.0 0.")
    ;

/*
    test_parser<command_parser_t>("command_parser_t", "");
    test_parser<command_parser_t>("command_parser_t", "ChangeUsername 'hazurl'");
    test_parser<command_parser_t>("command_parser_t", "GameState ball:( 1.0 , 0.8) (0,0) Pad : 1 0.0 Pad    :    1.5 25.888");
    test_parser<command_parser_t>("command_parser_t", "GameState");
    test_parser<command_parser_t>("command_parser_t", "GameState ");
    test_parser<command_parser_t>("command_parser_t", "gAMe");
    test_parser<command_parser_t>("command_parser_t", "c");
    test_parser<command_parser_t>("command_parser_t", "ChangeUsernam");
    test_parser<command_parser_t>("command_parser_t", "@");
    test_parser<command_parser_t>("command_parser_t", "gAMe@");
*/

    tester += it("Command", command) 
        > should_be(pong::packet::ChangeUsername{ "hazurl" })
            >> on ("changeUsername 'hazurl'")
            >> on (" \t changeUsername 'hazurl'")
            >> on ("changeUsername 'hazurl' \t ")

        > should_be(pong::packet::GameState{{{ 1, 2 }, { 3, 4 }}, { 5, 6 }, { 7, 8 }})
            >> on("gamestate ball:(1, 2.0) (3.0, 4) pad: 5 6 pad: 7.0 8.0")

        > should_be(pong::packet::CreateRoom{})
            >> on ("\tcreateroom")

        > should_be(pong::packet::EnterRoom{ 123 })
            >> on ("enterroom 123 ")
            
        > should_be(pong::packet::EnterRoomResponse{ pong::packet::EnterRoomResponse::Okay })
            >> on ("EnterRoomResponse OKAY ")

        > should_be(pong::packet::Input{ pong::Input::Idle })
            >> on ("input Idle ")

        > should_be(pong::packet::LeaveRoom{})
            >> on ("\tLEAVEROOM")

        > should_be(pong::packet::LobbyInfo{{ "blabla", "something" }, { 123, 456 }})
            >> on ("LobbyInfo ['blabla',   \"something\" ][ 123, 456 ] ")

        > should_be(pong::packet::NewRoom{ 123 })
            >> on ("newroom 123 ")

        > should_be(pong::packet::NewUser{ "hazurl" })
            >> on ("newuser 'hazurl'")
            
        > should_be(pong::packet::OldRoom{ 123 })
            >> on ("OldRoom 123 ")

        > should_be(pong::packet::OldUser{ "hazurl" })
            >> on (" olduser'hazurl'")
            
        > should_be(pong::packet::RoomInfo{ "abc_123", "something", { "blabla" }})
            >> on ("RoomInfo 'abc_123' \"something\"['blabla'] ")

        > should_be(pong::packet::ChangeUsernameResponse{ pong::packet::ChangeUsernameResponse::Okay })
            >> on ("ChangeUsernameResponse OKAY ")

        > should_fail(predictions_t{ "eaveroom", "obbyinfo" })
            >> on ("l")
            >> on ("L")
            >> on ("    \t\t  L")

        > should_fail(predictions_t{ "room", "user" })
            >> on ("New")

        > should_be(pong::packet::RoomInfo{ "abc_123", "something", { "blabla" } })
            >> on("RoomInfo'abc_123' 'something' ['blabla']")
            >> on("RoomInfo 'abc_123' 'something' ['blabla']")
            >> on("RoomInfo\t\t'abc_123' \"something\" [ 'blabla'    \t]")

        > should_fail(predictions_t{})
            >> on("RoomInfo1", 0)
            >> on("RoomInfo[", 8)
            >> on("RoomInfo 1", 9)
            >> on("RoomInfo [", 9)

        > should_fail(predictions_t{{ "[]" }})
            >> on("RoomInfo 'abc_123' 'something' ")

        > should_fail(predictions_t{{ "]" }})
            >> on("RoomInfo 'abc_123' 'something' [")
            >> on("RoomInfo'abc_123' 'something' [")

        > should_fail(predictions_t{{"\"\""}, {"''"}})
            >> on("roominfo ")
            >> on("roominfo'abc_123'")
            >> on("roominfo 'abc_123'")
            >> on("roominfo 'abc_123' 'abc_123' ['blabla', ")
            >> on("roominfo")

    ;
    return tester.report();
}

}