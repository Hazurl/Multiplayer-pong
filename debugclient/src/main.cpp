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

#include <dbg_pckt/PacketToString.hpp>
#include <dbg_pckt/Parser.hpp>
#include <dbg_pckt/TextArea.hpp>
#include <dbg_pckt/CommandTextArea.hpp>

struct TextArea : sf::Drawable, sf::Transformable {

    TextArea(sf::Font& font, sf::String const& default_string, unsigned height, unsigned width) 
        : font{ font }, default_message{ default_string }
        , fancy_text{ sftk::TextBuilder{ font } << sftk::txt::size(height - 6) << sf::Color{ 100, 100, 100 } << default_message, sf::VertexBuffer::Usage::Dynamic }
        , box({ static_cast<float>(width), static_cast<float>(height) })
        , cursor({ font.getGlyph('X', height - 6, false, 0).bounds.width, 2 })
        , selection({ 0, height }) {

        box.setFillColor(sf::Color::Black);
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2);

        fancy_text.setOrigin(0, fancy_text.get_local_bounds().height / 2.f);
        fancy_text.setPosition(3, height / 2.f);

        cursor.setFillColor(sf::Color::White);
        cursor.setPosition(3, 17);


        selection.setFillColor(sf::Color::White);
        selection.setPosition(3, 0);
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
            //auto res = parse_command(content);
            auto pred = ""; //res.predictions.empty() ? "" : res.predictions.front();

            auto builder = sftk::TextBuilder{ font } << sftk::txt::size(box.getSize().y - 6);
            bool is_red{ false };
            bool is_selected{ false };

            float cursor_x{ 3 };
            float selection_lower_bound_x{ 3 };
            float selection_upper_bound_x{ 3 };
            std::cout << "INDEX: " << cursor_index << '\n';

            std::size_t const selection_lower_bound = std::max(0, static_cast<int>(cursor_index) + std::min(0, selection_offset));
            std::size_t const selection_upper_bound = cursor_index + std::max(0, selection_offset);

            for(std::size_t i{ 0 }; i < content.size(); ++i) {
                bool is_in_error = false;//res.index_of_error && i >= *res.index_of_error;
                bool is_in_selection = i >= selection_lower_bound && i < selection_upper_bound;

                if (is_in_error && !is_red) {
                    builder.set_fill_color(sf::Color::Red);
                    is_red = true;
                } 
                else if (!is_in_error && is_red) {
                    builder.set_fill_color(is_in_selection ? sf::Color::Black : sf::Color::White);
                    is_red = false;
                }
                
                if (is_in_selection && !is_selected) {
                    builder.set_fill_color(is_in_error ? sf::Color::Red : sf::Color::Black);
                    is_selected = true;
                } 
                else if (!is_in_selection && is_selected) {
                    builder.set_fill_color(is_in_error ? sf::Color::Red : sf::Color::White);
                    is_selected = false;
                }
                builder.append(content[i]);

                if(i + 1 == cursor_index) {
                    cursor_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
                }

                if(i + 1 == selection_lower_bound) {
                    selection_lower_bound_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
                }

                if(i + 1 == selection_upper_bound) {
                    selection_upper_bound_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
                }
            }

            if (cursor_index >= content.size()) {
                cursor_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
                cursor_index = content.size();
            } 

            if (selection_lower_bound >= content.size()) {
                selection_lower_bound_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
            } 

            if (selection_upper_bound >= content.size()) {
                selection_upper_bound_x = fancy_text.getTransform().transformPoint(builder.get_current_position()).x;
            } 

            fancy_text.set_text(std::move(builder)
                << sf::Color{ 100, 100, 100 }
                << pred);

            cursor.setPosition(cursor_x, 17);
            selection.setSize({ selection_upper_bound_x == selection_lower_bound_x ? 0 : selection_upper_bound_x - selection_lower_bound_x + 1, selection.getSize().y });
            selection.setPosition(selection_lower_bound_x - 1, 0);
        }
    }

    /* Drawable */
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(box, states);

        //target.draw(prediction, states);
        //target.draw(text, states);
        target.draw(selection, states);
        target.draw(fancy_text, states);
        target.draw(cursor, states);
    }


    sf::Font& font;
    std::string const default_message;
    std::string content;
    sftk::FancyText fancy_text;
    sf::RectangleShape box;
    sf::RectangleShape cursor;
    sf::RectangleShape selection;
    std::size_t cursor_index{ 0 };
    int selection_offset{ 0 };
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

    dbg_pckt::gui::CommandTextArea new_text_area(font, 780, 20, "Type a packet to send it");
    new_text_area.setPosition({ 8, 500 });

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
                        if (text_area.selection_offset != 0) {
                            std::size_t const selection_lower_bound = std::max(0, static_cast<int>(text_area.cursor_index) + std::min(0, text_area.selection_offset));
                            std::size_t const selection_upper_bound = text_area.cursor_index + std::max(0, text_area.selection_offset);
                            text_area.content.erase(
                                std::begin(text_area.content) + selection_lower_bound,
                                std::begin(text_area.content) + selection_upper_bound);

                            if (text_area.selection_offset < 0) {
                                text_area.cursor_index -= selection_upper_bound - selection_lower_bound;
                            }

                            text_area.selection_offset = 0;
                        }

                        else if (!text_area.content.empty() && text_area.cursor_index > 0) {
                            text_area.content.erase(--text_area.cursor_index, 1);
                        }

                        new_text_area.remove_previous_character();
                    } 
                    else if (event.text.unicode == 127) { // Supr
                        if (text_area.selection_offset != 0) {
                            std::size_t const selection_lower_bound = std::max(0, static_cast<int>(text_area.cursor_index) + std::min(0, text_area.selection_offset));
                            std::size_t const selection_upper_bound = text_area.cursor_index + std::max(0, text_area.selection_offset);
                            text_area.content.erase(
                                std::begin(text_area.content) + selection_lower_bound,
                                std::begin(text_area.content) + selection_upper_bound);

                            if (text_area.selection_offset < 0) {
                                text_area.cursor_index -= selection_upper_bound - selection_lower_bound;
                            }

                            text_area.selection_offset = 0;
                        }

                        else if (text_area.cursor_index < text_area.content.size()) {
                            text_area.content.erase(text_area.cursor_index, 1);
                        }

                        new_text_area.remove_next_character();
                    } 
                    else {
                        if (event.text.unicode < 32 || event.text.unicode > 127) {
                            break;
                        }
                        text_area.content.insert(text_area.cursor_index, 1, event.text.unicode);
                        ++text_area.cursor_index;

                        new_text_area.write(static_cast<char>(event.text.unicode));
                    }

                    text_area.update();

                    //parse_terms(text_area.text.getString().toAnsiString());

                    break;
    }

                case sf::Event::KeyPressed: {

                    if(event.key.control && event.key.code == sf::Keyboard::V) {
                        auto clipboard = sf::Clipboard::getString().toAnsiString();
                        clipboard.erase(std::remove_if(std::begin(clipboard), std::end(clipboard), [] (char c) { 
                            return c < 32 || c > 127; 
                        }), std::end(clipboard));
                        auto size = clipboard.size();

                        if (text_area.selection_offset != 0) {
                            std::size_t const selection_lower_bound = std::max(0, static_cast<int>(text_area.cursor_index) + std::min(0, text_area.selection_offset));
                            std::size_t const selection_upper_bound = text_area.cursor_index + std::max(0, text_area.selection_offset);
                            text_area.content.erase(
                                std::begin(text_area.content) + selection_lower_bound,
                                std::begin(text_area.content) + selection_upper_bound);

                            if (text_area.selection_offset < 0) {
                                text_area.cursor_index -= selection_upper_bound - selection_lower_bound;
                            }

                            text_area.selection_offset = 0;
                        }

                        text_area.content.insert(text_area.cursor_index, std::move(clipboard));
                        text_area.cursor_index += size;
                        text_area.update();

                        new_text_area.write(clipboard);
                    }

                    if(event.key.control && (event.key.code == sf::Keyboard::C || event.key.code == sf::Keyboard::X)) {
                        std::size_t const selection_lower_bound = std::max(0, static_cast<int>(text_area.cursor_index) + std::min(0, text_area.selection_offset));
                        std::size_t const selection_upper_bound = text_area.cursor_index + std::max(0, text_area.selection_offset);

                        std::string selected_string = text_area.content.substr(selection_lower_bound, selection_upper_bound - selection_lower_bound);
                        sf::Clipboard::setString(selected_string);

                        if (event.key.code == sf::Keyboard::X) {
                            text_area.content.erase(
                                std::begin(text_area.content) + selection_lower_bound,
                                std::begin(text_area.content) + selection_upper_bound);

                            if (text_area.selection_offset < 0) {
                                text_area.cursor_index -= selection_upper_bound - selection_lower_bound;
                            }

                            text_area.selection_offset = 0;
                            text_area.update();
                        }

                        sf::Clipboard::setString(new_text_area.copy_selection());
                        if (event.key.code == sf::Keyboard::X) {
                            new_text_area.remove_selection();
                        }
                    }

                    if (event.key.code == sf::Keyboard::Enter) {
                        /*auto res = parse_command(text_area.content);
                        std::cout << (res.command ? std::visit([] (auto const& v) { return packet_to_string(v).toAnsiString(); }, *res.command) : "Not found") << '\n';
                        for(auto const& p : res.predictions) {
                            std::cout << "> " << p << '\n';
                        }
                        if(res.index_of_error) {
                            std::cout << "# " << *res.index_of_error << '\n';
                        }*/

                        new_text_area.clear();
                    } 
                    else if(event.key.code == sf::Keyboard::Left) {
                        std::cout << "LEFT\n";
                        if (text_area.cursor_index > 0) {
                            if (event.key.shift) {
                                ++text_area.selection_offset;
                                --text_area.cursor_index;
                            } 
                            else if (text_area.selection_offset < 0) {
                                text_area.cursor_index += text_area.selection_offset;
                                text_area.selection_offset = 0;
                            }
                            else if (text_area.selection_offset > 0) {
                                text_area.selection_offset = 0;
                            }
                            else {
                                --text_area.cursor_index;
                            }

                            text_area.update();
                        } else if (!event.key.shift) {
                            text_area.selection_offset = 0;
                            text_area.update();
                        }
                        {
                            using namespace dbg_pckt::gui;
                            new_text_area.move_cursor(
                                CursorDirection::Left, 
                                event.key.shift ? Selection::Keep : Selection::Reset, 
                                Step::Character);
                        }
                    }

                    else if(event.key.code == sf::Keyboard::Right) {
                        std::cout << "RIGHT\n";
                        if (text_area.cursor_index < text_area.content.size()) {
                            if (event.key.shift) {
                                --text_area.selection_offset;
                                ++text_area.cursor_index;
                            } 
                            else if (text_area.selection_offset > 0) {
                                text_area.cursor_index += text_area.selection_offset;
                                text_area.selection_offset = 0;
                            }
                            else if (text_area.selection_offset < 0) {
                                text_area.selection_offset = 0;
                            }
                            else {
                                ++text_area.cursor_index;
                            }
                            text_area.update();
                        } else if (!event.key.shift) {
                            text_area.selection_offset = 0;
                            text_area.update();
                        }

                        {
                            using namespace dbg_pckt::gui;
                            new_text_area.move_cursor(
                                CursorDirection::Right, 
                                event.key.shift ? Selection::Keep : Selection::Reset, 
                                Step::Character);
                        }
                    }

                    else if (event.key.code == sf::Keyboard::Tab) {
                        new_text_area.write_prediction();
                        /*auto res = parse_command(text_area.content);
                        if (!res.predictions.empty()) {
                            text_area.content += res.predictions[0];
                            text_area.cursor_index = text_area.content.size();
                            text_area.selection_offset = 0;
                            text_area.update();
                        }*/
                    }

                    else if (event.key.code == sf::Keyboard::Up) {
                        new_text_area.cycle_prediction();
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        new_text_area.cycle_previous_prediction();
                    }

                    break;
                }

                default: { break; }
            }
        }


        window.clear(sf::Color::Black);

        window.draw(text_area);
        window.draw(new_text_area);

        window.display();
    }
}
