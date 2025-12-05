#pragma once

#include "../../shared/networking/messages.h"
#include "../../shared/layer/app.h"

#include <SFML/Network.hpp>
#include <deque>

struct ClientConnectionState
{
    enum class ConnectionState
    {
        NotReady,
        Ready,
        Syncing,
        Disconnected
    };

    ClientConnectionState() = default;
    ~ClientConnectionState() = default;
    ConnectionState connectionState = ConnectionState::Syncing;

    void nextState();
    void disconnected();
    void finishedSyncing();
    bool isReady();
    bool isConnectedAndNotSyncing();
    bool isReceiving();
    bool isSyncing();
    bool isDisconnected();
};

class ClientConnection
{
    public:
    ClientConnection(int id, std::unique_ptr<sf::TcpSocket> socket);
    ~ClientConnection() = default;

    int id;
    std::unique_ptr<sf::TcpSocket> socket;
    ClientConnectionState connectionState = ClientConnectionState();
    std::deque<ShipMessage> shipMessageHistory = {};
    float interpolationTime;
    std::deque<float> pingHistory = {};
    float averagePing = 0.f;
    float theirGameTime = 0.f;
    float theirDeltaTime = 0.f;
    float deltaWithServer = 0.f;
    bool syncRequestSent = false;

    ShipMessage interpolateShipMessages(float deltaTime, float totalTime);
    void updateAveragePing();

    private:
    static sf::Vector2f interpolateVectorLinear(
        sf::Vector2f vector0, sf::Vector2f vector1, float interpolationTime
    );
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

    void init(ServerConfig *config);
    bool checkListenAvailable();
    void acceptPlayerConnections();
    void scanSelector();
    void checkForSyncs();
    std::vector<std::unique_ptr<Message>> receiveDataFromClients();
    void sendPlayerConnectMessage(int32_t serverId);
    void relayNetworkUpdates(
        const std::vector<std::unique_ptr<Message>> &networkUpdates
    );
    std::vector<std::unique_ptr<Message>>
    processClientUpdates(std::vector<std::unique_ptr<Message>> clientUpdates);
    void sendPlayerPingCheckMessages();
    void sendPingCheckMessage(const ClientConnection *clientConnection);
    std::vector<std::unique_ptr<ClientConnection>> clientConnections;
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
