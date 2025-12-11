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
    std::string handle, std::unique_ptr<sf::TcpSocket> socket
)
{
    this->handle= handle;
    this->socket = std::move(socket);
    this->ipAddress = this->socket->getRemoteAddress().value().toString();
    this->port = this->socket->getRemotePort();
    this->connectionDetails = std::format(
        "{}:{}", this->socket->getRemoteAddress()->toString(),
        std::to_string(this->socket->getRemotePort())
    );
}

sf::Vector2f ClientConnection::interpolateVectorLinear(
    sf::Vector2f vector0, sf::Vector2f vector1, float interpolationFactor
)
{
    return ((vector1 - vector0) * interpolationFactor) + vector0;
}

sf::Vector2f ClientConnection::predictVectorWithLinearVelocity(
    sf::Vector2f currentVector, sf::Vector2f previousVector,
    float currentVectorTime, float previousVectorTime, float predictionTime
)
{
    sf::Vector2f predictedVector;
    sf::Vector2f deltaVector = currentVector - previousVector;
    float deltaTime = currentVectorTime - previousVectorTime;
    sf::Vector2f velocity = deltaVector / deltaTime;
    float forwardTimeStep = predictionTime - currentVectorTime;

    predictedVector = velocity * forwardTimeStep + currentVector;
    return predictedVector;
}

ShipMessage ClientConnection::predictShipMessage(
    float totalTime, bool isInterpolationEnabled, bool isPredictionEnabled
)
{
    ShipMessage newShipMessage;
    const ShipMessage newestMessage = this->shipMessageHistory[0];
    float interpolationDelay = 0.1f;
    float correctionLerp = 0.15f;

    float predictionTime = this->estimatedCurrentGameTime - interpolationDelay;
    sf::Vector2f predictedPosition = newestMessage.position;
    sf::Vector2f predictedForwardVector = newestMessage.forwardVector;

    // TODO - what happens if previous message very old?
    // Using TCP so guaranteed to have the messages in order and longer
    // history not useful unless averaging out or using non-linear stuff
    // TODO - (non-linear could be useful if previously turning)
    if (this->shipMessageHistory.size() > 1)
    {
        const ShipMessage previousMessage = shipMessageHistory[1];
        float messageTimeDelta = newestMessage.time - previousMessage.time;
        float interpolationStep;

        // if prediction time is between the previous two messages
        // scale interpolation step with estimated game time and message delta
        // https://www.reddit.com/r/gamedev/comments/mnfxze/networkingclientside_interpolation_how_do_compute/
        // otherwise if prediction time ahead of the newest message we need
        // to predict
        if ((predictionTime <= newestMessage.time) &&
            (predictionTime > previousMessage.time) && isInterpolationEnabled)
        {
            interpolationStep =
                (predictionTime - previousMessage.time) / messageTimeDelta;
            interpolationStep = std::clamp(interpolationStep, 0.f, 1.f);
            predictedPosition = this->interpolateVectorLinear(
                previousMessage.position, newestMessage.position,
                interpolationStep
            );
            predictedForwardVector = this->interpolateVectorLinear(
                previousMessage.forwardVector, newestMessage.forwardVector,
                interpolationTime
            );
        }
        else if ((predictionTime > newestMessage.time) && isPredictionEnabled)
        {
            predictedPosition = this->predictVectorWithLinearVelocity(
                newestMessage.position, previousMessage.position,
                newestMessage.time, previousMessage.time, predictionTime
            );
        }
    }

    // Smooth from new predicted position and previous position
    // by lerping using a fixed factor, vv important for smooth motion
    sf::Vector2f smoothedPosition = predictedPosition;
    sf::Vector2f smoothedForwardVector = predictedForwardVector;
    if (this->previousPredictedShipPosition == sf::Vector2f{})
    {
        this->previousPredictedShipPosition = predictedPosition;
        this->previousPredictedShipForwardVector = predictedForwardVector;
    }
    else
    {
        smoothedPosition = this->interpolateVectorLinear(
            this->previousPredictedShipPosition, predictedPosition,
            correctionLerp
        );

        smoothedForwardVector = this->interpolateVectorLinear(
            this->previousPredictedShipForwardVector, predictedForwardVector,
            correctionLerp
        );
    }

    this->previousPredictedShipPosition = smoothedPosition;
    this->previousPredictedShipForwardVector = smoothedForwardVector;

    newShipMessage.handle = newestMessage.handle;
    newShipMessage.time = this->estimatedCurrentGameTime;
    newShipMessage.position = smoothedPosition;
    newShipMessage.forwardVector = smoothedForwardVector;
    newShipMessage.turnDirection = newestMessage.turnDirection;
    newShipMessage.skinIndex = newestMessage.skinIndex;

    return newShipMessage;
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
