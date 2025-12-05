#pragma once

#include "messages.h"

#include <SFML/Network.hpp>
#include <deque>
#include <memory>

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

    void nextState();
    void disconnected();
    void syncing();
    void finishedSyncing();
    bool isReady();
    bool isConnectedAndNotSyncing();
    bool isReceiving();
    bool isSyncing();
    bool isDisconnected();
    ConnectionState connectionState = ConnectionState::Syncing;
};

class ClientConnection
{
    public:
    ClientConnection(int id, std::unique_ptr<sf::TcpSocket> socket);
    ~ClientConnection() = default;

    int id;
    std::unique_ptr<sf::TcpSocket> socket;
    std::string connectionDetails;
    std::string ipAddress;
    int port;
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
