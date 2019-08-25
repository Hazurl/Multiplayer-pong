#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <optional>
#include <future>
#include <chrono>
#include <variant>
#include <deque>

#include <multipong/Game.hpp>
#include <multipong/Packets.hpp>

#include <SFML-Toolkit/include/sftk/fancyText/FancyText.hpp>

std::string to_string(float f) {
    auto str = std::to_string(f);
    if (str.find('.') != std::string::npos) {
        str.erase(std::begin(str) + (str.find_last_not_of('0') + 1), std::end(str));

        if (str.back() == '.') {
            str.erase(std::end(str) - 1);
        }
    }

    return str;
}

sf::String packet_to_string(pong::packet::ChangeUsername const& change_username) {
    return "ChangeUsername{ username: " + change_username.username + " }";
}

sf::String packet_to_string(pong::packet::UsernameResponse const& username_response) {
    std::string response{ "??? "};
    switch(username_response.result) {
        case pong::packet::UsernameResponse::Okay: response = "Okay"; break;
        case pong::packet::UsernameResponse::InvalidCharacters: response = "InvalidCharacters"; break;
        case pong::packet::UsernameResponse::TooShort: response = "TooShort"; break;
        case pong::packet::UsernameResponse::TooLong: response = "TooLong"; break;
    }
    return "UsernameResponse{ result: " + response + " }";
}

sf::String packet_to_string(pong::packet::LobbyInfo const& lobby_info) {
    sf::String res = "LobbyInfo{ users: {";
    if (!lobby_info.users.empty()) {
        res += " ";

        for(auto it = std::begin(lobby_info.users); it != std::end(lobby_info.users); ++it) {
            auto const& user = *it;
            res += user;
            if (it + 1 != std::end(lobby_info.users)) {
                res += ", ";
            }
        }

        res += " ";
    }

    res += "}, rooms: {";

    if (!lobby_info.rooms.empty()) {
        res += " ";

        for(auto it = std::begin(lobby_info.rooms); it != std::end(lobby_info.rooms); ++it) {
            auto const& room = *it;
            res += std::to_string(room);
            if (it + 1 != std::end(lobby_info.rooms)) {
                res += ", ";
            }
        }

        res += " ";
    }
    return res + "} }";
}

sf::String packet_to_string(pong::packet::NewUser const& new_user) {
    return "NewUser{ username: " + new_user.username + " }";
}

sf::String packet_to_string(pong::packet::OldUser const& old_user) {
    return "OldUser{ username: " + old_user.username + " }";
}

sf::String packet_to_string(pong::packet::NewRoom const& new_room) {
    return "NewRoom{ id: " + std::to_string(new_room.id) + " }";
}

sf::String packet_to_string(pong::packet::OldRoom const& old_room) {
    return "OldRoom{ id: " + std::to_string(old_room.id) + " }";
}

sf::String packet_to_string(pong::packet::EnterRoom const& enter_room) {
    return "EnterRoom{ id: " + std::to_string(enter_room.id) + " }";
}

sf::String packet_to_string(pong::packet::CreateRoom const&) {
    return "CreateRoom{}";
}

sf::String packet_to_string(pong::packet::EnterRoomResponse const& enter_room_response) {
    std::string response{ "??? "};
    switch(enter_room_response.result) {
        case pong::packet::EnterRoomResponse::Okay: response = "Okay"; break;
        case pong::packet::EnterRoomResponse::Full: response = "Full"; break;
        case pong::packet::EnterRoomResponse::InvalidID: response = "InvalidID"; break;
    }
    return "EnterRoomResponse{ result: " + response + " }";
}

sf::String packet_to_string(pong::packet::RoomInfo const& room_info) {
    sf::String res = "RoomInfo{ left_player: " + room_info.left_player;
    res += ", right_player: " + room_info.right_player + ", spectators: {";

    if (!room_info.spectators.empty()) {
        res += " ";

        for(auto it = std::begin(room_info.spectators); it != std::end(room_info.spectators); ++it) {
            auto const& spectator = *it;
            res += spectator;
            if (it + 1 != std::end(room_info.spectators)) {
                res += ", ";
            }
        }

        res += " ";
    }
    return res + "} }";

}

sf::String packet_to_string(pong::packet::LeaveRoom const&) {
    return "LeaveRoom{}";
}

sf::String packet_to_string(pong::packet::GameState const& game_state) {
    auto const& ball = game_state.ball;
    sf::String ball_str = "ball: { position: (" + to_string(ball.position.x) + ", " + to_string(ball.position.y) + "), speed: (";
    ball_str += to_string(ball.speed.x) + ", " + to_string(ball.speed.y) + ") }";

    auto pad_to_string = [] (pong::Pad const& pad) {
        return "pad: { y_position: " + to_string(pad.y) + ", speed: " + to_string(pad.speed) + " }";
    };

    auto left_str = "left_" + pad_to_string(game_state.left);
    auto right_str = "right_" + pad_to_string(game_state.right);

    return "GameState{ " + ball_str + ", " + left_str + ", " + right_str + " }";
}

sf::String packet_to_string(pong::packet::Input const& input) {
    std::string response{ "??? "};
    switch(input.input) {
        case pong::Input::Idle: response = "Idle"; break;
        case pong::Input::Up: response = "Up"; break;
        case pong::Input::Down: response = "Down"; break;
    }
    return "Input{ input: " + response + " }";
}

enum class packet_name_t {

};

enum class keyword_t {
    Okay, InvalidID, Full, InvalidCharacters, TooShort, TooLong, Idle, Up, Down
};

using vec2_t = sf::Vector2f;
using number_t = float;
using string_t = std::string;
using ball_t = pong::Ball;
using pad_t = pong::Pad;

struct Term : std::variant<packet_name_t, number_t, string_t, keyword_t, vec2_t, ball_t, pad_t, std::vector<Term>> {};

std::optional<Term> parse_term(std::string const& input, std::size_t& idx) {
    auto c = input[idx++];

    if(c == '"') {
        bool escaped{ true };
        std::string word;

        while(idx < input.size()) {
            c = input[idx++];

            if (c == '"' && !escaped) {
                return Term{ string_t{ std::move(word) } };
            }

            if (c == '\\' && !escaped) {
                escaped = true;
                continue;
            }

            // Support only " and \ escaped characters
            escaped = false;
            word += c;
        }

        // String not closed
        return std::nullopt;
    }

    if (c >= '0' && c <= '9') {
        float num{ c - '0' };
        float inverse_factor{ 1 };
        bool in_decimal_part{ false };
        while(idx < input.size()) {
            c = input[idx++];

            if (c == '.') {
                if (in_decimal_part) {
                    // Two decimal points in the number
                    return std::nullopt;
                }

                in_decimal_part = true;
                continue;
            }

            if (c < '0' || c > '9') {
                break;
            }

            num = num * 10 + (c - '0');
            if (in_decimal_part) {
                inverse_factor *= 10;
            }
        }

        return Term{ number_t{ num / inverse_factor } };
    }

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        std::string word(1, c);
        while(idx < input.size()) {
            c = input[idx++];
            if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z')) {
                break;
            }
            word += c;
        }

        // TODO: Make the lookup function `word -> keyword_t | packet_name_t`
        // return lookup_ident(word);
    }

    // TODO: Implement array, vec2 and objects like ball_t and pad_t

    // Symbol not supported
    return std::nullopt;
}

std::optional<std::string> parse_ident(std::string_view const& str, std::size_t& idx) {
    if (idx >= str.size()) return "";

    std::cout << "# `" << str << "`\n";

    std::string word;

    while(idx < str.size()) {        
        char c = str[idx];

        if (c == ' ') {
            break;
        }

        if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z')) {
            return std::nullopt;
        }

        word += c;
        ++idx;
    }

    return word;
}

std::optional<std::string> parse_string(std::string_view const& str, std::size_t& idx) {
    if (idx + 1 >= str.size() || (str[idx] != '"' && str[idx] != '\'')) return std::nullopt;

    char c = str[idx++];
    char quote = c;
    c = str[idx++];

    std::string word;
    while(c != quote) {
        if (idx >= str.size()) return std::nullopt;
        word += c;
        c = str[idx++];
    }

    ++idx;

    return word;
}

void skip_spaces(std::string_view const& str, std::size_t& idx) {
    while(idx < str.size() && str[idx] == ' ') { ++idx; std::cout << "||SKIP||\n"; }
}

struct CommandParsingResult {
    std::optional<std::variant<
        pong::packet::ChangeUsername, 
        pong::packet::UsernameResponse, 
        pong::packet::EnterRoom, 
        pong::packet::EnterRoomResponse, 
        pong::packet::LeaveRoom, 
        pong::packet::Input, 
        pong::packet::GameState, 
        pong::packet::NewUser, 
        pong::packet::OldUser, 
        pong::packet::CreateRoom, 
        pong::packet::NewRoom, 
        pong::packet::OldRoom, 
        pong::packet::LobbyInfo, 
        pong::packet::RoomInfo>> command;

    std::optional<std::size_t> index_of_error;
    std::vector<std::string> predictions;
};

CommandParsingResult parse_change_username(std::string_view const& str, std::size_t& idx) {
    {
        std::size_t idx_before_skip{ idx };
        skip_spaces(str, idx);

        if (idx >= str.size()) {
            return CommandParsingResult {
                std::nullopt,
                {},
                { idx_before_skip == idx ? " ''" : "''" }
            };
        }
    }

    char quote = str[idx];

    auto username = parse_string(str, idx);
    if (!username) {
        std::vector<std::string> predictions;
        if (quote == '\'' || quote == '"') {
            predictions.emplace_back(1, quote);
        }
        return CommandParsingResult {
            std::nullopt,
            predictions.empty() ? std::optional<std::size_t>(idx) : std::nullopt,
            predictions
        };
    }

    return CommandParsingResult {
        pong::packet::ChangeUsername{ *username },
        {},
        {}
    };

}

CommandParsingResult parse_username_response(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_enter_room(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_enter_room_response(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_room_info(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_leave_room(std::string_view const& str, std::size_t& idx) {
    std::size_t const initial_idx{ idx };
    while(idx < str.size()) {
        char c = str[idx++];
        if (c != ' ') {
            return CommandParsingResult {
                std::nullopt,
                { idx },
                {}
            };
        }
    }
    return CommandParsingResult {
        pong::packet::LeaveRoom{},
        {},
        {}
    };  
}

CommandParsingResult parse_create_room(std::string_view const& str, std::size_t& idx) {
    std::size_t const initial_idx{ idx };
    while(idx < str.size()) {
        char c = str[idx++];
        if (c != ' ') {
            return CommandParsingResult {
                std::nullopt,
                { idx },
                {}
            };
        }
    }
    return CommandParsingResult {
        pong::packet::CreateRoom{},
        {},
        {}
    };  
}

CommandParsingResult parse_new_user(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_old_user(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_new_room(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_old_room(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_lobby_info(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_input(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_game_state(std::string_view const& str, std::size_t& idx) {
    return CommandParsingResult {
        std::nullopt,
        { idx },
        {}
    };
}

CommandParsingResult parse_command(std::string_view const& str) {
    std::size_t idx{ 0 };

    auto name = parse_ident(str, idx);
    if (!name) {
        return CommandParsingResult {
            std::nullopt,
            { 0 },
            {}  
        };
    }

    std::cout << "~~ " << *name << '\n';

    std::pair<std::string_view, CommandParsingResult(*)(std::string_view const&, std::size_t&)> const names[] {
        { "ChangeUsername", parse_change_username }, 
        { "UsernameResponse", parse_username_response }, 
        { "EnterRoom", parse_enter_room }, 
        { "EnterRoomResponse", parse_enter_room_response }, 
        { "LeaveRoom", parse_leave_room }, 
        { "Input", parse_input }, 
        { "GameState", parse_game_state }, 
        { "NewUser", parse_new_user }, 
        { "OldUser", parse_old_user }, 
        { "CreateRoom", parse_create_room }, 
        { "NewRoom", parse_new_room }, 
        { "OldRoom", parse_old_room }, 
        { "LobbyInfo", parse_lobby_info }, 
        { "RoomInfo", parse_room_info }
    };

    std::vector<std::string> predictions;

    for(auto&&[n, f] : names) {
        if (n == *name) {
            return f(str, idx);
        }

        if (n.size() >= name->size() && n.substr(0, name->size()) == std::string_view{ *name }) {
            predictions.push_back(std::string{ n.substr(name->size()) });
        }
    }

    return CommandParsingResult {
        std::nullopt,
        predictions.empty() ? std::optional<std::size_t>(0) : std::nullopt,
        predictions  
    };
}

struct TextArea : sf::Drawable, sf::Transformable {

    TextArea(sf::Font& font, sf::String const& default_string, unsigned height, unsigned width) 
        : font{ font }, default_message{ default_string }
        , fancy_text{ sftk::TextBuilder{ font } << sftk::txt::size(height - 6) << sf::Color{ 100, 100, 100 } << default_message, sf::VertexBuffer::Usage::Dynamic }
        , box({ static_cast<float>(width), static_cast<float>(height) })
        , cursor({ font.getGlyph('X', height - 6, false, 0).bounds.width, 2 }) {

        box.setFillColor(sf::Color::Black);
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2);

        fancy_text.setOrigin(0, fancy_text.get_local_bounds().height / 2.f);
        fancy_text.setPosition(3, height / 2.f);

        cursor.setFillColor(sf::Color::White);
        cursor.setPosition(3, 17);
    }

    void update() {
        if (content.empty()) {
            fancy_text.set_text(sftk::TextBuilder{ font }
                << sftk::txt::size(box.getSize().y - 6)
                << sf::Color{ 100, 100, 100 }
                << default_message
            );

            cursor.setPosition(3, 17);

        }
        else {
            auto res = parse_command(content);
            auto pred = res.predictions.empty() ? "" : res.predictions.front();

            auto builder = sftk::TextBuilder{ font } << sftk::txt::size(box.getSize().y - 6);
            bool is_red{ false };

            float cursor_x{ 3 };
            std::cout << "INDEX: " << cursor_index << '\n';

            for(std::size_t i{ 0 }; i < content.size(); ++i) {
                bool is_in_error = res.index_of_error && i >= *res.index_of_error;
                if (is_in_error && !is_red) {
                    builder.set_fill_color(sf::Color::Red);
                } else if (!is_in_error && is_red) {
                    builder.set_fill_color(sf::Color::White);
                }
                builder.append(content[i]);

                if(i + 1 == cursor_index) {
                    cursor_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
                }
            }

            if (cursor_index == 0) {
                cursor_x = 3; 
            }
            else if (cursor_index >= content.size()) {
                cursor_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
                cursor_index = content.size();
            } 
            fancy_text.set_text(std::move(builder)
                << sf::Color{ 100, 100, 100 }
                << pred);

            cursor.setPosition(cursor_x, 17);
        }
    }

    /* Drawable */
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(box, states);

        //target.draw(prediction, states);
        //target.draw(text, states);
        target.draw(fancy_text, states);
        target.draw(cursor, states);
    }


    sf::Font& font;
    std::string const default_message;
    std::string content;
    sftk::FancyText fancy_text;
    sf::RectangleShape box;
    sf::RectangleShape cursor;
    std::size_t cursor_index{ 0 };
};

int main(int argc, char** argv) {
    /* --------------------------
        * INITIALISATION
        * --------------------------
        */
    /* Connect to server */ 
    /*
    unsigned short const default_port = 48622;
    unsigned short const port = argc >= 2 ? std::stoi(argv[1]) : default_port;
    auto socket = std::make_unique<sf::TcpSocket>();
    if (socket->connect("127.0.0.1", port) != sf::Socket::Done) {
        std::cerr << "Couldn't connect to port " << port << '\n';
        return 1;
    }
    socket->setBlocking(false);
    */
    /*
    std::cout << 
        packet_to_string(pong::packet::ChangeUsername{ "Hazurl" }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::UsernameResponse{ pong::packet::UsernameResponse::InvalidCharacters }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::LobbyInfo{ {"Hazurl", "Zoid", "Tintin"}, {0, 2, 3, 5} }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::NewUser{ "Hazurl" }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::OldUser{ "Hazurl" }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::NewRoom{ 1 }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::OldRoom{ 2 }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::EnterRoom{ 5 }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::CreateRoom{}).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::EnterRoomResponse{ pong::packet::EnterRoomResponse::InvalidID }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::RoomInfo{ "Hazurl", "Zoid", {"Tintin, Ziliq"} }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::LeaveRoom{}).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::GameState{ pong::Ball{{126, 150}, {10.123456789, -10}}, pong::Pad{188, -5.82}, pong::Pad{25, 12} }).toAnsiString() << '\n' <<
        packet_to_string(pong::packet::Input{ pong::Input::Down }).toAnsiString() << '\n';
    */
    
    /* Open windows */
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Multiplayer pong");
    /* Load assets */
    sf::Font font;
    if (!font.loadFromFile("../assets/neoletters.ttf")) {
        std::cout << "Couldn't load font '../assets/neoletters.ttf'\n";
        std::exit(1);
    }

    sf::Clock clock;

    TextArea text_area(font, "Type a packet to send it", 20, 780);
    text_area.setPosition({8, 568});

    /* --------------------------
        * MAIN LOOP
        * --------------------------
        */
    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            switch(event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                } 

                case sf::Event::MouseButtonReleased: {
                    if (event.mouseButton.button == sf::Mouse::Button::Left) {

                    }

                    break;
                }

                case sf::Event::TextEntered: {
                    //std::cout << "TextEntered: " << event.text.unicode << '\n';

                    if (event.text.unicode >= 128) { break; } // not ascii

                    if (event.text.unicode == 8) { // Delete
                        if (!text_area.content.empty() && text_area.cursor_index > 0) {
                            text_area.content.erase(--text_area.cursor_index, 1);
                        }
                    } 
                    else if (event.text.unicode == 127) { // Supr
                        if (text_area.cursor_index < text_area.content.size()) {
                            text_area.content.erase(text_area.cursor_index, 1);
                        }
                    } 
                    else {
                        if (event.text.unicode < 32 || event.text.unicode > 127) {
                            break;
                        }
                        text_area.content.insert(text_area.cursor_index, 1, event.text.unicode);
                        ++text_area.cursor_index;
                    }

                    text_area.update();

                    //parse_terms(text_area.text.getString().toAnsiString());

                    break;
                }

                case sf::Event::KeyPressed: {

                    if (event.key.code == sf::Keyboard::Enter) {
                        auto res = parse_command(text_area.content);
                        std::cout << (res.command ? std::visit([] (auto const& v) { return packet_to_string(v).toAnsiString(); }, *res.command) : "Not found") << '\n';
                        for(auto const& p : res.predictions) {
                            std::cout << "> " << p << '\n';
                        }
                        if(res.index_of_error) {
                            std::cout << "# " << *res.index_of_error << '\n';
                        }
                    } 
                    else if(event.key.code == sf::Keyboard::Left) {
                        std::cout << "LEFT\n";
                        if (text_area.cursor_index > 0) {
                            --text_area.cursor_index;
                            text_area.update();
                        }
                    }

                    else if(event.key.code == sf::Keyboard::Right) {
                        std::cout << "RIGHT\n";
                        if (text_area.cursor_index < text_area.content.size()) {
                            ++text_area.cursor_index;
                            text_area.update();
                        }
                    }

                    break;
                }

                default: { break; }
            }
        }


        window.clear(sf::Color::Black);

        window.draw(text_area);

        window.display();
    }
}
