#include "server.h"

#include <spdlog/spdlog.h>

ClientConnection::ClientConnection(
    int id, std::unique_ptr<sf::TcpSocket> socket
)
{
    this->id = id;
    this->socket = std::move(socket);
}

ServerConfig::ServerConfig(unsigned short port)
{
    this->port = port;
}

void Server::init(ServerConfig *config)
{
    this->port = config->port;
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
            spdlog::debug("New connection from " + connectionDetailsString);

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
    // TODO - update
    message.time = 0.f;
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

    if (receivedMessagesFromAllClients.size() > 0)
        spdlog::debug(
            "Game time received: " +
            std::to_string(receivedMessagesFromAllClients.at(0)->time)
        );
    return receivedMessagesFromAllClients;
}
