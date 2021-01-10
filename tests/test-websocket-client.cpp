#include <filesystem>
#include <string>

#include <boost/test/unit_test.hpp>

#include <network-monitor/websocket-client.hpp>


BOOST_AUTO_TEST_SUITE( network_monitor )

BOOST_AUTO_TEST_CASE(cacert_pem)
{
    BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}

BOOST_AUTO_TEST_CASE( class_WebSocketClient )
{
    constexpr auto kHost{"echo.websocket.org"};
    constexpr auto kPort{"80"};
    const std::string kMessage{"Hello WebSocket"};

    boost::asio::io_context asioContext{};
    NetworkMonitor::WebSocketClient wsClient{kHost, kPort, asioContext};

    bool connected{false};
    bool messageSent{false};
    bool messageReceived{false};
    bool disconnected{false};
    std::string echoResponse{};

    auto onSend{[&messageSent](boost::beast::error_code ec) {
        messageSent = !ec;
    }};
    auto onConnect{[&connected, &wsClient, &onSend, kMessage](boost::beast::error_code ec) {
        connected = !ec;
        if (!ec) {
            wsClient.Send(kMessage, onSend);
        }
    }};
    auto onDisconnect{[&disconnected](boost::beast::error_code ec) {
        disconnected = !ec;
    }};
    auto onReceive{
        [&messageReceived, &echoResponse, &wsClient, &onDisconnect]
        (boost::beast::error_code ec, std::string &&received) {
            messageReceived = !ec;
            echoResponse = std::move(received);
            wsClient.Close(onDisconnect);
    }};

    wsClient.Connect(onConnect, onReceive);
    asioContext.run();

    BOOST_CHECK(connected);
    BOOST_CHECK(messageSent);
    BOOST_CHECK(messageReceived);
    BOOST_CHECK(disconnected);
    BOOST_CHECK_EQUAL(kMessage, echoResponse);
}

BOOST_AUTO_TEST_SUITE_END()
