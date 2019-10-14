#include <pong/client/net/Connection.hpp>

#include <cassert>

#include <pong/client/Logger.hpp>

#include <sftk/print/Printer.hpp>

namespace pong::client::net {

template<typename T>
bool is_future_ready(std::future<T> const& f) { 
    return f.valid() && f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

bool Connection::is_connected() const {
    return socket != nullptr;
}

bool Connection::is_connecting() const {
    return future_socket.valid();
}

void Connection::stop_connection() {
    NOTICE("Stop connection");
    socket = nullptr;
    future_socket = {};
    assert(!is_connecting());
}

void Connection::attempt_to_connect(sf::IpAddress const& _addr, unsigned short _port, sf::Time _timeout) {
    assert(!is_connecting() && !is_connected());
    NOTICE("Attempt to connect to ", _addr, ":" , _port, " under ", _timeout);
    future_socket = std::async(std::launch::async, [] (sf::IpAddress const& addr, unsigned short port, sf::Time timeout) -> std::unique_ptr<sf::TcpSocket> {
        auto socket_ptr = std::make_unique<sf::TcpSocket>();
        if (socket_ptr->connect(addr, port, timeout) != sf::Socket::Done) {
            return nullptr;
        }
        socket_ptr->setBlocking(false);
        return socket_ptr;
    }, _addr, _port, _timeout);
}

bool Connection::check_connection() {
    if (is_future_ready(future_socket)) {
        std::unique_ptr<sf::TcpSocket> res = future_socket.get();
        if (!res) {
            ERROR("Couldn't connect to server... Abording...");
            stop_connection();
            return false;
        }

        socket = std::move(res);
        SUCCESS("Connected");

        return true;
    }
    return false;
}

sf::TcpSocket* Connection::get_socket() {
    return socket.get();
}

}