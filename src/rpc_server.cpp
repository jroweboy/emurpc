

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include "json_protocol.h"
#include "rpc_server.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Echoes back all received WebSocket messages
class Session : public std::enable_shared_from_this<Session> {

public:
    // Take ownership of the socket
    explicit Session(tcp::socket&& socket) : ws(std::move(socket)), strand(ws.get_executor()) {}

    // Start the asynchronous operation
    void Run() {

        // Accept the websocket handshake
        ws.async_accept(boost::asio::bind_executor(
            strand, std::bind(&Session::OnAccept, shared_from_this(), std::placeholders::_1)));
    }

    void OnAccept(beast::error_code ec) {
        // Read a message
        DoRead();
    }

    void DoRead() {
        // Read a message into our buffer
        ws.async_read(buffer, boost::asio::bind_executor(
                                  strand, std::bind(&Session::OnRead, shared_from_this(),
                                                    std::placeholders::_1, std::placeholders::_2)));
    }

    void OnRead(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // Echo the message
        ws.text(ws.got_text());
        ws.async_write(buffer.data(),
                       boost::asio::bind_executor(
                           strand, std::bind(&Session::OnWrite, shared_from_this(),
                                             std::placeholders::_1, std::placeholders::_2)));
    }

    void OnWrite(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // Clear the buffer
        buffer.consume(buffer.size());

        // Do another read
        DoRead();
    }

private:
    websocket::stream<tcp::socket> ws;
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
    boost::beast::multi_buffer buffer;
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint)
        : acceptor(ioc), socket(ioc) {
        boost::system::error_code ec;

        // Open the acceptor
        acceptor.open(endpoint.protocol(), ec);

        // Allow address reuse
        acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);

        // Bind to the server address
        acceptor.bind(endpoint, ec);

        // Start listening for connections
        acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    }

    // Start accepting incoming connections
    void Run() {
        if (!acceptor.is_open())
            return;
        DoAccept();
    }

    void DoAccept() {
        acceptor.async_accept(
            socket, std::bind(&Listener::OnAccept, shared_from_this(), std::placeholders::_1));
    }

    void OnAccept(boost::system::error_code ec) {
        if (!ec) {
            // Create the session and run it
            std::make_shared<Session>(std::move(socket))->Run();
        }

        // Accept another connection
        DoAccept();
    }

private:
    tcp::acceptor acceptor;
    tcp::socket socket;
};

RPCServer::RPCServer(const std::string& hostname, u16 port) {
    const auto address = boost::asio::ip::make_address(hostname);
    auto io = new net::io_context;
    ioc = std::unique_ptr<net::io_context>(io);
    listener = std::make_shared<Listener>(*ioc, tcp::endpoint{address, port});

    // Create and launch a listening port
    io_thread = std::thread{[&] {
        listener->Run();
    }};
}

RPCServer::~RPCServer() {
    ioc->stop();
    io_thread.join();
}

void RPCServer::ProcessClientEvents() {}

void RPCServer::HandleEmuCallbacks(const Response::Packet* p) {}
