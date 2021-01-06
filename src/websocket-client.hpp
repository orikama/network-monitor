#pragma once

#include <string>
#include <string_view>
#include <functional>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/websocket/stream.hpp>


// NOTE: Pass asio::io_context as std::shared_ptr ?
// NOTE: Use smth like setCallback() instead of passing callbacks to
//  Connect, Send, Close methods ?


namespace NetworkMonitor
{

/*! \brief Client to connect to a WebSocket server over plain TCP.
 */
class WebSocketClient
{
public:
    using ConnectHandler = std::function<void (boost::beast::error_code)>;
    using SendHandler = std::function<void (boost::beast::error_code)>;
    using MessageHandler = std::function<void (boost::beast::error_code, std::string &&)>;
    using DisconnectHandler = std::function<void (boost::beast::error_code)>;
    using CloseHandler = std::function<void (boost::beast::error_code)>;


    /*! \brief Construct a WebSocket client.
     *
     *  \note This constructor does not initiate a connection.
     *
     *  \param url  The URL of the server.
     *  \param port The port on the server.
     *  \param ioc  The io_context object. The user takes care of calling ioc.run().
     */
    WebSocketClient(
        const std::string_view url,
        const std::string_view port,
        boost::asio::io_context &ioc
    );

    /*! \brief Destructor
     */
    ~WebSocketClient() = default;

    /*! \brief Connect to the server.
     *
     *  \param onConnect    Called when the connection fails or succeeds.
     *  \param onMessage    Called when a message is successfully received.
     *                      The message is an rvalue reference; ownership is passed to the receiver.
     *  \param onDisconnect Called when the connection is closed by the server or due to a connection error.
     */
    void Connect(
        ConnectHandler onConnect = nullptr,
        MessageHandler onMessage = nullptr,
        DisconnectHandler onDisconnect = nullptr
    );

    /*! \brief Send a text message to the WebSocket server.
     *
     *  \param message The message to send.
     *  \param onSend  Called when a message is sent successfully or if it failed to send.
     */
    void Send(
        const std::string &message,
        SendHandler onSend = nullptr
    );

    /*! \brief Close the WebSocket connection.
     *
     *  \param onClose Called when the connection is closed, successfully or not.
     */
    void Close(
        CloseHandler onClose = nullptr
    );

private:
    void _onResolve(
        boost::beast::error_code ec,
        boost::asio::ip::tcp::resolver::results_type endpoints
    );
    void _onConnect(boost::beast::error_code ec);
    void _onHandshake(boost::beast::error_code ec);
    void _listenToIncomingMessages(boost::beast::error_code ec);
    void _onRead(boost::beast::error_code ec, std::size_t bytesTransferred);

private:
    const std::string m_url;
    const std::string m_port;

    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::beast::tcp_stream> m_websocket;

    boost::beast::flat_buffer m_recieveBuffer;

    ConnectHandler m_onConnectUserHandler;
    MessageHandler m_onMessageUserHandler;
    DisconnectHandler m_onDisconnectUserHandler;
};

} // namespace NetworkMonitor
