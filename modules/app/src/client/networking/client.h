#pragma once

#include "../../shared/layer/app.h"
#include "../../shared/networking/messages.h"
#include <SFML/Network.hpp>

struct ClientConfig
{
    ClientConfig(
        unsigned short serverPort, unsigned short clientPort,
        sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost
    );
    ~ClientConfig() = default;

    unsigned short serverPort;
    unsigned short clientPort;
    sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost;
};

class Client
{
    public:
    Client() = default;
    ~Client() = default;

    void init(ClientConfig *config);
    bool connectToServer();
    void sendNetworkUpdates(AppLayer *appLayer);
    std::vector<std::unique_ptr<Message>>
    receiveNetworkUpdates();

    private:
    float sendTimer = 0.f;
    float sendRate;
    int32_t serverId;
    unsigned short serverPort;
    unsigned short clientPort;
    sf::TcpSocket tcpSocket;
    bool isConnectedToServer = false;
    sf::IpAddress serverIPAddress = sf::IpAddress::LocalHost;

    std::vector<std::unique_ptr<Message>>
    prepareNetworkUpdates(AppLayer *appLayer);
    bool receivePlayerConnectMessage();
};
