#pragma once

#include <SFML/Network.hpp>

#include <memory>
#include <future>

namespace pong::client::net {

class Connection {
private:

    std::unique_ptr<sf::TcpSocket> socket;
    std::future<std::unique_ptr<sf::TcpSocket>> future_socket;

public:
    
    bool is_connected() const;
    bool is_connecting() const;
    
    void stop_connection();
    void attempt_to_connect(sf::IpAddress const& _addr, unsigned short _port, sf::Time _timeout = sf::Time::Zero);
    
    bool check_connection();

    sf::TcpSocket* get_socket();

};

}