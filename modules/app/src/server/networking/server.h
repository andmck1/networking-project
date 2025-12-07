#pragma once

#include "../../shared/networking/messages.h"
#include "../../shared/layer/app.h"
#include "../../shared/networking/clientconnection.h"

#include <SFML/Network.hpp>

struct NonBlockingTCPRelayPacket : NonBlockingTCPPacket
{
    NonBlockingTCPPacketSendStatus
    statusHandler(sf::Socket::Status status) override;
    int nNetworkUpdates = 0;
};

struct NonBlockingTCPPingCheckPacket : NonBlockingTCPPacket
{
    NonBlockingTCPPacketSendStatus
    statusHandler(sf::Socket::Status status) override;
};

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

    void sendRelayNetworkUpdates(
        const std::vector<std::unique_ptr<Message>> &networkUpdates
    );
    void sendPlayerPingCheckMessages();
    void sendPingCheckMessage(ClientConnection *clientConnection);
    void sendNonBlockingPackets();

    std::vector<std::unique_ptr<Message>>
    processClientUpdates(std::vector<std::unique_ptr<Message>> clientUpdates);
    void frameClientConnections();
    std::vector<std::unique_ptr<Message>>
    prepareGameUpdates(AppLayer *appLayer);

    bool isInterpolationEnabled = true;
    bool isPredictionEnabled = true;

    private:
    unsigned short port;
    sf::TcpListener listener;
    sf::SocketSelector selector;
    int clientIDCounter = 0;
    sf::Time *deltaTime;
    sf::Time *totalTime;
    float pingCheckTimer = 0.f;
};
