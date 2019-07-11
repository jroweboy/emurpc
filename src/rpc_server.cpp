

#include <unordered_map>
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

class Session : public std::enable_shared_from_this<Session> {

public:
    // Take ownership of the socket
    explicit Session(u32 id, ClientEmuQueue& client_emu, tcp::socket&& socket)
        : id(id), client_emu(client_emu), ws(std::move(socket)), strand(ws.get_executor()),
          ser(std::make_unique<JSONSerializer>()) {}

    // Start the asynchronous operation
    void Run() {
        // Accept the websocket handshake
        ws.async_accept(boost::asio::bind_executor(
            strand, std::bind(&Session::OnAccept, shared_from_this(), std::placeholders::_1)));
    }

    void Send(const Response::AnyPacket& packet) {
        send_data = ser->SerializeResponse(packet);
        size_t n = boost::asio::buffer_copy(write_buffer.prepare(send_data.size()),
                                            boost::asio::buffer(send_data));
        write_buffer.commit(n);
        ws.async_write(write_buffer.data(),
                       boost::asio::bind_executor(
                           strand, std::bind(&Session::OnWrite, shared_from_this(),
                                             std::placeholders::_1, std::placeholders::_2)));
    }

private:
    void OnAccept(beast::error_code ec) {
        // Read a message
        DoRead();
    }

    void OnWrite(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // Clear the buffer
        write_buffer.consume(write_buffer.size());
    }

    void DoRead() {
        // Read a message into our buffer
        ws.async_read(read_buffer,
                      boost::asio::bind_executor(
                          strand, std::bind(&Session::OnRead, shared_from_this(),
                                            std::placeholders::_1, std::placeholders::_2)));
    }

    void OnRead(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (bytes_transferred == 0) {
            DoRead();
            return;
        }

        std::vector<u8> bytes(read_buffer.size());
        boost::asio::buffer_copy(boost::asio::buffer(bytes), read_buffer.data());
        read_buffer.consume(read_buffer.size());

        // Echo the message
        if (ws.got_text()) {
            // TODO: choose serializer per message?
        }
        client_emu.push({id, ser->DeserializeRequest(bytes)});
        DoRead();
    }

    websocket::stream<tcp::socket> ws;
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
    boost::beast::multi_buffer read_buffer;
    boost::beast::multi_buffer write_buffer;
    std::vector<u8> send_data;
    ClientEmuQueue& client_emu;
    std::unique_ptr<ProtocolSerializer> ser;
    u32 id;
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(ClientEmuQueue& client_emu, boost::asio::io_context& ioc,
             const tcp::endpoint& endpoint)
        : client_emu(client_emu), acceptor(ioc), socket(ioc) {
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

    void Send(u32 client_id, const Response::AnyPacket& packet) {
        if (auto sess = sessions[client_id].lock()) {
            sess->Send(packet);
        }
    }

private:
    void DoAccept() {
        acceptor.async_accept(
            socket, std::bind(&Listener::OnAccept, shared_from_this(), std::placeholders::_1));
    }

    void OnAccept(boost::system::error_code ec) {
        if (!ec) {
            // Create the session and run it
            auto sess = std::make_shared<Session>(next_client_id, client_emu, std::move(socket));
            sessions[next_client_id++] = sess;
            sess->Run();
        }

        // Accept another connection
        DoAccept();
    }

    tcp::acceptor acceptor;
    tcp::socket socket;
    u32 next_client_id = 0;
    std::unordered_map<u32, std::weak_ptr<Session>> sessions;
    ClientEmuQueue& client_emu;
};

RPCServer::RPCServer(ClientEmuQueue& client_emu, EmuClientQueue& emu_client,
                     const std::string& hostname, u16 port)
    : emu_client(emu_client) {
    const auto address = boost::asio::ip::make_address(hostname);
    auto io = new net::io_context;
    ioc = std::unique_ptr<net::io_context>(io);
    listener = std::make_shared<Listener>(client_emu, *ioc, tcp::endpoint{address, port});

    // Create and launch a listening port
    io_thread = std::thread{[&] {
        listener->Run();
        ioc->run();
    }};
}

RPCServer::~RPCServer() {
    // stopped = true;
    ioc->stop();
    io_thread.join();
}

void RPCServer::SendResponse(u32 client_id, const Response::AnyPacket& p) {
    listener->Send(client_id, p);
}
