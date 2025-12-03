#pragma once

#include <SFML/Network/Packet.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>

enum MessageType : int32_t
{
    Ship = 0,
    PlayerConnect = 1
};

struct Message
{
    int32_t type;
    int32_t time = 0;
    int32_t id = 0;

    virtual ~Message() = default;

    // Copilot suggestion for abstract serialisation
    void baseSerialise(sf::Packet &packet) const;
    virtual void serialise(sf::Packet &packet) const = 0;

    static std::vector<std::unique_ptr<Message>>
    deserialiseMessagesFromPacket(sf::Packet &packet);

    static std::unique_ptr<Message>
    deserialiseMessageFromPacket(sf::Packet &packet);

    private:
    void baseDeserialise(sf::Packet &packet);
    virtual void deserialise(sf::Packet &packet) = 0;
};

struct ShipMessage : public Message
{
    ShipMessage();
    ~ShipMessage() = default;

    sf::Vector2f position = {0.f, 0.f};
    sf::Vector2f forwardVector = {0.f, -1.f};

    void serialise(sf::Packet &packet) const override;

    private:
    void deserialise(sf::Packet &packet) override;
};

struct PlayerConnectMessage : public Message
{
    PlayerConnectMessage();
    ~PlayerConnectMessage() = default;

    void serialise(sf::Packet &packet) const override;

    private:
    void deserialise(sf::Packet &packet) override;
};

sf::Packet &operator<<(sf::Packet &packet, const Message &message);
sf::Packet &operator>>(sf::Packet &packet, Message &message);
