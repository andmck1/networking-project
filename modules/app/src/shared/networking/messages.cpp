#include "messages.h"
#include <memory>
#include <spdlog/spdlog.h>

sf::Packet &operator<<(sf::Packet &packet, const Message &message)
{
    message.baseSerialise(packet);
    message.serialise(packet);
    return packet;
}

void Message::baseSerialise(sf::Packet &packet) const
{
    packet << this->type << this->time << this->id;
}

void Message::baseDeserialise(sf::Packet &packet)
{
    // Assume you get type before this
    packet >> this->time >> this->id;
}

std::unique_ptr<Message>
Message::deserialiseMessageFromPacket(sf::Packet &packet)
{
    int32_t messageType;
    packet >> messageType;

    std::unique_ptr<Message> message;
    switch (messageType)
    {
        case (MessageType::Ship):
        {
            message = std::make_unique<ShipMessage>();
            break;
        }
        case (MessageType::PlayerConnect):
        {
            message = std::make_unique<PlayerConnectMessage>();
            break;
        }
        case (MessageType::Shoot):
        {
            message = std::make_unique<ShootMessage>();
            break;
        }
        case (MessageType::PingCheck):
        {
            message = std::make_unique<PingCheckMessage>();
            break;
        }
        case (MessageType::Hit):
        {
            message = std::make_unique<HitMessage>();
            break;
        }
        default:
        {
            spdlog::error("Unhandled message type, will error");
        }
    }
    message->baseDeserialise(packet);
    message->deserialise(packet);
    return message;
}

std::vector<std::unique_ptr<Message>>
Message::deserialiseMessagesFromPacket(sf::Packet &packet)
{
    std::vector<std::unique_ptr<Message>> messages;
    int32_t nMessages;
    packet >> nMessages;

    spdlog::debug(std::to_string(nMessages) + " messages received");

    for (int messageIndex = 0; messageIndex < nMessages; messageIndex++)
    {
        std::unique_ptr<Message> message =
            Message::deserialiseMessageFromPacket(packet);
        messages.push_back(std::move(message));
    }

    return messages;
}

ShipMessage::ShipMessage()
{
    this->type = MessageType::Ship;
}

void ShipMessage::serialise(sf::Packet &packet) const
{
    packet << this->position.x << this->position.y << this->forwardVector.x
           << this->forwardVector.y << this->turnDirection;
}

void ShipMessage::deserialise(sf::Packet &packet)
{
    packet >> this->position.x >> this->position.y >> this->forwardVector.x >>
        this->forwardVector.y >> this->turnDirection;
}

PlayerConnectMessage::PlayerConnectMessage()
{
    this->type = MessageType::PlayerConnect;
}

void PlayerConnectMessage::serialise(sf::Packet &packet) const
{
}

void PlayerConnectMessage::deserialise(sf::Packet &packet)
{
}

ShootMessage::ShootMessage()
{
    this->type = MessageType::Shoot;
}

void ShootMessage::serialise(sf::Packet &packet) const
{
    packet << this->position.x << this->position.y << this->forwardVector.x
           << this->forwardVector.y;
}

void ShootMessage::deserialise(sf::Packet &packet)
{
    packet >> this->position.x >> this->position.y >> this->forwardVector.x >>
        this->forwardVector.y;
}

PingCheckMessage::PingCheckMessage()
{
    this->type = MessageType::PingCheck;
}

void PingCheckMessage::serialise(sf::Packet &packet) const
{
    packet << this->echoTime << this->ping << this->deltaTime;
}

void PingCheckMessage::deserialise(sf::Packet &packet)
{
    packet >> this->echoTime >> this->ping >> this->deltaTime;
}

HitMessage::HitMessage()
{
    this->type = MessageType::Hit;
}

void HitMessage::serialise(sf::Packet &packet) const
{
    packet << this->hitShipId;
}

void HitMessage::deserialise(sf::Packet &packet)
{
    packet >> this->hitShipId;
}
