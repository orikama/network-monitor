#include <iostream>
#include <string>

#include <websocket-client.hpp>


int main()
{
    constexpr auto kHost{"echo.websocket.org"};
    constexpr auto kPort{"80"};
    const std::string kMessage{"Hello WebSocket"};

    try {
        boost::asio::io_context asioContext{};
        NetworkMonitor::WebSocketClient wsClient{kHost, kPort, asioContext};

        bool connected{false};
        bool messageSent{false};
        bool messageReceived{false};
        bool messageMatches{false};
        bool disconnected{false};

        auto onSend{[&messageSent](auto ec) {
            messageSent = !ec;
        }};
        auto onConnect{[&connected, &wsClient, &onSend, kMessage](auto ec) {
            connected = !ec;
            if (!ec) {
                wsClient.Send(kMessage, onSend);
            }
        }};
        auto onDisconnect{[&disconnected](auto ec) {
            disconnected = !ec;
        }};
        auto onReceive{[&messageReceived, &messageMatches, &wsClient, &onDisconnect, &kMessage](auto ec, auto received) {
            messageReceived = !ec;
            messageMatches = kMessage == received;
            wsClient.Close(onDisconnect);
        }};

        wsClient.Connect(onConnect, onReceive);
        asioContext.run();

        if (connected && messageSent && messageReceived && messageMatches && disconnected) {
            std::cout << "Ok\n";
        }
        else {
            std::cerr << "\nTest failed\n";
            return 1;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
