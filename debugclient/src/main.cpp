#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

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
                in_decimal_part *= 10;
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

std::vector<Term> parse_terms(std::string const& input) {
    // TODO: wipe out this functions and call `parse_term` until it hit the end of the string
    // Good luck :D
    
    std::size_t idx{ 0 };
    std::size_t const size = input.size(); 

    std::vector<Term> terms;
    std::string word;

    bool quoted{ false };
    bool escaped{ true };



    while(idx < size) {
        auto c = input[idx++];

        if (quoted) {
            if (c == '"' && !escaped) {
                quoted = false;
                terms.emplace_back(string_t{ std::move(word) });
                word.clear();
                continue;
            }

            if (c == '\\' && !escaped) {
                escaped = true;
                continue;
            }

            escaped = false;
            word += c;
        }

        if (c == '[') {
            continue;
        }

        if (c == ']') {
            continue;
        }

        if (c == '(') {
            continue;
        }

        if (c == ')') {
            continue;
        }

        if (c == '{') {
            continue;
        }

        if (c == '}') {
            continue;
        }

        if (c == '"') {
            if (!word.empty()) {
                words.emplace_back(std::move(word));
                word.clear();
            }
            quoted = true;
            continue;
        }

        if(c == ' ') {
            if (!word.empty()) {
                words.emplace_back(std::move(word));
                word.clear();
            }

            continue;
        }

        word += c;
    }
/*
    std::cout << "------------\n";
    for(auto const& w : words) {
        std::cout << "`" << w << "`\n";
    }
*/
    return terms;
}

struct TextArea : sf::Drawable {

    TextArea(sf::Font& font, sf::String const& default_string, unsigned height, unsigned width) : default_text(default_string, font, height - 6), text("", font, height - 6), box({ static_cast<float>(width), static_cast<float>(height) }) {
        default_text.setColor(sf::Color{100, 100, 100});
        default_text.setOrigin({ -3, -1 });
        default_text.setStyle(sf::Text::Style::Italic);

        text.setColor(sf::Color::White);
        text.setOrigin({ -3, -1 });

        box.setFillColor(sf::Color::Black);
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2);
    }

    void set_position(sf::Vector2f const& position) {
        box.setPosition(position);
        text.setPosition(position);
        default_text.setPosition(position);
    }

    void move(sf::Vector2f const& disp) {
        return set_position(disp + get_position());
    }

    sf::Vector2f get_position() const {
        return box.getPosition();
    }

    /* Drawable */
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(box, states);
        if (text.getString().isEmpty()) {
            target.draw(default_text, states);
        }
        else {
            target.draw(text, states);
        }
    }


    sf::Text default_text;
    sf::Text text;
    sf::RectangleShape box;
};

int main(int argc, char** argv) {
    try {
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
        text_area.set_position({8, 568});

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
                            auto str = text_area.text.getString();
                            if (!str.isEmpty()) {
                                str.erase(str.getSize() - 1);
                                text_area.text.setString(str);
                            }
                        } 
                        else {
                            auto str = text_area.text.getString();
                            str.insert(str.getSize(), event.text.unicode);
                            text_area.text.setString(str);
                        }

                        parse_terms(text_area.text.getString().toAnsiString());

                        break;
                    }

                    case sf::Event::KeyPressed: {

                        break;
                    }

                    default: { break; }
                }
            }


            window.clear(sf::Color::Black);

            window.draw(text_area);

            window.display();
        }
    } catch(std::exception const& e) {
        std::cerr << "Unexpected exception: " << e.what() << '\n';
    }
}