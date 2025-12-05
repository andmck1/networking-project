#pragma once

#include "../../shared/layer/app.h"
#include "../../shared/networking/messages.h"
#include <SFML/Network.hpp>

struct ClientConfig
{
    ClientConfig(
        unsigned short serverPort, unsigned short clientPort, float *updateRate,
        sf::Time *deltaTime, sf::Time *totalTime,
        sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost
    );
    ~ClientConfig() = default;

    unsigned short serverPort;
    unsigned short clientPort;
    sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost;
    float *updateRate;
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
    void sendNetworkUpdates(AppLayer *appLayer);
    std::vector<std::unique_ptr<Message>> receiveNetworkUpdates();

    int32_t serverId;
    float serverTime;
    float ping = 0.f;

    private:
    float sendTimer = 0.f;
    float *updateRate;
    unsigned short serverPort;
    unsigned short clientPort;
    sf::TcpSocket tcpSocket;
    bool isConnectedToServer = false;
    sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost;
    sf::Time *deltaTime;
    sf::Time *totalTime;

    std::vector<std::unique_ptr<Message>>
    prepareNetworkUpdates(AppLayer *appLayer);
    bool receivePlayerConnectMessage();
    void sendPingCheckMessageResponse(PingCheckMessage *receivedRequestMessage);
};
