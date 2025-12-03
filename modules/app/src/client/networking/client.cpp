#include "client.h"

#include "../../shared/config/settings.h"

#include <spdlog/spdlog.h>

ClientConfig::ClientConfig(
    unsigned short serverPort, unsigned short clientPort,
    sf::IpAddress serverIPAddress
)
{
    this->serverPort = serverPort;
    this->clientPort = clientPort;
    this->serverIPAddress = serverIPAddress;
}

void Client::init(ClientConfig *config)
{
    this->serverPort = config->serverPort;
    this->clientPort = config->clientPort;
    this->serverIPAddress = config->serverIPAddress;
    this->sendRate = Settings::Networking::clientUpdateRate;
    this->isConnectedToServer = false;
    this->tcpSocket = sf::TcpSocket();
    // TODO - support non blocking calls
    this->tcpSocket.setBlocking(false);
}

bool Client::connectToServer()
{
    spdlog::info("Connecting to server");
    this->tcpSocket.setBlocking(true);
    sf::Socket::Status status =
        tcpSocket.connect(this->serverIPAddress, this->serverPort);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug("Connected to server!");
            this->isConnectedToServer = true;
            break;
        }
        case (sf::Socket::Status::NotReady):
            break;
        default:
        {
            spdlog::error("Unexpected error connecting to server");
            break;
        }
    }

    if (this->isConnectedToServer)
    {
        spdlog::info("Waiting for player connect confirmation");
        this->isConnectedToServer = this->receivePlayerConnectMessage();
    }
    this->tcpSocket.setBlocking(false);
    return this->isConnectedToServer;
}

bool Client::receivePlayerConnectMessage()
{
    sf::Packet packet;
    sf::Socket::Status status = this->tcpSocket.receive(packet);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug("Received player connect message");
            std::unique_ptr<Message> message =
                Message::deserialiseMessageFromPacket(packet);
            this->serverId = message->id;
            spdlog::debug(
                "Server assigned ID: " + std::to_string(this->serverId)
            );
            return true;
            break;
        }
        default:
        {
            spdlog::error("Unexpected error connecting to server");
            break;
        }
    }
    return false;
}

std::vector<std::unique_ptr<Message>>
Client::prepareNetworkUpdates(AppLayer *appLayer)
{
    std::vector<std::unique_ptr<Message>> networkUpdates;

    ShipMessage shipUpdateMessage;

    shipUpdateMessage.time = appLayer->getGameTime();
    // TODO - change to given server id
    shipUpdateMessage.id = this->serverId;
    shipUpdateMessage.position = appLayer->getPlayer()->position;
    shipUpdateMessage.forwardVector = appLayer->getPlayer()->forwardVector;
    networkUpdates.push_back(std::make_unique<ShipMessage>(shipUpdateMessage));

    return networkUpdates;
}

void Client::sendNetworkUpdate(AppLayer *appLayer)
{
    this->sendTimer += appLayer->getDeltaTime();
    if (this->sendTimer > this->sendRate)
    {
        spdlog::debug("Sending network update");

        sf::Packet packet;
        std::vector<std::unique_ptr<Message>> networkUpdates =
            this->prepareNetworkUpdates(appLayer);
        int32_t nNetworkUpdates = networkUpdates.size();

        // serialise packet with all network updates
        packet << nNetworkUpdates;
        for (const std::unique_ptr<Message> &networkUpdate : networkUpdates)
        {
            packet << *networkUpdate;
        }

        // send packet
        spdlog::debug(
            "Sending " + std::to_string(nNetworkUpdates) +
            " messages in one packet"
        );
        sf::Socket::Status status = this->tcpSocket.send(packet);
        status = this->tcpSocket.send(packet);
        switch (status)
        {
            case (sf::Socket::Status::Done):
            {
                spdlog::debug("Packet send to server");
                break;
            }
            case (sf::Socket::Status::Disconnected):
            {
                spdlog::error("Server disconnected while sending packet");
                break;
            }
            case (sf::Socket::Status::NotReady):
            {
                spdlog::error("Server not able to send/receive data");
                break;
            }
            default:
            {
                spdlog::warn("Unexpected error sending packet to server");
                break;
            }
        }

        spdlog::debug(
            "Sent " + std::to_string(nNetworkUpdates) +
            " messages in one packet"
        );
        this->sendTimer = 0.f;
    }
}
