#pragma once

#include <SFML/Network/Packet.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>

enum MessageType : int32_t
{
    Ship,
    PlayerConnect,
    Shoot,
    PingCheck,
    Hit,
    END
};

struct Message
{
    int32_t type;
    float_t time = 0.f;
    std::string handle;
    float_t ping = 0.f;

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
    int32_t turnDirection = 0;

    void serialise(sf::Packet &packet) const override;

    private:
    void deserialise(sf::Packet &packet) override;
};

struct PlayerConnectMessage : public Message
{
    PlayerConnectMessage();
    ~PlayerConnectMessage() = default;
    
    int32_t skinIndex = 0;

    void serialise(sf::Packet &packet) const override;

    private:
    void deserialise(sf::Packet &packet) override;
};

struct ShootMessage : public Message
{
    ShootMessage();
    ~ShootMessage() = default;

    sf::Vector2f position = {0.f, 0.f};
    sf::Vector2f forwardVector = {0.f, -1.f};

    void serialise(sf::Packet &packet) const override;

    private:
    void deserialise(sf::Packet &packet) override;
};

struct PingCheckMessage : public Message
{
    PingCheckMessage();
    ~PingCheckMessage() = default;

    float_t echoTime;
    float_t deltaTime = 0.f;

    void serialise(sf::Packet &packet) const override;

    private:
    void deserialise(sf::Packet &packet) override;
};

struct HitMessage : public Message
{
    HitMessage();
    ~HitMessage() = default;

    std::string hitShipHandle;
    int32_t newHealth;

    void serialise(sf::Packet &packet) const override;

    private:
    void deserialise(sf::Packet &packet) override;
};

sf::Packet &operator<<(sf::Packet &packet, const Message &message);
sf::Packet &operator>>(sf::Packet &packet, Message &message);
