#include <iostream>
#include <iomanip>
#include <string_view>

#include <boost/system/error_code.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;


constexpr auto kHost{"echo.websocket.org"};
constexpr auto kPort{"80"};


void LogError(const std::string_view where, const beast::error_code& ec)
{
    std::cerr << '[' << std::setw(20) << where << "]Error: " << ec.message() << std::endl;
}


void OnReceive(
    // lambda
    beast::flat_buffer &receive_buffer,
    // args
    const beast::error_code &ec
)
{
    if (ec) {
        LogError("OnReceive", ec);
        return;
    }
    std::cout << "ECHO: " << beast::make_printable(receive_buffer.data()) << std::endl;
}

void OnSend(
    // lambda
    beast::websocket::stream<beast::tcp_stream> &tcp_ws,
    beast::flat_buffer &receive_buffer,
    // args
    const beast::error_code &ec
)
{
    if (ec) {
        LogError("OnSend", ec);
        return;
    }
    tcp_ws.async_read(receive_buffer,
        [&receive_buffer](auto ec, auto bytes_read){
            OnReceive(receive_buffer, ec);
        }
    );
}

void OnHandshake(
    // lambda
    beast::websocket::stream<beast::tcp_stream> &tcp_ws,
    const asio::const_buffer &send_buffer,
    beast::flat_buffer &receive_buffer,
    // args
    const beast::error_code &ec
)
{
    if (ec) {
        LogError("OnHandshake", ec);
        return;
    }
    tcp_ws.async_write(send_buffer,
        [&tcp_ws, &receive_buffer](auto ec, auto bytes_written){
            OnSend(tcp_ws, receive_buffer, ec);
        }
    );
}

void OnConnect(
    // lambda
    beast::websocket::stream<beast::tcp_stream> &tcp_ws,
    const std::string_view url,
    const asio::const_buffer &send_buffer,
    beast::flat_buffer &receive_buffer,
    // args
    const beast::error_code &ec
)
{
    if (ec) {
        LogError("OnConnect", ec);
        return;
    }
    tcp_ws.async_handshake(url, "/",
        [&tcp_ws, &send_buffer, &receive_buffer](auto ec){
            OnHandshake(tcp_ws, send_buffer, receive_buffer, ec);
        }
    );
}

void OnResolve(
    // lambda
    beast::websocket::stream<beast::tcp_stream> &tcp_ws,
    const std::string_view url,
    const asio::const_buffer &send_buffer,
    beast::flat_buffer &receive_buffer,
    // args
    const beast::error_code& ec,
    asio::ip::tcp::resolver::results_type endpoints
)
{
    if (ec) {
        LogError("OnResolve", ec);
        return;
    }
    tcp_ws.next_layer().async_connect(endpoints,
        [&tcp_ws, url, &send_buffer, &receive_buffer](auto ec, auto _){
            OnConnect(tcp_ws, url, send_buffer, receive_buffer, ec);
        }
    );
}


int main()
{
    try {
        asio::io_context asio_context{};
        beast::websocket::stream<beast::tcp_stream> tcp_wss{asio_context};

        asio::const_buffer send_buffer{"Echo message", 13};
        beast::flat_buffer recieve_buffer{};

        asio::ip::tcp::resolver resolver{asio_context};
        resolver.async_resolve(kHost, kPort,
            [&tcp_wss, &send_buffer, &recieve_buffer](auto ec, auto endpoints){
                OnResolve(tcp_wss, kHost, send_buffer, recieve_buffer, ec, endpoints);
            }
        );

        asio_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
