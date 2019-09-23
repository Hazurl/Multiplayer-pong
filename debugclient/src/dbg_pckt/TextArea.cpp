#include <dbg_pckt/TextArea.hpp>

#include <iostream>
#include <cassert>

namespace dbg_pckt::gui {

TextArea::TextArea(sf::Font& _font, unsigned width, unsigned height) 
:   content{}
,   font{ _font }
,   box({ static_cast<float>(width), static_cast<float>(height) })
,   cursor({ font.getGlyph('X', height - 6, false, 0).bounds.width, 2 })
,   selection({ 0, static_cast<float>(height) })
,   fancy_text{ sf::VertexBuffer::Dynamic }
,   cursor_index{ 0 }
,   selection_offset{ 0 }
,   focus_enable{ true }
{
    box.setFillColor(sf::Color::Black);
    box.setOutlineColor(sf::Color::White);
    box.setOutlineThickness(2);

    force_generation();
    fancy_text.setOrigin(0, fancy_text.get_local_bounds().height / 2.f);
    fancy_text.setPosition(3, height / 2.f);

    cursor.setFillColor(sf::Color::White);
    cursor.setPosition(3, 17);

    selection.setFillColor(sf::Color::White);
    selection.setPosition(3, 0);
}

void TextArea::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(box, states);
    target.draw(selection, states);
    target.draw(fancy_text, states);
    target.draw(cursor, states);
}

void TextArea::write(char c) {
    remove_selection();
    content.insert(cursor_index, 1, c);
    ++cursor_index;
    force_generation();
}

void TextArea::write(std::string const& str) {
    remove_selection();
    content.insert(cursor_index, str);
    cursor_index += str.size();
    force_generation();
}

std::string TextArea::copy_selection() const {
    auto lower = selection_lower_bound();
    auto size = selection_size();
    return content.substr(lower, size);
}

void TextArea::remove_selection() {
    auto lower = selection_lower_bound();
    auto size = selection_size();
    
    content.erase(lower, size);
    cursor_index = lower;
    selection_offset = 0;
    force_generation();
}

void TextArea::remove_next_character() {
    if(selection_offset != 0) {
        remove_selection();
        return;
    }

    if(cursor_index < content.size()) {
        content.erase(cursor_index, 1);
        force_generation();
    }
}

void TextArea::remove_previous_character() {
    if(selection_offset != 0) {
        remove_selection();
        return;
    }

    if(cursor_index > 0 && !content.empty()) {
        content.erase(cursor_index - 1, 1);
        --cursor_index;
        force_generation();
    }
}

void TextArea::clear() {
    content.clear();
    cursor_index = selection_offset = 0;
    force_generation();
    //selection.setSize({ 0, selection.getSize().y });
}

void TextArea::move_cursor(CursorDirection dir, Selection sel, Step) {
    bool can_move = !(
        (cursor_index == 0 && dir == CursorDirection::Left) || 
        (cursor_index == content.size() && dir == CursorDirection::Right));

    if(can_move) {
        if (dir == CursorDirection::Left) {
            --cursor_index;
        } else {
            ++cursor_index;
        }
    }

    switch(sel) {
        case Selection::Keep:
            if (can_move) {
                if(selection_offset <= 0 && dir == CursorDirection::Left) {
                    ++selection_offset;
                }
                else if(selection_offset < 0 && dir == CursorDirection::Right) {
                    --selection_offset;
                }
                else if(selection_offset >= 0 && dir == CursorDirection::Right) {
                    --selection_offset;
                }
                else if(selection_offset > 0 && dir == CursorDirection::Left) {
                    ++selection_offset;
                }
            }
            break;

        case Selection::Reset:
            selection_offset = 0;
            break;
    }

    auto lower = selection_lower_bound();
    auto sel_size = selection_size();
    force_generation();
}

void TextArea::set_focus(bool enable) {
    focus_enable = enable;
}

void TextArea::force_generation() {
    auto res = generate_text();
    fancy_text.set_text(std::move(res.builder));

    auto transform = [&] (float x) {
        return fancy_text.getTransform().transformPoint(
            { x, 0 }
        ).x;
    };

    res.cursor_x = transform(res.cursor_x);
    res.selection_left = transform(res.selection_left);
    res.selection_right = transform(res.selection_right);

    cursor.setPosition(res.cursor_x, 17);
    selection.setSize({ 
        res.selection_right == res.selection_left ? 
            0 : 
            res.selection_right - res.selection_left + 2, 
        selection.getSize().y });
    selection.setPosition(res.selection_left - 1, 0);
}

TextArea::GeneratedText TextArea::generate_text() {
    TextArea::GeneratedText generated_text{
        sftk::TextBuilder{ get_font() } << sftk::txt::size(get_font_size()),
        0, 0, 0
    };

    assert(get_cursor_index() <= content.size());
    assert(get_left_selection_index() <= content.size());
    assert(get_right_selection_index() <= content.size());

    auto& builder = generated_text.builder;
    for(std::size_t i{ 0 }; i < content.size(); ++i) {
        builder.append(content[i]);
        if (get_cursor_index() == i + 1) {
            generated_text.cursor_x = builder.get_current_position().x;
        }

        if (get_left_selection_index() == i + 1) {
            generated_text.selection_left = builder.get_current_position().x;
        }

        if (get_right_selection_index() == i + 1) {
            generated_text.selection_right = builder.get_current_position().x;
        }
    }

    return generated_text;
}

sf::Font const& TextArea::get_font() const          { return font; }
float TextArea::get_font_size() const               { return box.getSize().y - 6.f; }
bool TextArea::is_focus() const                     { return focus_enable; }
std::string const& TextArea::get_content() const    { return content; }

bool TextArea::is_in_selection(std::size_t c) const { 
    auto lower = selection_lower_bound();
    return c >= lower && c < lower + selection_size();
}

std::size_t TextArea::get_cursor_index() const {
    return cursor_index;
}

std::size_t TextArea::get_left_selection_index() const {
    return selection_lower_bound();
}

std::size_t TextArea::get_right_selection_index() const {
    return selection_lower_bound() + selection_size();
}


std::size_t TextArea::selection_lower_bound() const {
    auto tmp =  selection_offset < 0 ? cursor_index + selection_offset : cursor_index;
    return tmp;
}

std::size_t TextArea::selection_size() const {
    return selection_offset < 0 ? -selection_offset : selection_offset;
}


}