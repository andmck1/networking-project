#pragma once

#include "../../shared/networking/messages.h"

#include <SFML/Network.hpp>

struct ClientConnection
{
    // Lab code
    enum class ConnectionState
    {
        NotReady,
        Ready,
        Disconnected
    };

    ClientConnection(int id, std::unique_ptr<sf::TcpSocket> socket);
    ~ClientConnection() = default;

    int id;
    std::unique_ptr<sf::TcpSocket> socket;
    ConnectionState connectionState = ConnectionState::Disconnected;
};

struct ServerConfig
{
    ServerConfig(unsigned short port);
    ~ServerConfig() = default;

    unsigned short port;
};

class Server
{
    public:
    Server() = default;
    ~Server() = default;

    void init(ServerConfig *config);
    bool checkListenAvailable();
    void acceptPlayerConnections();
    void scanSelector();
    std::vector<std::unique_ptr<Message>> receiveDataFromClients();
    void sendPlayerConnectMessage(int32_t serverId);

    private:
    unsigned short port;
    sf::TcpListener listener;
    sf::SocketSelector selector;
    std::vector<std::unique_ptr<ClientConnection>> clientConnections;
    int clientIDCounter = 0;
};
