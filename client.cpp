#include <waltham-client.h>

#include <boost/asio.hpp>

#include <iostream>

using boost::asio::ip::tcp;

int main(int argc, char *argv[])
{
    try {
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "localhost", "61000");
        tcp::resolver::iterator iter = resolver.resolve(query);

        tcp::socket s(io_service);
        boost::asio::connect(s, iter);

        char request[] = "hoge";
        std::size_t len = std::strlen(request);
        boost::asio::write(s, boost::asio::buffer(request, len));

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
