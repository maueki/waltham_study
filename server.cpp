
#include "common.h"

#include <waltham-server.h>

#include <cstdint>
#include <cstdio>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <string>
#include <memory>

using boost::asio::ip::tcp;

enum { max_length = 1024 };

class session {
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service) {}

    tcp::socket& socket() { return socket_; }

    void handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred) {
        boost::asio::mutable_buffer b =
            boost::asio::buffer(data_, bytes_transferred);
        std::size_t s = boost::asio::buffer_size(b);
        std::string str(data_, s);
        printf("receive data: %s\n", str.c_str());

        start();
    }

    void start() {
        socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server {
public:
    server(boost::asio::io_service& io_service)
        : io_service_(io_service)
        , acceptor_(io_service, tcp::endpoint(tcp::v4(), server_port)) {
        start_accept();
    }

    void start_accept() {
        session* sess = new session(io_service_);
        acceptor_.async_accept(sess->socket(),
                              boost::bind(&server::handle_accept, this, sess,
                                          boost::asio::placeholders::error));
    }

private:
    void handle_accept(session* sess, const boost::system::error_code& error) {
        if (!error) {
            sess->start();
        } else {
            delete sess;
        }

        start_accept();
    }

    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
    boost::asio::io_service io_service;

    server srv(io_service);

    io_service.run();
    return 0;
}
