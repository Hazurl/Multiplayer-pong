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
#include <tuple>
#include <deque>
#include <random>
#include <algorithm>

#include <pong/client/gui/constraint/Constraint.hpp>
#include <pong/client/gui/constraint/Interface.hpp>
#include <pong/client/gui/constraint/Set.hpp>

#include <sftk/print/Printer.hpp>

using sftk::operator<<;

float middle(std::vector<float> const& fs) {
    float parent_width = fs.at(0);
    float width = fs.at(1);
    float parent_left = fs.at(2);
    return parent_width / 2 - width / 2 + parent_left;
}

float one_tenth(std::vector<float> const& fs) {
    return fs.at(0) * 0.1f;
}

struct ConstrainedRectangle {
private:

    sf::RectangleShape transformable;
    pong::client::gui::property_id_t id;

    static constexpr std::size_t left_offset = 0;
    static constexpr std::size_t top_offset = 1;
    static constexpr std::size_t width_offset = 2;
    static constexpr std::size_t height_offset = 3;

public:


    ConstrainedRectangle(sf::RectangleShape _transformable, pong::client::gui::Gui<>& gui)
    :   transformable(_transformable)
    ,   id(gui.allocate_properties(4))
    {} 



    void update_to_Gui(pong::client::gui::Gui<>& gui) const {
        auto const position = transformable.getPosition();
        auto const size = transformable.getSize();
        gui.set_property(left(), position.x);
        gui.set_property(top(), position.y);
        gui.set_property(width(), size.x);
        gui.set_property(height(), size.y);
    }

    void update_from_Gui(pong::client::gui::Gui<> const& gui) {
        transformable.setPosition(
            gui.get_property(left()),
            gui.get_property(top())
        );
        transformable.setSize({
            gui.get_property(width()),
            gui.get_property(height())
        });
    }

    pong::client::gui::property_id_t left() const {
        return id + left_offset;
    }

    pong::client::gui::property_id_t top() const {
        return id + top_offset;
    }

    pong::client::gui::property_id_t width() const {
        return id + width_offset;
    }

    pong::client::gui::property_id_t height() const {
        return id + height_offset;
    }

    sf::RectangleShape& rect() {
        return transformable;
    }

    sf::RectangleShape const& rect() const {
        return transformable;
    }

};


template<std::size_t I, typename F>
void call_range(F f) {
    if constexpr (I > 0) {
        call_range<I - 1>(f);
        f(std::integral_constant<std::size_t, I - 1>{});
    }
}


int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Multiplayer pong");

    sf::Clock clock;

    using namespace pong::client::gui;

    Gui<> gui;

    std::vector<sf::Color> colors = {
        sf::Color{ 0x1a, 0xbc, 0x9c }, sf::Color{ 0x2e, 0xcc, 0x71 },
        sf::Color{ 0x34, 0x98, 0xdb }, sf::Color{ 0x9b, 0x59, 0xb6 },
        sf::Color{ 0x34, 0x49, 0x5e }, sf::Color{ 0xf1, 0xc4, 0x0f },
        sf::Color{ 0xe6, 0x7e, 0x22 }, sf::Color{ 0xe7, 0x4c, 0x3c },
        sf::Color{ 0xec, 0xf0, 0xf1 }, sf::Color{ 0x95, 0xa5, 0xa6 },
    };

    std::random_device rd;
    std::mt19937 rng(rd());

    std::shuffle(std::begin(colors), std::end(colors), rng);
    auto color_it = std::begin(colors);
    auto const colors_end = std::end(colors);

    std::vector<ConstrainedRectangle> constrained_rects;
    
    auto const make_rect = [&] (sf::Vector2f const& position = {10, 10}, sf::Vector2f const& size = {10, 10}) -> ConstrainedRectangle& {
        sf::RectangleShape rect(size);
        rect.setPosition(position);

        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineThickness(1);
        rect.setOutlineColor(*color_it);

        if (++color_it == colors_end) {
            color_it = std::begin(colors);
        }

        auto& constrained = constrained_rects.emplace_back(rect, gui);
        constrained.update_to_Gui(gui);
        return constrained;
    };

    auto const center_vertically_in = [&] (ConstrainedRectangle const& parent, ConstrainedRectangle const& that) {
        gui.set_constraint<
            [] (float parent_height, float parent_top, float height) {
                return parent_height / 2.f - height / 2.f + parent_top;
            }
        >(that.top(), { parent.height(), parent.top(), that.height() });
    };

    auto const center_horizontally_in = [&] (ConstrainedRectangle const& parent, ConstrainedRectangle const& that) {
        gui.set_constraint<
            [] (float parent_width, float parent_left, float width) {
                return parent_width / 2.f - width / 2.f + parent_left;
            }
        >(that.left(), { parent.width(), parent.left(), that.width() });
    };



    // windows
    auto window_constraint = make_rect({ 5, 5 }, { window.getSize().x - 10, window.getSize().y - 10 });


    // options
    {
        auto option_border = make_rect({0, 0}, {80, 20});

        gui.set_constraint<
            [] (float window_left, float window_width, float width) {
                return window_left + window_width - width;
            }
        >(option_border.left(), { window_constraint.left(), window_constraint.width(),  option_border.width() });

        gui.set_constraint<
            [] (float window_top) {
                return window_top;
            }
        >(option_border.top(), { window_constraint.top() });



        auto option_close = make_rect();

        center_vertically_in(option_border, option_close);
        gui.set_constraint<
            [] (float height) {
                return height - 10;
            }
        >(option_close.height(), { option_border.height() });
        gui.set_constraint<
            [] (float height) {
                return height - 10;
            }
        >(option_close.width(), { option_border.height() });
        gui.set_constraint<
            [] (float left, float width, float my_width) {
                return left + width - 5 - my_width;
            }
        >(option_close.left(), { option_border.left(), option_border.width(), option_close.width() });



        auto option_button = make_rect();

        center_vertically_in(option_border, option_button);
        gui.set_constraint<
            [] (float height) {
                return height;
            }
        >(option_button.height(), { option_close.height() });
        gui.set_constraint<
            [] (float left) {
                return left + 5;
            }
        >(option_button.left(), { option_border.left() });
        gui.set_constraint<
            [] (float my_left, float close_left) {
                return close_left - my_left - 5;
            }
        >(option_button.width(), { option_button.left(), option_close.left() });
    }

    // bar
    {
        static constexpr std::size_t count = 10;
        static constexpr float padding = 5;
        static constexpr float size = 60;

        auto bar = make_rect({}, { count * (size + padding) + padding , size + padding * 2 });

        center_horizontally_in(window_constraint, bar);
        gui.set_constraint<
            [] (float height, float parent_top, float parent_height) {
                return parent_top + parent_height - height - padding * 2;
            }
        >(bar.top(), { bar.height(), window_constraint.top(), window_constraint.height() });



        
        call_range<count>([&] (auto _i) {
            constexpr std::size_t i = _i;
            constexpr float left = padding + i * (padding + size);
            auto cell = make_rect({}, { size, size });

            center_vertically_in(bar, cell);
            gui.set_constraint<
                [left] (float parent_left) {
                    return parent_left + left;
                }
            >(cell.left(), { bar.left() });
        });
    }

    auto slider_bar = make_rect({}, { -1, 8 });
    auto slider_value_property = gui.allocate_property(0.5f);
    auto slider = make_rect({}, { 8, 24 });
    bool grab_slider = false;

    // form
    {

        static constexpr float padding = 5;

        auto form = make_rect({}, { 200, -1 });

        gui.set_constraint<
            [] (float left) {
                return left + padding * 2;
            }
        >(form.left(), { window_constraint.left() });

        center_vertically_in(window_constraint, form);




        auto title = make_rect({}, { 80, 24 });

        center_horizontally_in(form, title);
        gui.set_constraint<
            [] (float form_top) {
                return padding + form_top;
            }
        >(title.top(), { form.top() });



        center_horizontally_in(slider_bar, slider);
        gui.set_constraint<
            [] (float title_top, float title_height) {
                return title_top + title_height + padding;
            }
        >(slider.top(), { title.top(), title.height() });

        gui.set_constraint<
            [] (float slider_bar_left, float slider_bar_width, float width, float value) {
                return slider_bar_left + slider_bar_width * value - width / 2.f;
            }
        >(slider.left(), { slider_bar.left(), slider_bar.width(), slider.width(), slider_value_property });

        center_horizontally_in(form, slider_bar);
        center_vertically_in(slider, slider_bar);
        gui.set_constraint<
            [] (float form_width) {
                return form_width * 0.8f;
            }
        >(slider_bar.width(), { form.width() });



        auto centered_button = make_rect({}, { -1, 24 });
        auto left_button = make_rect({}, { -1, 24 });
        auto right_button = make_rect({}, { -1, 24 });

        center_horizontally_in(form, centered_button);
        gui.set_constraint<
            [] (float form_width) {
                return form_width * 0.8f;
            }
        >(centered_button.width(), { form.width() });
        gui.set_constraint<
            [] (float left_top, float right_top, float height) {
                return std::min(left_top, right_top) - padding - height;
            }
        >(centered_button.top(), { left_button.top(), right_button.top(), centered_button.height() });

        gui.set_constraint<
            [] (float form_width) {
                return form_width * 0.35f;
            }
        >(left_button.width(), { form.width() });

        gui.set_constraint<
            [] (float form_width) {
                return form_width * 0.35f;
            }
        >(right_button.width(), { form.width() });

        gui.set_constraint<
            [] (float form_left, float form_width) {
                return form_left + form_width * 0.1f;
            }
        >(left_button.left(), { form.left(), form.width() });

        gui.set_constraint<
            [] (float form_left, float form_width, float width) {
                return form_left + form_width * 0.9f - width;
            }
        >(right_button.left(), { form.left(), form.width(), right_button.width() });

        gui.set_constraint<
            [] (float form_top, float form_height, float height) {
                return form_top + form_height - padding - height;
            }
        >(left_button.top(), { form.top(), form.height(), left_button.height() });

        gui.set_constraint<
            [] (float form_top, float form_height, float height) {
                return form_top + form_height - padding - height;
            }
        >(right_button.top(), { form.top(), form.height(), right_button.height() });




        gui.set_constraint<
            [] (float title_height, float slider_bar_height, float centered_button_height, float left_button_height, float right_button_height) {
                return padding + title_height + padding + slider_bar_height + padding + centered_button_height + padding + std::max(left_button_height, right_button_height) + padding;
            }
        >(form.height(), { title.height(), slider.height(), centered_button.height(), left_button.height(), right_button.height() });
    }

    if(!gui.compute_order()) {
        std::cerr << "ERROR COMPUTING ORDER!\n";
    }

    gui.update_properties();
    for(auto& constrained : constrained_rects) {
        constrained.update_from_Gui(gui);
    }

    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            switch(event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }
                case sf::Event::Resized: {
                    sf::View view({0, 0, event.size.width, event.size.height});
                    window.setView(view);

                    window_constraint.rect().setSize({event.size.width - 10, event.size.height - 10});
                    window_constraint.update_to_Gui(gui);

                    gui.update_properties();
                    for(auto& constrained : constrained_rects) {
                        constrained.update_from_Gui(gui);
                    }
                    break;
                }
                case sf::Event::KeyPressed: {
                    if (event.key.code == sf::Keyboard::Up) {
                        std::cout << "UP!\n";
                        gui.set_property(slider_value_property, gui.get_property(slider_value_property) + 0.1f);
                        gui.update_properties();
                        for(auto& constrained : constrained_rects) {
                            constrained.update_from_Gui(gui);
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        std::cout << "DOWN!\n";
                        gui.set_property(slider_value_property, gui.get_property(slider_value_property) - 0.1f);
                        gui.update_properties();
                        for(auto& constrained : constrained_rects) {
                            constrained.update_from_Gui(gui);
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Space) {
                        std::cout << "RESET!\n";
                        gui.set_property(slider_value_property, 0.5f);
                        gui.update_properties();
                        for(auto& constrained : constrained_rects) {
                            constrained.update_from_Gui(gui);
                        }
                    }
                    break;
                }

                case sf::Event::MouseButtonPressed: {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2f mouse{ event.mouseButton.x, event.mouseButton.y };
                        slider.update_from_Gui(gui);
                        if (slider.rect().getGlobalBounds().contains(mouse)) {
                            grab_slider = true;
                        }
                    }
                    break;
                }

                case sf::Event::MouseButtonReleased: {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        grab_slider = false;
                    }
                    break;
                }

                case sf::Event::MouseMoved: {
                    if (grab_slider) {
                        auto min = gui.get_property(slider_bar.left()); 
                        auto max = min + gui.get_property(slider_bar.width()); 
                        float x = std::max(min, std::min(static_cast<float>(event.mouseMove.x), max));
                        float value_normalized = (x - min) / (max - min);

                        gui.set_property(slider_value_property, value_normalized);
                        gui.update_properties();
                        for(auto& constrained : constrained_rects) {
                            constrained.update_from_Gui(gui);
                        }
                    }
                    break;
                }
                default: { break; }
            }
        }

        float dt = clock.restart().asSeconds();
        window.clear(sf::Color::Black);

        for(auto& constrained : constrained_rects) {
            window.draw(constrained.rect());
        }

        window.display();
    }

    return 0;
}