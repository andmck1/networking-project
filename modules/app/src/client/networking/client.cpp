#include "client.h"

#include "../../shared/config/settings.h"
#include <format>
#include <spdlog/spdlog.h>

ClientConfig::ClientConfig(
    unsigned short serverPort, unsigned short clientPort, float *updateRate,
    sf::Time *deltaTime, sf::Time *totalTime, sf::IpAddress serverIPAddress
)
{
    this->serverPort = serverPort;
    this->clientPort = clientPort;
    this->serverIPAddress = serverIPAddress;
    this->updateRate = updateRate;
    this->deltaTime = deltaTime;
    this->totalTime = totalTime;
}

void Client::init(ClientConfig *config)
{
    this->serverPort = config->serverPort;
    this->clientPort = config->clientPort;
    this->serverIPAddress = config->serverIPAddress;
    this->updateRate = config->updateRate;
    this->deltaTime = config->deltaTime;
    this->totalTime = config->totalTime;

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
            spdlog::info("Connected to server!");
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

    this->tcpSocket.setBlocking(true);
    sf::Socket::Status status = this->tcpSocket.receive(packet);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::info("Received player connect message");
            std::unique_ptr<Message> message =
                Message::deserialiseMessageFromPacket(packet);

            // TODO - check logic
            // Should really check for ping first but it doesn't matter too
            // much as well update quickly
            this->serverTime = message->time;

            this->serverId = message->id;
            spdlog::info(
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
    this->tcpSocket.setBlocking(false);
    return false;
}

bool Client::syncWithServer()
{
    spdlog::info("Syncing with server");
    this->tcpSocket.setBlocking(true);

    float serverSyncTime = 0.f;
    for (int pingCheckCount = 0;
         pingCheckCount < Settings::Networking::maxPingCheckMessageHistory;
         pingCheckCount++)
    {
        sf::Packet packet;
        sf::Socket::Status status = tcpSocket.receive(packet);
        switch (status)
        {
            case (sf::Socket::Status::Done):
            {
                spdlog::info(
                    std::format(
                        "Received sync message {} / {}", pingCheckCount + 1,
                        Settings::Networking::maxPingCheckMessageHistory
                    )
                );
                std::vector<std::unique_ptr<Message>> messages =
                    Message::deserialiseMessagesFromPacket(packet);
                if (messages.empty())
                {
                    spdlog::error("Failed to sync, no messages received");
                    return false;
                }
                PingCheckMessage *pingCheckMessage =
                    static_cast<PingCheckMessage *>(messages[0].get());
                serverSyncTime =
                    pingCheckMessage->time + pingCheckMessage->ping;
                this->ping = pingCheckMessage->ping;
                this->sendPingCheckMessageResponse(pingCheckMessage);
                break;
            }
            default:
            {
                spdlog::error("Unexpected error connecting to server");
                return false;
                break;
            }
        }
    }
    this->serverTime = serverSyncTime;
    this->tcpSocket.setBlocking(false);
    return true;
}

std::vector<std::unique_ptr<Message>>
Client::prepareNetworkUpdates(AppLayer *appLayer)
{
    std::vector<std::unique_ptr<Message>> networkUpdates;

    // ship
    ShipMessage shipUpdateMessage;
    shipUpdateMessage.id = this->serverId;
    shipUpdateMessage.time = appLayer->totalTime.asSeconds();
    shipUpdateMessage.position = appLayer->player->position;
    shipUpdateMessage.forwardVector = appLayer->player->forwardVector;
    shipUpdateMessage.turnDirection = appLayer->player->currentTurnDirection;
    networkUpdates.push_back(std::make_unique<ShipMessage>(shipUpdateMessage));

    // projectiles
    while (!appLayer->shootEvents.empty())
    {
        std::unique_ptr<ShootMessage> &shootEvent =
            appLayer->shootEvents.back();
        shootEvent->id = this->serverId;
        shootEvent->time = appLayer->totalTime.asSeconds();
        networkUpdates.push_back(std::move(shootEvent));

        appLayer->shootEvents.pop_back();
    }

    return networkUpdates;
}

void Client::sendNetworkUpdates(AppLayer *appLayer)
{
    this->sendTimer += appLayer->deltaTime.asSeconds();
    if (this->sendTimer > *this->updateRate)
    {
        spdlog::debug("Sending network updates");

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
        switch (status)
        {
            case (sf::Socket::Status::Done):
            {
                spdlog::debug("Packet sent to server");
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

std::vector<std::unique_ptr<Message>> Client::receiveNetworkUpdates()
{
    // TODO - copied code from server, should share?
    sf::Packet packet;

    // TODO - single receive, should loop until not ready?
    std::vector<std::unique_ptr<Message>> allReceivedMessages = {};
    sf::Socket::Status status = this->tcpSocket.receive(packet);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug("Deserialising");
            spdlog::debug(std::to_string(packet.getDataSize()));
            std::vector<std::unique_ptr<Message>> receivedMessages;
            receivedMessages = Message::deserialiseMessagesFromPacket(packet);

            for (std::unique_ptr<Message> &message : receivedMessages)
            {
                if (message->type == MessageType::PingCheck)
                {
                    PingCheckMessage *pingCheckMessage =
                        static_cast<PingCheckMessage *>(message.get());
                    this->ping = pingCheckMessage->ping;
                    this->sendPingCheckMessageResponse(pingCheckMessage);
                }
                else
                    allReceivedMessages.push_back(std::move(message));
            }
            spdlog::debug(
                std::to_string(allReceivedMessages.size()) +
                " messages received"
            );
            break;
        }
        case (sf::Socket::Status::Disconnected):
        {
            spdlog::error("Server disconnected while sending packet");
            break;
        }
        case (sf::Socket::Status::NotReady):
        {
            break;
        }
        default:
        {
            spdlog::error(
                "Unexpected error reading data from ready client "
                "connection"
            );
            break;
        }
    }
    return allReceivedMessages;
}

void Client::sendPingCheckMessageResponse(
    PingCheckMessage *receivedRequestMessage
)
{
    sf::Packet packet;
    PingCheckMessage responseMessage = PingCheckMessage();
    responseMessage.id = this->serverId;
    responseMessage.time = this->totalTime->asSeconds();
    responseMessage.echoTime = receivedRequestMessage->time;

    packet << (int32_t) 1;
    packet << responseMessage;

    sf::Socket::Status status = this->tcpSocket.send(packet);
    while (status == sf::Socket::Status::Partial)
        status = this->tcpSocket.send(packet);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug("Packet sent to server");
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
}
