#include "clientconnection.h"

#include <spdlog/spdlog.h>

void ClientConnectionState::nextState()
{
    switch (this->connectionState)
    {
        case (ClientConnectionState::ConnectionState::Ready):
        {
            this->connectionState =
                ClientConnectionState::ConnectionState::NotReady;
            break;
        }
        case (ClientConnectionState::ConnectionState::Syncing):
        {
            this->connectionState =
                ClientConnectionState::ConnectionState::Syncing;
            break;
        }
        case (ClientConnectionState::ConnectionState::NotReady):
        {
            this->connectionState =
                ClientConnectionState::ConnectionState::Ready;
            break;
        }
        case (ClientConnectionState::ConnectionState::Disconnected):
        {
            break;
        }
        default:
        {
            spdlog::warn("Connection state path not defined, disconnecting");
            this->disconnected();
            break;
        }
    }
}

void ClientConnectionState::disconnected()
{
    this->connectionState =
        ClientConnectionState::ConnectionState::Disconnected;
}

void ClientConnectionState::syncing()
{
    this->connectionState = ClientConnectionState::ConnectionState::Syncing;
}

void ClientConnectionState::finishedSyncing()
{
    this->connectionState = ClientConnectionState::ConnectionState::NotReady;
}

bool ClientConnectionState::isSyncing()
{
    if (this->connectionState ==
        ClientConnectionState::ConnectionState::Syncing)
        return true;
    return false;
}

bool ClientConnectionState::isReady()
{
    if (this->connectionState == ClientConnectionState::ConnectionState::Ready)
        return true;
    return false;
}

bool ClientConnectionState::isConnectedAndNotSyncing()
{
    if (!this->isDisconnected() && !this->isSyncing())
        return true;
    return false;
}

bool ClientConnectionState::isReceiving()
{
    if (this->isSyncing() || this->isReady())
        return true;
    return false;
}

bool ClientConnectionState::isDisconnected()
{
    if (this->connectionState ==
        ClientConnectionState::ConnectionState::Disconnected)
        return true;
    return false;
}

ClientConnection::ClientConnection(
    int id, std::unique_ptr<sf::TcpSocket> socket
)
{
    this->id = id;
    this->socket = std::move(socket);
    this->ipAddress = this->socket->getRemoteAddress().value().toString();
    this->port = this->socket->getRemotePort();
    this->connectionDetails = std::format(
        "{}:{}", this->socket->getRemoteAddress()->toString(),
        std::to_string(this->socket->getRemotePort())
    );
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
        // TODO - Could calculate using ping and deltaTime as well
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
