
#include "common.h"

#include <waltham-server.h>
#include <waltham-connection.h>

#include <cstdint>
#include <cstdio>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <string>
#include <memory>

using boost::asio::ip::tcp;

enum { max_length = 1024 };

static void display_handle_client_version(struct wth_display* wth_display,
                                          uint32_t client_version) {
    wth_object_post_error((struct wth_object*)wth_display, 0,
                          "unimplemented: %s", __func__);
}

static void display_handle_sync(struct wth_display* wth_display,
                                struct wthp_callback* callback) {
    auto* c =
        static_cast<class client*>(wth_object_get_user_data((struct wth_object*)wth_display));

    fprintf(stderr, "Client %p requested wth_display.sync\n", c);
    wthp_callback_send_done(callback, 0);
    wthp_callback_free(callback);
}

static void display_handle_get_registry(struct wth_display* wth_display,
                                        struct wthp_registry* registry) {
    auto* c = static_cast<class client*>(wth_object_get_user_data((struct wth_object*)wth_display));

    fprintf(stderr, "display_handle_get_registry\n");

#if 0  // TODO: 
    struct registry* reg;

    reg = zalloc(sizeof *reg);
    if (!reg) {
        client_post_out_of_memory(c);
        return;
    }

    reg->obj = registry;
    reg->client = c;
    wl_list_insert(&c->registry_list, &reg->link);
    wthp_registry_set_interface(registry, &registry_implementation, reg);

    /* XXX: advertise our globals */
    wthp_registry_send_global(registry, 1, "wthp_compositor", 4);
#endif
}

static const struct wth_display_interface display_implementation = {
    display_handle_client_version, display_handle_sync,
    display_handle_get_registry};

class client {
public:
    client(boost::asio::io_service& io_service) : socket_(io_service) {}

    tcp::socket& socket() { return socket_; }

    void handle_read(const boost::system::error_code& error) {
        if (!error) {
            wth_connection_read(conn_);
            // TODO: error
            wth_connection_dispatch(conn_);
            // TODO: error

            prepare_read();
        } else {
            printf("delete this\n");
            delete this;
        }
    }

    void start() {
        int32_t fd = socket_.native_handle();

        // TODO: assert(fd);

        conn_ = wth_connection_from_fd(fd, WTH_CONNECTION_SIDE_SERVER);

        prepare_read();

        wth_display* disp = wth_connection_get_display(conn_);
        wth_display_set_interface(disp, &display_implementation, this);
    }

    void prepare_read() {
        socket_.async_read_some(
            boost::asio::null_buffers(),
            boost::bind(&client::handle_read, this,
                        boost::asio::placeholders::error));
    }

    tcp::socket socket_;
    wth_connection* conn_;
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
        client* sess = new client(io_service_);
        acceptor_.async_accept(sess->socket(),
                               boost::bind(&server::handle_accept, this, sess,
                                           boost::asio::placeholders::error));
    }

private:
    void handle_accept(client* cli, const boost::system::error_code& error) {
        if (!error) {
            cli->start();
        } else {
            delete cli;
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
