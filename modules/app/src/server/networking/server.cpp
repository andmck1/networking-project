#include "server.h"

#include "../../shared/config/settings.h"
#include <spdlog/spdlog.h>

ClientConnection::ClientConnection(
    int id, std::unique_ptr<sf::TcpSocket> socket
)
{
    this->id = id;
    this->socket = std::move(socket);
}

sf::Vector2f ClientConnection::interpolateVectorLinear(
    sf::Vector2f vector0, sf::Vector2f vector1, float interpolationTime
)
{
    return ((vector1 - vector0) * interpolationTime) + vector0;
}

ShipMessage
ClientConnection::interpolateShipMessages(float deltaTime, float totalTime)
{
    ShipMessage interpolatedShipMessage;

    ShipMessage message0 = this->shipMessageHistory[0];
    sf::Vector2f interpolatedPosition = this->shipMessageHistory[0].position;
    sf::Vector2f interpolatedForwardVector =
        this->shipMessageHistory[0].forwardVector;
    int32_t interpolatedTurnDirection =
        this->shipMessageHistory[0].turnDirection;

    // Some lab code here (linear interpolation)
    if (this->shipMessageHistory.size() > 1)
    {
        ShipMessage message1 = this->shipMessageHistory[1];
        float step =
            1.f / (((float) message0.time - (float) message1.time) / deltaTime);
        this->interpolationTime += step;
        this->interpolationTime = std::clamp(interpolationTime, 0.f, 1.f);

        interpolatedPosition = this->interpolateVectorLinear(
            message1.position, message0.position, interpolationTime
        );
        interpolatedForwardVector = this->interpolateVectorLinear(
            message1.forwardVector, message0.forwardVector, interpolationTime
        );
        interpolatedForwardVector /= interpolatedForwardVector.length();
        interpolatedTurnDirection = this->shipMessageHistory[1].turnDirection;
    }

    interpolatedShipMessage.id = message0.id;
    interpolatedShipMessage.time = totalTime;
    interpolatedShipMessage.position = interpolatedPosition;
    interpolatedShipMessage.forwardVector = interpolatedForwardVector;
    interpolatedShipMessage.turnDirection = interpolatedTurnDirection;

    return interpolatedShipMessage;
}

void ClientConnection::updateAveragePing()
{
    float cumulativePing = 0.f;
    for (const float ping : this->pingHistory)
    {
        cumulativePing += ping;
    }
    this->averagePing = cumulativePing / this->pingHistory.size();
}

ServerConfig::ServerConfig(
    unsigned short port, sf::Time *deltaTime, sf::Time *totalTime
)
{
    this->port = port;
    this->deltaTime = deltaTime;
    this->totalTime = totalTime;
}

void Server::init(ServerConfig *config)
{
    this->port = config->port;
    this->deltaTime = config->deltaTime;
    this->totalTime = config->totalTime;
    this->listener.setBlocking(false);
}

bool Server::checkListenAvailable()
{
    if (this->listener.listen(port) == sf::Socket::Status::Error)
    {
        spdlog::error("Failed to connect to player");
        return false;
    }
    return true;
}

void Server::acceptPlayerConnections()
{
    spdlog::debug("Checking for player connections");
    std::unique_ptr<sf::TcpSocket> tcpSocket =
        std::make_unique<sf::TcpSocket>();
    sf::Socket::Status status = this->listener.accept(*tcpSocket);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            // create some functions?
            // log
            std::string connectionDetailsString =
                tcpSocket->getRemoteAddress()->toString() + ":" +
                std::to_string(tcpSocket->getRemotePort());
            spdlog::info("New connection from " + connectionDetailsString);

            // add new client
            std::unique_ptr<ClientConnection> newClient =
                std::make_unique<ClientConnection>(
                    this->clientIDCounter, std::move(tcpSocket)
                );
            newClient->connectionState =
                ClientConnection::ConnectionState::NotReady;
            newClient->socket->setBlocking(false);
            this->selector.add(*newClient->socket);
            this->clientConnections.push_back(std::move(newClient));
            this->sendPlayerConnectMessage(this->clientIDCounter);
            this->clientIDCounter++;
            break;
        }
        case (sf::Socket::Status::Disconnected):
            break;
        case (sf::Socket::Status::NotReady):
            break;
        case (sf::Socket::Status::Partial):
            break;
        case (sf::Socket::Status::Error):
        {
            spdlog::warn("Unexpected error connecting to players");
            break;
        }
        default:
        {
            spdlog::error("Not implemented socket status");
        }
    }
}

void Server::sendPlayerConnectMessage(int32_t serverId)
{
    const std::unique_ptr<ClientConnection> &clientConnection =
        this->clientConnections.at(serverId);
    sf::Packet packet;
    PlayerConnectMessage message;
    message.id = serverId;
    message.time = this->totalTime->asSeconds();
    packet << message;
    sf::Socket::Status status = clientConnection->socket->send(packet);
    while (status == sf::Socket::Status::Partial)
        status = clientConnection->socket->send(packet);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug("Sent player connect message");
            break;
        }
        case (sf::Socket::Status::Disconnected):
        {
            spdlog::error(
                "Disconnected from player while sending player connect message"
            );
            break;
        }
        case (sf::Socket::Status::NotReady):
        {
            spdlog::error("Client not able to send/receive data");
            break;
        }
        default:
        {
            spdlog::error(
                "Unexpected error while sending player connect message to "
                "client"
            );
            break;
        }
    }
}

void Server::scanSelector()
{
    if (this->selector.wait(sf::milliseconds(5)))
    {
        for (const std::unique_ptr<ClientConnection> &clientConnection :
             this->clientConnections)
        {
            if (this->selector.isReady(*clientConnection->socket))
                clientConnection->connectionState =
                    ClientConnection::ConnectionState::Ready;
        }
    }
}

std::vector<std::unique_ptr<Message>> Server::receiveDataFromClients()
{
    std::vector<std::unique_ptr<Message>> receivedMessagesFromAllClients;

    // for all clients, if ready receive packet, deserialise and append messages
    for (const std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        if (clientConnection->connectionState ==
            ClientConnection::ConnectionState::Ready)
        {
            spdlog::debug("Connection ready");
            sf::Packet packet;
            // TODO - single receive, should loop until not ready?
            sf::Socket::Status status =
                clientConnection->socket->receive(packet);
            switch (status)
            {
                case (sf::Socket::Status::Done):
                {
                    spdlog::debug("Deserialising");
                    std::vector<std::unique_ptr<Message>> receivedMessages;
                    receivedMessages =
                        Message::deserialiseMessagesFromPacket(packet);

                    for (std::unique_ptr<Message> &message : receivedMessages)
                    {
                        receivedMessagesFromAllClients.push_back(
                            std::move(message)
                        );
                    }
                    clientConnection->connectionState =
                        ClientConnection::ConnectionState::NotReady;
                    break;
                }
                case (sf::Socket::Status::Disconnected):
                {
                    spdlog::error("Client disconnected while sending packet");
                    break;
                }
                case (sf::Socket::Status::NotReady):
                {
                    spdlog::error("Client not able to send/receive data");
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
        }
    }

    spdlog::debug(
        "Received " + std::to_string(receivedMessagesFromAllClients.size()) +
        " messages"
    );

    return receivedMessagesFromAllClients;
}

std::vector<std::unique_ptr<Message>> Server::processClientUpdates(
    std::vector<std::unique_ptr<Message>> clientUpdates
)
{
    std::vector<std::unique_ptr<Message>> processedMessages = {};

    for (std::unique_ptr<Message> &message : clientUpdates)
    {
        const int32_t clientId = message->id;
        const std::unique_ptr<ClientConnection> &clientConnection =
            this->clientConnections[clientId];
        switch (message->type)
        {
            case (MessageType::Ship):
            {
                const ShipMessage *shipMessage =
                    static_cast<ShipMessage *>(message.get());

                clientConnection->interpolationTime = 0.f;
                clientConnection->shipMessageHistory.push_front(*shipMessage);
                if (clientConnection->shipMessageHistory.size() >
                    Settings::Networking::maxShipMessageHistory)
                {
                    clientConnection->shipMessageHistory.pop_back();
                }
                break;
            }
            case (MessageType::Shoot):
            {
                processedMessages.push_back(std::move(message));
                break;
            }
            case (MessageType::PingCheck):
            {
                const PingCheckMessage *pingCheckMessage =
                    static_cast<PingCheckMessage *>(message.get());
                clientConnection->pingHistory.push_front(
                    (this->totalTime->asSeconds() -
                     pingCheckMessage->echoTime) /
                    2.f
                );
                if (clientConnection->pingHistory.size() >
                    Settings::Networking::maxPingCheckMessageHistory)
                {
                    clientConnection->pingHistory.pop_back();
                }
                clientConnection->updateAveragePing();
                break;
            }
            default:
            {
                spdlog::warn("Unsupported message type sent");
                break;
            }
        }
    }

    for (const std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        if (!clientConnection->shipMessageHistory.empty())
        {
            processedMessages.push_back(
                std::make_unique<ShipMessage>(
                    clientConnection->interpolateShipMessages(
                        this->deltaTime->asSeconds(),
                        this->totalTime->asSeconds()
                    )
                )
            );
        }
    }

    return processedMessages;
}

void Server::relayNetworkUpdates(
    const std::vector<std::unique_ptr<Message>> &networkUpdates
)
{
    // for all clients, create packet,
    // for all network updates, add to packet if client didn't send update
    // send network update to client
    for (const std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        sf::Packet packet;
        sf::Packet packetMessages;
        int32_t nNetworkUpdates = 0;
        for (const std::unique_ptr<Message> &networkUpdate : networkUpdates)
        {
            if (networkUpdate->id != clientConnection->id)
            {
                packetMessages << *networkUpdate;
                nNetworkUpdates++;
            }
        }
        if (nNetworkUpdates > 0)
        {
            packet << nNetworkUpdates;
            packet.append(
                packetMessages.getData(), packetMessages.getDataSize()
            );
            std::string relayInformation =
                " to " + std::to_string(clientConnection->id);

            // TODO - get rid of these loops, keep in queue and try again if
            // any left in queue
            sf::Socket::Status status = clientConnection->socket->send(packet);
            while (status == sf::Socket::Status::Partial)
                status = clientConnection->socket->send(packet);
            switch (status)
            {
                case (sf::Socket::Status::Done):
                {
                    spdlog::debug(
                        std::to_string(nNetworkUpdates) + " messages relayed " +
                        relayInformation
                    );
                    break;
                }
                case (sf::Socket::Status::NotReady):
                {
                    break;
                }
                case (sf::Socket::Status::Partial):
                {
                    break;
                }
                case (sf::Socket::Status::Disconnected):
                {
                    spdlog::error(
                        "Client disconnected when sending relay packet " +
                        relayInformation
                    );
                    break;
                }
                default:
                {
                    spdlog::error(
                        "Unexpected error when sending relay packet " +
                        relayInformation
                    );
                    break;
                }
            }
        }
    }
}

void Server::sendPlayerPingCheckMessages()
{
    this->pingCheckTimer += this->deltaTime->asSeconds();
    if (this->pingCheckTimer > Settings::Networking::serverPingCheckRate)
    {
        this->pingCheckTimer = 0.f;
        for (const std::unique_ptr<ClientConnection> &clientConnection :
             clientConnections)
        {
            sf::Packet packet;
            PingCheckMessage message = PingCheckMessage();
            message.id = clientConnection->id;
            message.time = this->totalTime->asSeconds();
            message.ping = clientConnection->averagePing;
            packet << (int32_t) 1;
            packet << message;

            std::string relayInformation =
                " to " + std::to_string(clientConnection->id);
            sf::Socket::Status status = clientConnection->socket->send(packet);
            while (status == sf::Socket::Status::Partial)
                status = clientConnection->socket->send(packet);
            switch (status)
            {
                case (sf::Socket::Status::Done):
                {
                    spdlog::debug(
                        "Player connect message sent" + relayInformation
                    );
                    break;
                }
                case (sf::Socket::Status::NotReady):
                {
                    break;
                }
                case (sf::Socket::Status::Disconnected):
                {
                    spdlog::error(
                        "Client disconnected when sending relay packet " +
                        relayInformation
                    );
                    break;
                }
                default:
                {
                    spdlog::error(
                        "Unexpected error when sending relay packet " +
                        relayInformation
                    );
                    break;
                }
            }
        }
    }
}
