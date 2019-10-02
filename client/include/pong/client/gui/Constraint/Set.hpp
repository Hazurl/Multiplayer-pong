#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <stdexcept>

namespace pong::client::gui {

template<typename I = std::uint64_t>
struct BitSet {
public:

    static constexpr std::size_t bits_count = sizeof(I) * 8;
    static constexpr I filled_value = 0;
    static constexpr I empty_value = ~filled_value;


    static_assert([]{
        for(std::size_t i{ 0 }; i < bits_count; ++i) {
            if (!(empty_value & (I{ 1 } << i))) return false;
        }
        return true;
    }(), "The binary notation of `~I{ 0 }` must not contain 0s");

    static_assert([]{
        for(std::size_t i{ 0 }; i < bits_count; ++i) {
            if (filled_value & (I{ 1 } << i)) return false;
        }
        return true;
    }(), "The binary notation of `I{ 0 }` must not contain 1s");


    /*
        If the bit `n` is 1, the position `n` is empty
    */
    std::vector<I> bits{ empty_value };
    std::size_t first_with_hole{ 0 };


    bool is_filled(std::size_t handler) const {
        assert(handler < bits.size());
        return bits[handler] == filled_value;
    }

    bool has_a_hole(std::size_t handler) const {
        return !is_filled(handler);
    }

    std::size_t find_with_hole(std::size_t first_handler) {
        assert(first_handler <= bits.size());
        for(
            auto const size = bits.size()
        // break the loop if `first_handler` exceed the size or if it contains a hole
        // additionnally push an empty value so first_handler is always in bound (assuming `first_handler <= size` initially)
        ;   (first_handler < size || (bits.push_back(empty_value), false)) && is_filled(first_handler)
        ;   ++first_handler);

        assert(first_handler < bits.size());
        assert(has_a_hole(first_handler));

        return first_handler;
    }

    std::size_t update_first_with_hole() {
        return first_with_hole = find_with_hole(first_with_hole);
    }

    static constexpr I mask_n_bits(std::size_t const count)  {
        assert(count <= bits_count);
        return count == bits_count ? ~I{ 0 } : ((I{ 1 } << count) - 1);
    }

    std::pair<std::size_t, std::size_t> index_of_multiple_hole(std::size_t const count) {
        for(auto handler_with_hole = first_with_hole; true; handler_with_hole = find_with_hole(handler_with_hole + 1)) {
            assert(handler_with_hole < bits.size());
            // Case `count` fits in `I`
            I const value = bits[handler_with_hole];

            if (count <= bits_count) {
                // Sub-case search if count bits empty in the current handler
                I const mask_count_bits = mask_n_bits(count);
                for(std::size_t i{ 0 }; i <= bits_count - count; ++i) {
                    // `(value >> i) & mask_count_bits` is filled with 1 if all the values are empty
                    if (((value >> i) & mask_count_bits) == mask_count_bits) {
                        return std::make_pair(handler_with_hole, i);
                    }
                }
            }

            // Sub-case the holes overlaps multitple values

            // Find maximum numbers of holes at the end of the current handler
            std::size_t number_of_hole_at_end{ std::min(count - 1, bits_count) };
            for(; number_of_hole_at_end > 0 ; --number_of_hole_at_end) {
                I const mask = mask_n_bits(number_of_hole_at_end);
                if (((value >> (bits_count - number_of_hole_at_end)) & mask) == mask) {
                    break;
                }
            }

            if (number_of_hole_at_end > 0) {
                // Check the next handler
                std::size_t remaining = count - number_of_hole_at_end;
                std::size_t current_handler = handler_with_hole + 1;
                if (current_handler >= bits.size()) {
                    bits.push_back(empty_value);
                    assert(current_handler <= bits.size());
                }

                bool error{ false };

                for(; remaining >= bits_count; remaining -= bits_count) {

                    if (bits[current_handler] != empty_value) {
                        error = true;
                        break;
                    }

                    if (++current_handler >= bits.size()) {
                        bits.push_back(empty_value);
                        assert(current_handler <= bits.size());
                    }
                }

                if (!error && (bits[current_handler] & mask_n_bits(remaining)) == mask_n_bits(remaining)) {
                    return std::make_pair(handler_with_hole, bits_count - number_of_hole_at_end);
                }
            }
        }
    }

    std::size_t combine(std::size_t const handler, std::size_t sub_index) const {
        return (handler * bits_count) + sub_index;
    }

    // (handler, sub_index) 
    std::pair<std::size_t, std::size_t> decompose(std::size_t const index) const {
        return std::make_pair(index / bits_count, index % bits_count);
    }

    void set(std::size_t const handler, std::size_t const sub_index) {
        assert(handler < bits.size() && sub_index < bits_count);

        bits[handler] &= ~(I{ 1 } << sub_index);
    }

    void unset(std::size_t const handler, std::size_t const sub_index) {
        assert(handler < bits.size() && sub_index < bits_count);

        bits[handler] |= I{ 1 } << sub_index;
    }

    static std::size_t right_most_set_bit(I value) {
        using int_t = int; // parameter of __builtin_ffs
        constexpr std::size_t size_int_t = sizeof(int_t) * 8;
        static_assert(bits_count <= size_int_t * 4, "type of size > sizeof(int) * 4 are not supported");

        int_t rhs = static_cast<int_t>(value);

        auto index = __builtin_ffs(rhs);
        if (index > 0) {
            return index;
        }
        
        if constexpr (bits_count > size_int_t) {
            int_t lhs = static_cast<int_t>(value >> (size_int_t));
            index = __builtin_ffs(lhs) + size_int_t;
            if (index > size_int_t) {
                return index;
            }
        }

        if constexpr (bits_count > size_int_t * 2) {
            int_t llhs = static_cast<int_t>(value >> (size_int_t * 2));
            index = __builtin_ffs(llhs) + size_int_t * 2;
            if (index > size_int_t * 2) {
                return index;
            }
        }

        if constexpr (bits_count > size_int_t * 3) {
            int_t lllhs = static_cast<int_t>(value >> (size_int_t * 3));
            index = __builtin_ffs(lllhs) + size_int_t * 3;
            if (index > size_int_t * 3) {
                return index;
            }
        }

        return 0;
    }

    std::size_t index_of_hole(std::size_t const handler) const {
        assert(has_a_hole(handler));
        return right_most_set_bit(bits[handler]) - 1;
    } 

    std::pair<std::size_t, std::size_t> next_hole() {
        auto const with_hole = update_first_with_hole();
        auto const hole_sub_index = index_of_hole(with_hole);
        assert(with_hole < bits.size());
        assert(hole_sub_index < bits_count);
        return std::make_pair(with_hole, hole_sub_index);
    }

public:

    std::size_t set_next() {
        auto [handler, sub_index] = next_hole();

        assert(sub_index < bits_count);
        assert(!contains(combine(handler, sub_index)));

        set(handler, sub_index);

        return combine(handler, sub_index);
    }

    std::size_t set_next(std::size_t count) {
        auto [handler, sub_index] = index_of_multiple_hole(count);
        auto const index = combine(handler, sub_index);

        while(count-- > 0) {
            assert(handler < bits.size());
            assert(sub_index < bits_count);

            set(handler, sub_index);
            
            if (++sub_index >= bits_count) {
                ++handler;
                sub_index = 0;
            }
        }

        return index;
    }



    void set(std::size_t const index) {
        auto const[handler, sub_index] = decompose(index);

        if (handler >= bits.size()) {
            bits.resize(handler + 1, ~I{ 0 });
        }

        set(handler, sub_index);
    }

    void set_multiple(std::size_t const index, std::size_t count) {
        auto [handler, sub_index] = decompose(index);

        if (handler >= bits.size()) {
            bits.resize(handler + 1 + (sub_index + count) / bits_count, ~I{ 0 });
        }

        while(count-- > 0) {
            set(handler, sub_index);
            
            if (++sub_index >= bits_count) {
                ++handler;
                sub_index = 0;

                if (handler >= bits.size()) {
                    bits.push_back(empty_value);
                    assert(handler < bits.size());
                }
            }
        }
    }



    void unset(std::size_t const index) {
        auto const[handler, sub_index] = decompose(index);

        if (handler >= bits.size()) {
            return;
        }

        first_with_hole = std::min(handler, first_with_hole);

        unset(handler, sub_index);
    } 

    void unset_multiple(std::size_t const index, std::size_t count) {
        auto [handler, sub_index] = decompose(index);

        while(count-- > 0) {
            unset(handler, sub_index);
            
            if (++sub_index >= bits_count) {
                ++handler;
                sub_index = 0;

                if (handler >= bits.size()) {
                    bits.push_back(empty_value);
                    assert(handler < bits.size());
                }
            }
        }
    }



    bool contains(std::size_t const index) const {
        auto const[handler, sub_index] = decompose(index);

        return handler < bits.size() && !((bits[handler] >> sub_index) & I{ 1 });
    }

    std::size_t capacity() const {
        return bits.size() * bits_count;
    }

    void clear() {
        bits = { empty_value };
        first_with_hole = 0;
    }

};

template<typename T>
struct Set {
private:

    BitSet<> bitset;
    std::vector<T> values;

public:

    T& operator[](std::size_t const index) {
        assert(bitset.contains(index));
        return values[index];
    }

    T const& operator[](std::size_t const index) const {
        assert(bitset.contains(index));
        return values[index];
    }

    T& at(std::size_t const index) {
        if (!bitset.contains(index)) {
            throw std::out_of_range("Set::at(__index): out of range; with __index = " + std::to_string(index));
        }
        return values[index];
    }

    T const& at(std::size_t const index) const {
        if (!bitset.contains(index)) {
            throw std::out_of_range("Set::at(__index): out of range; with __index = " + std::to_string(index));
        }
        return values[index];
    }

    std::size_t capacity() const {
        return values.size();
    }

    bool contains(std::size_t const index) const {
        return bitset.contains(index);
    }

    std::size_t push(T&& value) {
        auto index = bitset.set_next();

        if (index >= values.size()) {
            assert(index == values.size());
            values.push_back(std::move(value));
        } else {
            values[index] = std::move(value);
        }

        return index;
    } 

    std::size_t push(T const& value) {
        auto index = bitset.set_next();

        if (index >= values.size()) {
            assert(index == values.size());
            values.push_back(value);
        } else {
            values[index] = value;
        }

        return index;
    } 

    template<typename...Args>
    std::size_t emplace(Args&&...args) {
        auto index = bitset.set_next();

        if (index >= values.size()) {
            assert(index == values.size());
            values.push_back(std::forward<Args>(args)...);
        } else {
            values[index] = T(std::forward<Args>(args)...);
        }

        return index;
    } 

    template<typename...Args>
    std::size_t push_multiple(std::size_t const count, T const& value = T()) {
        assert(count > 0);
        auto const index = bitset.set_next(count);

        if (index + count > values.size()) {
            for(std::size_t i{ index }; i < values.size(); ++i) {
                values[i] = value;
            }
            values.resize(index + count, value);
        } else {
            for(std::size_t i{ index }; i < index + count; ++i) {
                values[i] = value;
            }
        }

        return index;
    }

    void erase(std::size_t const index) {
        bitset.unset(index);
    }

    template<typename...Args>
    void erase_multiple(std::size_t const index, std::size_t const count) {
        bitset.unset_multiple(index, count);
    }

};

}