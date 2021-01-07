#include <network-monitor/websocket-client.hpp>

#include <iostream>

#include <boost/asio/buffer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/buffers_to_string.hpp>


static void LogError(std::string_view where, boost::beast::error_code ec)
{
    std::cerr << '[' << where << "] Error: " << ec.message() << std::endl;
}


namespace NetworkMonitor
{

WebSocketClient::WebSocketClient(std::string_view url, std::string_view port, boost::asio::io_context &ioc)
    : m_url{url}
    , m_port{port}
    , m_resolver{ioc}
    , m_websocket{ioc}
{}

void
WebSocketClient::Connect(
    ConnectHandler onConnect /*= nullptr*/,
    MessageHandler onMessage /*= nullptr*/,
    DisconnectHandler onDisconnect /*= nullptr*/)
{
    m_onConnectUserHandler = onConnect;
    m_onMessageUserHandler = onMessage;
    m_onDisconnectUserHandler = onDisconnect;

    m_resolver.async_resolve(
        m_url, m_port,
        [this](boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type endpoints) {
            _onResolve(ec, endpoints);
        }
    );
}

void
WebSocketClient::Send(const std::string &message, SendHandler onSend /*= nullptr*/)
{
    // NOTE: Why they take message as const&, but then std::move it into buffer
    m_websocket.async_write(
        boost::asio::const_buffer{message.data(), message.size()},
        [onSend](boost::beast::error_code ec, std::size_t bytesTransferred){
            onSend(ec);
        }
    );
}

void
WebSocketClient::Close(CloseHandler onClose /*= nullptr*/)
{
    // TODO: check if onClose!=null
    m_websocket.async_close(
        boost::beast::websocket::close_code::normal,
        [onClose](boost::beast::error_code ec) {
            onClose(ec);
        }
    );
}


void
WebSocketClient::_onResolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type endpoints)
{
    if (ec) {
        LogError("WebSocketClient::_onResolve", ec);
        // TODO: check if m_onConnect!=null
        m_onConnectUserHandler(ec);
        return;
    }
    m_websocket.next_layer().async_connect(
        endpoints,
        [this](boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type _) {
            _onConnect(ec);
        }
    );
}

void
WebSocketClient::_onConnect(boost::beast::error_code ec)
{
    if (ec) {
        LogError("WebSocketClient::_onConnect", ec);
        // TODO: check if m_onConnect!=null
        m_onConnectUserHandler(ec);
        return;
    }
    m_websocket.async_handshake(
        m_url, "/",
        [this](boost::beast::error_code ec) {
            _onHandshake(ec);
        }
    );
}

void
WebSocketClient::_onHandshake(boost::beast::error_code ec)
{
    if (ec) {
        LogError("WebSocketClient::_onHandshake", ec);
    } else {
        _listenToIncomingMessages(ec);
    }
    // TODO: check if m_onConnect!=null
    m_onConnectUserHandler(ec);
}

void
WebSocketClient::_listenToIncomingMessages(boost::beast::error_code ec)
{
    if (ec) {
        LogError("WebSocketClient::_listenToIncomingMessages", ec);
        // TODO: check if m_onConnect!=null
        m_onDisconnectUserHandler(ec);
        return;
    }
    m_websocket.async_read(
        m_recieveBuffer,
        [this](boost::beast::error_code ec, std::size_t bytesTransferred) {
            // NOTE: websocket::close() got called and all async callbacks canceled
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }
            _onRead(ec, bytesTransferred);
            _listenToIncomingMessages(ec);
        }
    );
}

void
WebSocketClient::_onRead(boost::beast::error_code ec, std::size_t bytesTransferred)
{
    if (ec) {
        LogError("WebSocketClient::_onRead", ec);
        return;
    }
    if (m_onMessageUserHandler != nullptr) {
        m_onMessageUserHandler(ec, boost::beast::buffers_to_string(m_recieveBuffer.data()));
    }
    m_recieveBuffer.consume(bytesTransferred);
}

} // namespace NetworkMonitor
