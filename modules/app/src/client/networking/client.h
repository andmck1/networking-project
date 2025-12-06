#pragma once

#include "../../shared/layer/app.h"
#include "../../shared/networking/messages.h"
#include <SFML/Network.hpp>

struct ClientConfig
{
    ClientConfig(
        unsigned short serverPort, unsigned short clientPort, float updateRate,
        sf::Time *deltaTime, sf::Time *totalTime,
        sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost
    );
    ~ClientConfig() = default;

    unsigned short serverPort;
    unsigned short clientPort;
    sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost;
    float updateRate;
    sf::Time *deltaTime;
    sf::Time *totalTime;
};

class Client
{
    public:
    Client() = default;
    ~Client() = default;

    void init(ClientConfig *config);
    bool connectToServer();
    bool syncWithServer();
    void sendNetworkUpdates(AppLayer *appLayer);
    void disconnectFromServer();
    void disconnectAndRestConnectionStatusWithServer();
    std::vector<std::unique_ptr<Message>> receiveNetworkUpdates();
    int32_t generateRandomId();
    void frameServerTime();

    int32_t serverId = -1;
    float lastRecordedServerTime = 0.f;
    float lastRecordedServerDeltaTime = 0.f;
    float estimatedServerTime = 0.f;
    float ping = 0.f;
    bool isConnectedToServer = false;
    bool isConnectionEnabled = true;
    sf::TcpSocket tcpSocket;
    int nServerConnectTries = 0;
    float updateRate;
    unsigned short clientPort = 0;
    unsigned short serverPort = 0;
    sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost;

    private:
    float sendTimer = 0.f;
    sf::Time *deltaTime;
    sf::Time *totalTime;

    std::random_device randomDevice;
    std::mt19937 randomNumberGenerator;
    std::uniform_int_distribution<int32_t> randomNumberDistribution;

    std::vector<std::unique_ptr<Message>>
    prepareNetworkUpdates(AppLayer *appLayer);
    bool sendPlayerConnectMessage();
    void sendPingCheckMessageResponse(PingCheckMessage *receivedRequestMessage);
};
