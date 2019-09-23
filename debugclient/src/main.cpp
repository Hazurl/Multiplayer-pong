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

int main(int argc, char** argv) {
    /* --------------------------
        * INITIALISATION
        * --------------------------
        */
    /* Connect to server */ 
    
    unsigned short const default_port = 48622;
    unsigned short const port = argc >= 2 ? std::stoi(argv[1]) : default_port;
    auto socket = std::make_unique<sf::TcpSocket>();
    if (socket->connect("127.0.0.1", port) != sf::Socket::Done) {
        std::cerr << "Couldn't connect to port " << port << '\n';
        return 1;
    }
    socket->setBlocking(false);
    
    /* Open windows */
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Multiplayer pong");
    /* Load assets */
    sf::Font font;
    if (!font.loadFromFile("../assets/neoletters.ttf")) {
        std::cout << "Couldn't load font '../assets/neoletters.ttf'\n";
        std::exit(1);
    }

    sf::Clock clock;
    std::deque<sf::Packet> packets_to_send;
    sf::Packet receiving_packet;

    dbg_pckt::gui::CommandTextArea text_area(font, 780, 20, "Type a packet to send it");
    text_area.setPosition({ 8, 568 });

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

                case sf::Event::TextEntered: {
                    if (event.text.unicode >= 128) { break; } // not ascii

                    if (event.text.unicode == 8) { // Delete
                        text_area.remove_previous_character();
                    } 
                    else if (event.text.unicode == 127) { // Supr
                        text_area.remove_next_character();
                    } 
                    else {
                        if (event.text.unicode < 32 || event.text.unicode > 127) {
                            break;
                        }
                        text_area.write(static_cast<char>(event.text.unicode));
                    }
                    break;
                }

                case sf::Event::KeyPressed: {

                    if(event.key.control && event.key.code == sf::Keyboard::V) {
                        auto clipboard = sf::Clipboard::getString().toAnsiString();
                        clipboard.erase(std::remove_if(std::begin(clipboard), std::end(clipboard), [] (char c) { 
                            return c < 32 || c > 127; 
                        }), std::end(clipboard));
                        auto size = clipboard.size();
                        text_area.write(clipboard);
                    }

                    if(event.key.control && (event.key.code == sf::Keyboard::C || event.key.code == sf::Keyboard::X)) {
                        sf::Clipboard::setString(text_area.copy_selection());
                        if (event.key.code == sf::Keyboard::X) {
                            text_area.remove_selection();
                        }
                    }

                    if (event.key.code == sf::Keyboard::Enter) {
                        auto content = text_area.get_content();
                        text_area.clear();

                        auto res = dbg_pckt::parser::parse_command(content);
                        if (res.is_success()) {
                            std::cout << wpr::describe(res.success()) << '\n';
                            sf::Packet packet;
                            std::visit([&packet] (auto&& value) { packet << value; }, res.success().as_variant());
                            packets_to_send.push_back(packet);
                        } else {
                            std::cout << "ERROR\n";
                        }
                    } 
                    else if(event.key.code == sf::Keyboard::Left) {
                        using namespace dbg_pckt::gui;
                        text_area.move_cursor(
                            CursorDirection::Left, 
                            event.key.shift ? Selection::Keep : Selection::Reset, 
                            Step::Character);
                    }

                    else if(event.key.code == sf::Keyboard::Right) {
                        using namespace dbg_pckt::gui;
                        text_area.move_cursor(
                            CursorDirection::Right, 
                            event.key.shift ? Selection::Keep : Selection::Reset, 
                            Step::Character);
                    }

                    else if (event.key.code == sf::Keyboard::Tab) {
                        text_area.write_prediction();
                    }

                    else if (event.key.code == sf::Keyboard::Up) {
                        text_area.cycle_prediction();
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        text_area.cycle_previous_prediction();
                    }

                    break;
                }

                default: { break; }
            }
        }

        if (!packets_to_send.empty()) {
            std::cout << packets_to_send.size() << " packets remaining to send\n";
            auto& p = packets_to_send.front();
            switch(socket->send(p)) {
                case sf::Socket::Done: {
                    packets_to_send.pop_front();
                    std::cout << "packet successfully sent\n";
                    break;
                }
                case sf::Socket::Disconnected: {
                    packets_to_send.clear();
                    break;
                }

                case sf::Socket::Error: {
                    window.close();
                    std::cerr << "Internal error on socket when sending packet...\n";
                    break;
                }

                default: {
                    break;
                }
            }
        }

        switch(socket->receive(receiving_packet)) {
            case sf::Socket::Done: {
                pong::packet::GamePacket game_packet;
                receiving_packet >> game_packet;
                std::cout << "Received: " << std::visit([] (auto&& value) { return dbg_pckt::packet_to_string(value); }, game_packet) << '\n';
                break;
            }

            case sf::Socket::Disconnected:
            case sf::Socket::Error: {
                window.close();
                std::cerr << "Internal error on socket when receiving packet...\n";
                break;
            }

            default: {
                break;
            }
        }


        window.clear(sf::Color::Black);

        window.draw(text_area);

        window.display();
    }
}
