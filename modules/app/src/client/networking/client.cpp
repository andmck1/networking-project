#include "client.h"

#include "../../shared/config/settings.h"
#include <format>
#include <spdlog/spdlog.h>

NonBlockingTCPPacketSendStatus
NonBlockingTCPNetworkUpdatePacket::statusHandler(sf::Socket::Status status)
{
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug("Packet sent to server");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Done;
            break;
        }
        case (sf::Socket::Status::Partial):
        {
            spdlog::error("Server not able to send/receive data");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Sending;
            break;
        }
        case (sf::Socket::Status::NotReady):
        {
            spdlog::error("Server connection unable to receive data.");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Sending;
            break;
        }
        case (sf::Socket::Status::Disconnected):
        {
            spdlog::error("Server disconnected while sending packet");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Error;
            break;
        }
        default:
        {
            spdlog::warn("Unexpected error sending packet to server");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Error;
            break;
        }
    }
    return this->sendStatus;
}

NonBlockingTCPPacketSendStatus
NonBlockingTCPPingCheckResponsePacket::statusHandler(sf::Socket::Status status)
{
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug("Ping check message sent");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Done;
            break;
        }
        case (sf::Socket::Status::NotReady):
        {
            spdlog::error(
                "Server not ready while sending ping check message response"
            );
            this->sendStatus = NonBlockingTCPPacketSendStatus::Sending;
            break;
        }
        case (sf::Socket::Status::Partial):
        {
            this->sendStatus = NonBlockingTCPPacketSendStatus::Sending;
            break;
        }
        case (sf::Socket::Status::Disconnected):
        {
            spdlog::error(
                "Server disconnected while sending ping check message response"
            );
            this->sendStatus = NonBlockingTCPPacketSendStatus::Error;
            break;
        }
        default:
        {
            spdlog::error(
                "Unexpected error sending ping check message to server"
            );
            this->sendStatus = NonBlockingTCPPacketSendStatus::Error;
            break;
        }
    }
    return this->sendStatus;
}

ClientConfig::ClientConfig(
    unsigned short serverPort, unsigned short clientPort, float updateRate,
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
    this->tcpSocket.setBlocking(false);

    // Some copilot help for this one
    this->randomNumberGenerator = std::mt19937(this->randomDevice());
    this->randomNumberDistribution =
        std::uniform_int_distribution<int32_t>(0, 1000000 - 1);
}

bool Client::connectToServer()
{
    sf::Socket::Status status;
    spdlog::info("Connecting to server");
    this->tcpSocket.setBlocking(true);
    status = this->tcpSocket.connect(
        this->serverIPAddress, this->serverPort, sf::seconds(1.f)
    );
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::info("Connected to server!");
            this->isConnectedToServer = true;
            this->nServerConnectTries = 0;
            this->clientPort = tcpSocket.getLocalPort();

            spdlog::info("Waiting for player connect confirmation");
            this->isConnectedToServer = this->sendPlayerConnectMessage();
            break;
        }
        case (sf::Socket::Status::NotReady):
            spdlog::info(
                "Waiting for connectiong to server. Try {}/{}",
                this->nServerConnectTries + 1,
                Settings::Networking::maxClientConnectToServerTries
            );
            this->nServerConnectTries += 1;
            break;
        default:
        {
            spdlog::error(
                "Unexpected error connecting to server, try {}/{}",
                this->nServerConnectTries + 1,
                Settings::Networking::maxClientConnectToServerTries
            );
            this->nServerConnectTries += 1;
            this->disconnectAndRestConnectionStatusWithServer();
            break;
        }
    }
    this->tcpSocket.setBlocking(false);
    return this->isConnectedToServer;
}

bool Client::sendPlayerConnectMessage()
{
    sf::Packet packet;
    PlayerConnectMessage message = PlayerConnectMessage();
    int32_t id;
    if (this->serverId < 0)
        id = this->generateRandomId();
    else
        id = this->serverId;
    message.id = id;
    message.time = this->totalTime->asSeconds();
    message.ping = this->ping;
    packet << (int32_t) 1;
    packet << message;

    this->tcpSocket.setBlocking(true);
    sf::Socket::Status status = this->tcpSocket.send(packet);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::info("Sent player connect message");
            this->serverId = id;
            spdlog::info("ID {} assigned", id);
            return true;
        }
        default:
        {
            spdlog::error("Unexpected error receiving player connect message");
            this->disconnectAndRestConnectionStatusWithServer();
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
                this->lastRecordedServerTime = pingCheckMessage->time;
                this->ping = pingCheckMessage->ping;
                this->estimatedServerTime =
                    pingCheckMessage->time + pingCheckMessage->ping;
                this->sendPingCheckMessageResponse(pingCheckMessage);
                break;
            }
            default:
            {
                spdlog::error("Unexpected error receiving sync message");
                this->nServerConnectTries++;
                return false;
                break;
            }
        }
    }
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
    shipUpdateMessage.ping = this->ping;
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
        shootEvent->ping = this->ping;
        networkUpdates.push_back(std::move(shootEvent));

        appLayer->shootEvents.pop_back();
    }

    return networkUpdates;
}

void Client::sendNetworkUpdates(AppLayer *appLayer)
{
    this->sendTimer += appLayer->deltaTime.asSeconds();
    if (this->sendTimer > this->updateRate)
    {
        spdlog::debug("Sending network updates");

        NonBlockingTCPNetworkUpdatePacket networkUpdatePacket;
        std::vector<std::unique_ptr<Message>> networkUpdates =
            this->prepareNetworkUpdates(appLayer);
        int32_t nNetworkUpdates = networkUpdates.size();

        // serialise packet with all network updates
        networkUpdatePacket.packet << nNetworkUpdates;
        for (const std::unique_ptr<Message> &networkUpdate : networkUpdates)
        {
            networkUpdatePacket.packet << *networkUpdate;
        }
        this->nonBlockingSendPackets.push_back(
            std::make_unique<NonBlockingTCPNetworkUpdatePacket>(
                networkUpdatePacket
            )
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
                    this->lastRecordedServerTime = pingCheckMessage->time;
                    this->lastRecordedServerDeltaTime =
                        pingCheckMessage->deltaTime;
                    this->estimatedServerTime =
                        pingCheckMessage->time + pingCheckMessage->ping;
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
            this->isConnectedToServer = false;
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
            this->isConnectedToServer = false;
            break;
        }
    }
    return allReceivedMessages;
}

void Client::sendPingCheckMessageResponse(
    PingCheckMessage *receivedRequestMessage
)
{
    NonBlockingTCPPingCheckResponsePacket pingCheckPacket;
    PingCheckMessage responseMessage = PingCheckMessage();
    responseMessage.id = this->serverId;
    responseMessage.time = this->totalTime->asSeconds();
    responseMessage.echoTime = receivedRequestMessage->time;
    responseMessage.deltaTime = this->deltaTime->asSeconds();
    responseMessage.ping = this->ping;

    pingCheckPacket.packet << (int32_t) 1;
    pingCheckPacket.packet << responseMessage;

    this->nonBlockingSendPackets.push_back(
        std::make_unique<NonBlockingTCPPingCheckResponsePacket>(pingCheckPacket)
    );
}

void Client::sendNonBlockingPackets()
{
    for (auto iterator = this->nonBlockingSendPackets.begin();
         iterator != this->nonBlockingSendPackets.end();)
    {
        std::unique_ptr<NonBlockingTCPPacket> &packet = *iterator;
        sf::Socket::Status status = this->tcpSocket.send(packet->packet);
        NonBlockingTCPPacketSendStatus sendStatus =
            packet->statusHandler(status);
        switch (sendStatus)
        {
            case (NonBlockingTCPPacketSendStatus::Done):
            {
                iterator = this->nonBlockingSendPackets.erase(iterator);
                break;
            }
            case (NonBlockingTCPPacketSendStatus::Sending):
            {
                iterator++;
                break;
            }
            case (NonBlockingTCPPacketSendStatus::Error):
            {
                iterator = this->nonBlockingSendPackets.erase(iterator);
                this->isConnectedToServer = false;
                break;
            }
            default:
            {
                spdlog::warn("Non blocking send status not handled");
                iterator = this->nonBlockingSendPackets.erase(iterator);
                this->isConnectedToServer = false;
            }
        }
    }
}

void Client::disconnectFromServer()
{
    this->tcpSocket.disconnect();
}

void Client::disconnectAndRestConnectionStatusWithServer()
{
    this->tcpSocket.disconnect();
    this->tcpSocket = sf::TcpSocket();
    this->tcpSocket.setBlocking(false);
    this->disconnectFromServer();
    this->isConnectedToServer = false;
}

int32_t Client::generateRandomId()
{
    return this->randomNumberDistribution(this->randomNumberGenerator);
}

void Client::frameServerTime()
{
    this->estimatedServerTime += this->deltaTime->asSeconds();
}
