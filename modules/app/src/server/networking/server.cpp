#include "server.h"

#include "../../shared/config/settings.h"
#include <spdlog/spdlog.h>

NonBlockingTCPPacketSendStatus
NonBlockingTCPRelayPacket::statusHandler(sf::Socket::Status status)
{
    switch (status)
    {
        case (sf::Socket::Status::Done):
        {
            spdlog::debug(
                "{} messages relayed", std::to_string(this->nNetworkUpdates)
            );
            this->sendStatus = NonBlockingTCPPacketSendStatus::Done;
            break;
        }
        case (sf::Socket::Status::NotReady):
        {
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
            spdlog::info("Client disconnected when sending relay packet");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Error;
            break;
        }
        default:
        {
            spdlog::error("Unexpected error when sending relay packet");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Error;
            break;
        }
    }
    return this->sendStatus;
}

NonBlockingTCPPacketSendStatus
NonBlockingTCPPingCheckPacket::statusHandler(sf::Socket::Status status)
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
            spdlog::info("Client disconnected when sending ping check");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Error;
            break;
        }
        default:
        {
            spdlog::error("Unexpected error when sending ping check to");
            this->sendStatus = NonBlockingTCPPacketSendStatus::Error;
            break;
        }
    }
    return this->sendStatus;
}

ServerConfig::ServerConfig(unsigned short port, AppLayer *appLayer)
{
    this->port = port;
    this->appLayer = appLayer;
}

void Server::init(ServerConfig *config)
{
    this->port = config->port;
    this->appLayer = config->appLayer;
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
    std::string handle;

    spdlog::debug("Receiving from tcp socket");
    sf::Socket::Status status = tcpSocket->receive(packet);

    // Should always expect a player connect message from ready connecting
    // client, if we don't they can't connect. When received get the player
    // handle.
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
            handle = message->handle;
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

    // Connection details, remove socket from selector as moving to client
    // connection as unique ptr
    std::string ipAddressString = tcpSocket->getRemoteAddress()->toString();
    unsigned int port = tcpSocket->getRemotePort();
    this->selector.remove(*tcpSocket);

    // If the client handle exists in the list and is disconnected then replace
    // the socket associated with that client. Otherwise, disconnect client TODO
    // - should send disconnection reason message. If client doesn't exist that
    // add new entry If client is connected, set to syncing state.
    if (this->clientConnectionsMap.find(handle) !=
        this->clientConnectionsMap.end())
    {
        ClientConnection *clientConnection = this->clientConnectionsMap[handle];
        if (clientConnection->connectionState.isDisconnected())
        {
            spdlog::info(
                "Client {} reconnected at {}:{}", handle, ipAddressString, port
            );
            this->selector.remove(*clientConnection->socket);
            clientConnection->socket = std::move(tcpSocket);
            this->selector.add(*clientConnection->socket);
            clientConnection->connectionState.syncing();
            clientConnection->ipAddress = ipAddressString;
            clientConnection->port = port;
            clientConnection->socket->setBlocking(false);
            clientConnection->pingHistory.clear();
            clientConnection->shipMessageHistory.clear();
        }
        else
        {
            spdlog::info(
                "Client at {}:{} tried to connect with existing handle {}, "
                "disconnecting",
                ipAddressString, port, handle
            );
            tcpSocket->disconnect();
        }
    }
    else
    {
        spdlog::info(
            "New client {} at {}:{} connected", handle, ipAddressString, port
        );
        std::unique_ptr<ClientConnection> newClient =
            std::make_unique<ClientConnection>(handle, std::move(tcpSocket));

        newClient->socket->setBlocking(false);
        this->selector.add(*newClient->socket);

        this->clientConnections.push_back(std::move(newClient));
        this->clientConnectionsMap.insert(
            {handle, this->clientConnections.back().get()}
        );
        this->clientConnections.back()->connectionState.syncing();
    }
}

void Server::acceptPlayerConnections()
{
    // If new client connecting, add to connecting client sockets and add to
    // selector
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
        // For all connecting clients, if they're ready then remove from
        // connecting client list and create new client
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

        // For all connected clients, if they're ready then progress their state
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
    // For all clients, if they're syncing then send ping check, if finished
    // syncing then update state and update players with score updates
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
                this->sendScoreUpdateMessages(true);
                spdlog::info("Client synced");
            }
            else if (!clientConnection->syncRequestSent)
            {
                spdlog::info("Client syncing");
                spdlog::info(
                    std::format(
                        "Sending ping check messages to {}",
                        clientConnection->handle
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
                        clientConnection->handle
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

    // For all client updates
    // If ship update then update ship message queue for interpolation and
    // prediction
    // If shoot message then pass to processed messages
    // If ping check
    // then must be response from client. Update network stats and do not push
    // forward.
    for (std::unique_ptr<Message> &message : clientUpdates)
    {
        const std::string clientHandle = message->handle;
        ClientConnection *clientConnection =
            this->clientConnectionsMap[clientHandle];
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
                    (this->appLayer->totalTime.asSeconds() -
                     pingCheckMessage->echoTime) /
                    2.f
                );
                if (clientConnection->pingHistory.size() >
                    Settings::Networking::maxPingCheckMessageHistory)
                {
                    clientConnection->pingHistory.pop_back();
                }
                clientConnection->updateAveragePing();
                clientConnection->lastRecordedGameTime = pingCheckMessage->time;
                clientConnection->lastRecordedDeltaTime =
                    pingCheckMessage->deltaTime;
                clientConnection->estimatedCurrentGameTime =
                    pingCheckMessage->time + clientConnection->averagePing;
                clientConnection->lastRecordedClockDrift =
                    this->appLayer->totalTime.asSeconds() -
                    clientConnection->estimatedCurrentGameTime;
                break;
            }
            default:
            {
                spdlog::warn("Unsupported message type sent");
                break;
            }
        }
    }

    // Get a predicted ship message from ship message history and add to
    // processed messages, setting connection status to enabled status if ship
    // is fully connected, otherwise connection status is false
    for (const std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        if (!clientConnection->shipMessageHistory.empty())
        {
            ShipMessage shipMessage = clientConnection->predictShipMessage(
                this->appLayer->totalTime.asSeconds(),
                this->isInterpolationEnabled, this->isPredictionEnabled
            );
            if (this->appLayer->shipActorMap.find(clientConnection->handle) !=
                this->appLayer->shipActorMap.end())
            {
                ShipActor *ship =
                    this->appLayer->shipActorMap[clientConnection->handle];
                shipMessage.isConnected = ship->isEnabled;
            }
            else
                shipMessage.isConnected = false;
            processedMessages.push_back(
                std::make_unique<ShipMessage>(shipMessage)
            );
        }
    }

    return processedMessages;
}

void Server::sendRelayNetworkUpdates(
    const std::vector<std::unique_ptr<Message>> &networkUpdates
)
{
    // Package up network updates per client into a single packet, including the
    // number of network messages in packet 
    //
    // for all clients, create packet, 
    // for all network updates, add to packet if client didn't send update send
    // network update to client
    for (std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        // TODO - use iterator with filter instead
        if (clientConnection->connectionState.isDisconnected())
            continue;
        NonBlockingTCPRelayPacket relayPacket;
        sf::Packet packetMessages;
        int32_t nNetworkUpdates = 0;
        for (const std::unique_ptr<Message> &networkUpdate : networkUpdates)
        {
            if ((networkUpdate->handle != clientConnection->handle) ||
                (networkUpdate->type == MessageType::Ship))
            {
                packetMessages << *networkUpdate;
                nNetworkUpdates++;
            }
        }
        if (nNetworkUpdates > 0)
        {
            relayPacket.packet << nNetworkUpdates;
            relayPacket.nNetworkUpdates = nNetworkUpdates;
            relayPacket.packet.append(
                packetMessages.getData(), packetMessages.getDataSize()
            );
            clientConnection->nonBlockingSendPackets.push_back(
                std::make_unique<NonBlockingTCPRelayPacket>(relayPacket)
            );
        }
    }
}

void Server::sendPingCheckMessage(ClientConnection *clientConnection)
{
    NonBlockingTCPPingCheckPacket pingCheckPacket;
    PingCheckMessage message = PingCheckMessage();
    message.handle = clientConnection->handle;
    message.time = this->appLayer->totalTime.asSeconds();
    message.ping = clientConnection->averagePing;
    message.deltaTime = this->appLayer->deltaTime.asSeconds();
    pingCheckPacket.packet << (int32_t) 1;
    pingCheckPacket.packet << message;
    clientConnection->nonBlockingSendPackets.push_back(
        std::make_unique<NonBlockingTCPPingCheckPacket>(pingCheckPacket)
    );
}

void Server::sendPlayerPingCheckMessages()
{
    // If at interval, update ping data for all connected clients
    this->pingCheckTimer += this->appLayer->deltaTime.asSeconds();
    if (this->pingCheckTimer > Settings::Networking::serverPingCheckInterval)
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

void Server::sendScoreUpdateMessage(
    const ShipActor *ship, ClientConnection *clientConnection
)
{
    NonBlockingTCPPingCheckPacket pingCheckPacket;
    ScoreUpdateMessage message = ScoreUpdateMessage();
    message.handle = clientConnection->handle;
    message.time = this->appLayer->totalTime.asSeconds();
    message.ping = clientConnection->averagePing;
    message.updateShipHandle = ship->handle;
    message.kills = ship->kills;
    message.deaths = ship->deaths;
    pingCheckPacket.packet << (int32_t) 1;
    pingCheckPacket.packet << message;
    clientConnection->nonBlockingSendPackets.push_back(
        std::make_unique<NonBlockingTCPPingCheckPacket>(pingCheckPacket)
    );
}

void Server::sendScoreUpdateMessages(bool isForceEnabled)
{
    // For each ship send stat updates if there has been a change
    for (const ShipActor *ship : this->appLayer->ships)
    {
        // Only send data if stats updated
        if (!ship->statsUpdated && !isForceEnabled)
            continue;

        if (this->clientConnectionsMap.find(ship->handle) ==
            this->clientConnectionsMap.end())
            continue;
        ClientConnection *shipClient = this->clientConnectionsMap[ship->handle];

        // Don't send data about disconnected ship
        if (shipClient->connectionState.isDisconnected())
            continue;

        for (const ShipActor *otherShip : this->appLayer->ships)
        {
            if (this->clientConnectionsMap.find(otherShip->handle) ==
                this->clientConnectionsMap.end())
                continue;

            // Don't send data to disconnected client
            ClientConnection *otherShipClient =
                this->clientConnectionsMap[otherShip->handle];
            if (otherShipClient->connectionState.isDisconnected())
                continue;
            this->sendScoreUpdateMessage(ship, otherShipClient);
        }
    }
}

void Server::sendNonBlockingPackets()
{
    // Run send on all packets in non-blocking TCP
    for (std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        for (auto iterator = clientConnection->nonBlockingSendPackets.begin();
             iterator != clientConnection->nonBlockingSendPackets.end();)
        {
            std::unique_ptr<NonBlockingTCPPacket> &packet = *iterator;
            sf::Socket::Status status =
                clientConnection->socket->send(packet->packet);
            NonBlockingTCPPacketSendStatus sendStatus =
                packet->statusHandler(status);
            switch (sendStatus)
            {
                case (NonBlockingTCPPacketSendStatus::Done):
                {
                    iterator = clientConnection->nonBlockingSendPackets.erase(
                        iterator
                    );
                    break;
                }
                case (NonBlockingTCPPacketSendStatus::Sending):
                {
                    iterator++;
                    break;
                }
                case (NonBlockingTCPPacketSendStatus::Error):
                {
                    iterator = clientConnection->nonBlockingSendPackets.erase(
                        iterator
                    );
                    clientConnection->connectionState.disconnected();
                    break;
                }
                default:
                {
                    spdlog::warn("Non blocking send status not handled");
                }
            }
        }
    }
}

void Server::frameClientConnections()
{
    // Update estimated client connection game time
    for (const std::unique_ptr<ClientConnection> &clientConnection :
         this->clientConnections)
    {
        clientConnection->estimatedCurrentGameTime +=
            this->appLayer->deltaTime.asSeconds();
    }
}

std::vector<std::unique_ptr<Message>> Server::prepareGameUpdates()
{
    // Currently only need to process hit events here, creating hit events
    // for broadcast to all ship
    std::vector<std::unique_ptr<Message>> gameUpdates = {};
    while (!this->appLayer->hitEvents.empty())
    {
        std::unique_ptr<HitMessage> &hitEvent = appLayer->hitEvents.back();
        for (const std::unique_ptr<ClientConnection> &clientConnection :
             this->clientConnections)
        {
            HitMessage gameUpdate = HitMessage();
            gameUpdate.handle = clientConnection->handle;
            gameUpdate.time = hitEvent->time;
            gameUpdate.hitShipHandle = hitEvent->hitShipHandle;
            gameUpdate.ping = clientConnection->averagePing;
            gameUpdate.newHealth = hitEvent->newHealth;
            gameUpdates.push_back(std::make_unique<HitMessage>(gameUpdate));
        }
        appLayer->hitEvents.pop_back();
    }
    return gameUpdates;
}

void Server::updateShipStatesBasedOnConnectionStatus()
{
    // For all clients
    // Get ship if available
    // If disconnected or clock drift too high then disable ship
    // If it's finished (re)spawning then remove disabled state
    for (const std::unique_ptr<ClientConnection> &client :
         this->clientConnections)
    {
        std::string clientHandle = client->handle;
        if (this->appLayer->shipActorMap.find(clientHandle) !=
            this->appLayer->shipActorMap.end())
        {
            ShipActor *ship = this->appLayer->shipActorMap[clientHandle];
            if (client->connectionState.isDisconnected())
            {
                ship->isEnabled = false;
            }
            else if (std::abs(client->lastRecordedClockDrift) >
                     Settings::Networking::maximumAllowedClockDrift)
            {
                ship->isEnabled = false;
            }
            else if (!ship->isSpawning)
            {
                ship->isEnabled = true;
            }
        }
    }
}
