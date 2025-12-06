#include "server.h"

#include "../../shared/config/settings.h"
#include <spdlog/spdlog.h>

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

void Server::createNewClient(std::unique_ptr<sf::TcpSocket> tcpSocket)
{
    spdlog::info("Creating new client");

    sf::Packet packet;
    int32_t serverId;

    spdlog::debug("Receiving from tcp socket");
    sf::Socket::Status status = tcpSocket->receive(packet);
    while (status == sf::Socket::Status::Partial)
        status = tcpSocket->receive(packet);
    
    spdlog::debug("Status available");
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::info(
                "Received player connect message from {}:{}",
                tcpSocket->getRemoteAddress()->toString(),
                tcpSocket->getRemotePort()
            );
            int32_t _;
            packet >> _;
            std::unique_ptr<Message> message =
                Message::deserialiseMessageFromPacket(packet);
            serverId = message->id;
            break;
        }
        default:
        {
            spdlog::error(
                "Unexpected error reading player connect message, not "
                "connecting client."
            );
            return;
            break;
        }
    }

    std::string ipAddressString = tcpSocket->getRemoteAddress()->toString();
    unsigned int port = tcpSocket->getRemotePort();
    this->selector.remove(*tcpSocket);

    // if client exists reconnect, otherwise add new one
    if (this->clientConnectionsMap.find(serverId) !=
        this->clientConnectionsMap.end())
    {
        spdlog::info(
            "Client {} reconnected at {}:{}", serverId, ipAddressString, port
        );
        ClientConnection *clientConnection =
            this->clientConnectionsMap[serverId];
        this->selector.remove(*clientConnection->socket);
        clientConnection->socket = std::move(tcpSocket);
        this->selector.add(*clientConnection->socket);
        clientConnection->connectionState.syncing();
        clientConnection->ipAddress = ipAddressString;
        clientConnection->port = port;
        clientConnection->socket->setBlocking(false);
    }
    else
    {
        spdlog::info("New client {}:{} connected", ipAddressString, port);
        std::unique_ptr<ClientConnection> newClient =
            std::make_unique<ClientConnection>(serverId, std::move(tcpSocket));

        newClient->socket->setBlocking(false);
        this->selector.add(*newClient->socket);

        this->clientConnections.push_back(std::move(newClient));
        this->clientConnectionsMap.insert(
            {serverId, this->clientConnections.back().get()}
        );
        this->clientConnections.back()->connectionState.syncing();
    }
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
            spdlog::info(
                "Accepted new client at {}:{}",
                tcpSocket->getRemoteAddress()->toString(),
                tcpSocket->getRemotePort()
            );
            tcpSocket->setBlocking(false);
            this->connectingClientSockets.push_back(std::move(tcpSocket));
            this->selector.add(*this->connectingClientSockets.back().get());
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

void Server::scanSelector()
{
    spdlog::debug("Scanning selector");
    if (this->selector.wait(sf::milliseconds(5)))
    {
        for (auto iterator = this->connectingClientSockets.begin();
             iterator != this->connectingClientSockets.end();)
        {
            std::unique_ptr<sf::TcpSocket> &connectingSocket = *iterator;
            if (connectingSocket)
            {
                if (this->selector.isReady(*connectingSocket))
                {
                    std::unique_ptr<sf::TcpSocket> movedConnectingSocket =
                        std::move(connectingSocket);
                    iterator = this->connectingClientSockets.erase(iterator);
                    this->createNewClient(std::move(movedConnectingSocket));
                }
                else
                {
                    iterator++;
                }
            }
            else
            {
                iterator++;
            }
        }

        spdlog::debug("Scanning current selector sockets");
        for (const std::unique_ptr<ClientConnection> &clientConnection :
             this->clientConnections)
        {
            if (this->selector.isReady(*clientConnection->socket))
                clientConnection->connectionState.nextState();
        }
    }
}

void Server::checkForSyncs()
{
    for (std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        if (clientConnection->connectionState.isSyncing())
        {
            // TODO - can check if out of sync and reset and ask to sync
            // again
            if (clientConnection->pingHistory.size() >=
                Settings::Networking::maxPingCheckMessageHistory)
            {
                clientConnection->connectionState.finishedSyncing();
                clientConnection->syncRequestSent = false;
                spdlog::info("Client synced");
            }
            else if (!clientConnection->syncRequestSent)
            {
                spdlog::info("Client syncing");
                spdlog::info(
                    std::format(
                        "Sending ping check messages to {}",
                        clientConnection->id
                    )
                );
                for (int _ = 0;
                     _ < Settings::Networking::maxPingCheckMessageHistory; _++)
                    this->sendPingCheckMessage(clientConnection.get());
                clientConnection->syncRequestSent = true;
            }
        }
    }
}

std::vector<std::unique_ptr<Message>> Server::receiveDataFromClients()
{
    std::vector<std::unique_ptr<Message>> receivedMessagesFromAllClients;

    // for all clients, if ready receive packet, deserialise and append
    // messages
    for (std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        if (clientConnection->connectionState.isReceiving())
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
                    clientConnection->connectionState.nextState();
                    break;
                }
                case (sf::Socket::Status::Disconnected):
                {
                    spdlog::info(
                        "Client {} disconnected while sending packet",
                        clientConnection->id
                    );
                    clientConnection->connectionState.disconnected();
                    break;
                }
                case (sf::Socket::Status::NotReady):
                {
                    if (clientConnection->connectionState.isSyncing())
                        spdlog::debug(
                            "Not ready status while receiving messages "
                            "from client while it's syncing"
                        );
                    else
                        spdlog::warn(
                            "Client not ready to send data while receiving "
                            "messages"
                        );
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
        ClientConnection *clientConnection =
            this->clientConnectionsMap[clientId];
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
                clientConnection->deltaWithServer =
                    this->totalTime->asSeconds() -
                    clientConnection->theirGameTime;
                if (clientConnection->pingHistory.size() >
                    Settings::Networking::maxPingCheckMessageHistory)
                {
                    clientConnection->pingHistory.pop_back();
                }
                clientConnection->updateAveragePing();
                clientConnection->theirGameTime =
                    pingCheckMessage->time + clientConnection->averagePing;
                clientConnection->theirDeltaTime = pingCheckMessage->deltaTime;
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
    for (std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        // TODO - use iterator with filter instead
        if (clientConnection->connectionState.isDisconnected())
            continue;
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
                        "{} messages relayed to {}",
                        std::to_string(nNetworkUpdates), clientConnection->id
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
                    spdlog::info(
                        "Client disconnected when sending relay packet to "
                        "{}",
                        clientConnection->id
                    );
                    clientConnection->connectionState.disconnected();
                    break;
                }
                default:
                {
                    spdlog::error(
                        "Unexpected error when sending relay packet to {}",
                        clientConnection->id
                    );
                    break;
                }
            }
        }
    }
}

void Server::sendPingCheckMessage(ClientConnection *clientConnection)
{
    sf::Packet packet;
    PingCheckMessage message = PingCheckMessage();
    message.id = clientConnection->id;
    message.time = this->totalTime->asSeconds();
    message.ping = clientConnection->averagePing;
    message.deltaTime = this->deltaTime->asSeconds();
    packet << (int32_t) 1;
    packet << message;

    sf::Socket::Status status = clientConnection->socket->send(packet);
    while (status == sf::Socket::Status::Partial)
        status = clientConnection->socket->send(packet);
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug(
                "Ping check message sent to {}", clientConnection->id
            );
            break;
        }
        case (sf::Socket::Status::NotReady):
        {
            break;
        }
        case (sf::Socket::Status::Disconnected):
        {
            spdlog::info(
                "Client disconnected when sending ping check to {}",
                clientConnection->id
            );
            clientConnection->connectionState.disconnected();
            break;
        }
        default:
        {
            spdlog::error(
                "Unexpected error when sending ping check to {}",
                clientConnection->id
            );
            break;
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
            // TODO - use iterator with filter?
            if (clientConnection->connectionState.isDisconnected())
                continue;
            this->sendPingCheckMessage(clientConnection.get());
        }
    }
}

void Server::frameClientConnections()
{
    for (const std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        clientConnection->theirGameTime += this->deltaTime->asSeconds();
    }
}

std::vector<std::unique_ptr<Message>>
Server::prepareGameUpdates(AppLayer *appLayer)
{
    std::vector<std::unique_ptr<Message>> gameUpdates = {};
    // hits
    while (!appLayer->hitEvents.empty())
    {
        std::unique_ptr<HitMessage> &hitEvent = appLayer->hitEvents.back();
        for (const std::unique_ptr<ClientConnection> &clientConnection :
             this->clientConnections)
        {
            HitMessage gameUpdate = HitMessage();
            gameUpdate.id = clientConnection->id;
            gameUpdate.time = hitEvent->time;
            gameUpdate.hitShipId = hitEvent->hitShipId;
            gameUpdates.push_back(std::make_unique<HitMessage>(gameUpdate));
        }
        appLayer->hitEvents.pop_back();
    }
    return gameUpdates;
}
