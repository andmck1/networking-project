#pragma once

#include "../../shared/networking/messages.h"
#include "../../shared/layer/app.h"
#include "../../shared/networking/clientconnection.h"

#include <SFML/Network.hpp>

struct ServerConfig
{
    ServerConfig(unsigned short port, sf::Time *deltaTime, sf::Time *totalTime);
    ~ServerConfig() = default;

    unsigned short port;
    sf::Time *deltaTime;
    sf::Time *totalTime;
};

class Server
{
    public:
    Server() = default;
    ~Server() = default;

    std::vector<std::unique_ptr<ClientConnection>> clientConnections;
    std::vector<std::unique_ptr<sf::TcpSocket>> connectingClientSockets;
    std::unordered_map<int32_t, ClientConnection *> clientConnectionsMap = {};

    void init(ServerConfig *config);
    bool checkListenAvailable();
    void acceptPlayerConnections();
    void createNewClient(std::unique_ptr<sf::TcpSocket> socket);
    void scanSelector();
    void checkForSyncs();
    std::vector<std::unique_ptr<Message>> receiveDataFromClients();
    void relayNetworkUpdates(
        const std::vector<std::unique_ptr<Message>> &networkUpdates
    );
    std::vector<std::unique_ptr<Message>>
    processClientUpdates(std::vector<std::unique_ptr<Message>> clientUpdates);
    void sendPlayerPingCheckMessages();
    void sendPingCheckMessage(ClientConnection *clientConnection);
    void frameClientConnections();
    std::vector<std::unique_ptr<Message>>
    prepareGameUpdates(AppLayer *appLayer);

    private:
    unsigned short port;
    sf::TcpListener listener;
    sf::SocketSelector selector;
    int clientIDCounter = 0;
    sf::Time *deltaTime;
    sf::Time *totalTime;
    float pingCheckTimer = 0.f;
};
