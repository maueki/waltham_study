
#include <waltham-server.h>

#include <cstdint>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

int main(int argc, char *argv[])
{
    boost::asio::io_service io_service;

    io_service.run();
    return 0;
}
