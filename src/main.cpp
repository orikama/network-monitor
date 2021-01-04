#include <iostream>

#include <boost/system/error_code.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/websocket.hpp>


namespace asio = boost::asio;
namespace beast = boost::beast;


constexpr auto kHost{"echo.websocket.org"};
constexpr auto kPort{"80"};


// void LogError(const boost::system::error_code ec)
// {
//     std::cerr << "Error: " << ec.message() << std::endl;
// }


int main()
{
    try {
        asio::io_context asio_context{};
        beast::websocket::stream<asio::ip::tcp::socket> tcp_websocket{asio_context};

        const auto connected_endpoint = asio::connect(
            tcp_websocket.next_layer(),
            asio::ip::tcp::resolver{asio_context}.resolve(kHost, kPort)
        );
        tcp_websocket.handshake(kHost, "/");

        tcp_websocket.text(true); // NOTE: Why???
        asio::const_buffer send_buffer{"Echo message", 13};
        tcp_websocket.write(send_buffer);

        beast::flat_buffer response_buffer{};
        tcp_websocket.read(response_buffer);

        tcp_websocket.close(beast::websocket::close_code::normal);

        std::cout << beast::make_printable(response_buffer.data()) << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
