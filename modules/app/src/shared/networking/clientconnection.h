#pragma once

#include "messages.h"

#include <SFML/Network.hpp>
#include <deque>
#include <memory>

enum class NonBlockingTCPPacketSendStatus
{
    Sending,
    Done,
    Error
};

struct NonBlockingTCPPacket
{
    NonBlockingTCPPacket() = default;
    virtual ~NonBlockingTCPPacket() = default;

    sf::Packet packet;
    NonBlockingTCPPacketSendStatus sendStatus =
        NonBlockingTCPPacketSendStatus::Sending;

    virtual NonBlockingTCPPacketSendStatus
    statusHandler(sf::Socket::Status status) = 0;
};

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
    ClientConnection(std::string handle, std::unique_ptr<sf::TcpSocket> socket);
    ~ClientConnection() = default;

    std::string handle;
    std::unique_ptr<sf::TcpSocket> socket;
    std::string connectionDetails;
    std::string ipAddress;
    int port;
    ClientConnectionState connectionState = ClientConnectionState();
    std::deque<ShipMessage> shipMessageHistory = {};
    float interpolationTime;
    std::deque<float> pingHistory = {};
    float averagePing = 0.f;
    float lastRecordedGameTime = 0.f;
    float lastRecordedDeltaTime = 0.f;
    float estimatedCurrentGameTime = 0.f;
    float lastRecordedClockDrift = 0.f;
    bool syncRequestSent = false;
    sf::Vector2f previousPredictedShipPosition = {};
    sf::Vector2f previousPredictedShipForwardVector = {};
    std::vector<std::unique_ptr<NonBlockingTCPPacket>> nonBlockingSendPackets =
        {};

    ShipMessage predictShipMessage(
        float totalTime, bool isInterpolationEnabled, bool isPredictionEnabled
    );
    void updateAveragePing();

    private:
    static sf::Vector2f predictVectorWithLinearVelocity(
        sf::Vector2f currentVector, sf::Vector2f previousVector,
        float currentVectorTime, float previousVectorTime, float predictionTime
    );
    static sf::Vector2f interpolateVectorLinear(
        sf::Vector2f vector0, sf::Vector2f vector1, float interpolationFactor
    );
};
