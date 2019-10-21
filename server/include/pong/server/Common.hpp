#pragma once

#include<numeric>

namespace pong::server {

using user_t = std::string;

template<typename...Ts>
sf::Packet to_packet(Ts&&... ts) {
    sf::Packet p;
    (p << ... << ts);
    return p;
}

template<typename T>
T from_packet(sf::Packet& p) {
    T t;
    p >> t;
    return t;
}





#define make_getter(C, F) getter<C, decltype(C::F), &C::F>

template<typename C, typename T, T (C::*field)>
T& getter(C& c) {
    return c.*field;
}




using user_id_t = std::uintptr_t;
constexpr user_id_t invalid_user_id = 0;
using user_handle_t = std::size_t;
constexpr user_handle_t invalid_user_handle = std::numeric_limits<std::size_t>::max();




struct User {
    std::unique_ptr<sf::TcpSocket> socket;
    std::vector<sf::Packet> packets {};
};





template<typename T>
struct UserData {
    User user;
    T data;
};

template<>
struct UserData<void> {
    User user;
};





struct Idle {};
using Leave = std::function<void()>;
struct Abord {}; 


using Action = std::variant<Idle, Leave, Abord>;





template<typename S>
using decltype_on_user_leave = decltype(&S::on_user_leave); 

template<typename S>
using decltype_on_user_enter = decltype(&S::on_user_enter); 





}