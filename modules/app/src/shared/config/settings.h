#pragma once

#include <SFML/Network/Socket.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>

namespace Settings
{
    namespace App
    {
        static std::string appName = "Networking Project";
    }

    namespace Window
    {
        static sf::Vector2u windowSize = {800, 800};
        static sf::Vector2u serverWindowSize = {1280, 960};
        static sf::Vector2u windowOrigin = {0, 0};
        static sf::Color clearColour = sf::Color::Black;
    } // namespace Window

    namespace Camera
    {
        static sf::Vector2f playerViewSize = {500, 500};
        static sf::Vector2f serverViewSize = {2560, 1920};
    } // namespace Camera

    namespace Actor
    {
        static sf::Vector2 shipScaling = {4.f, 4.f};
        static float shipTurningRadius = 10.f;
        static sf::Vector2f projectileScaling = {4.f, 4.f};
        static sf::Color connectedMask = sf::Color(255, 255, 255, 255);
        static sf::Color disconnectedMask = sf::Color(255, 255, 255, 100);
    } // namespace Actor

    namespace Networking
    {
        static unsigned short serverPort = 6969;
        static unsigned short playerPort = sf::Socket::AnyPort;
        static float clientUpdateInterval = 0.1f;
        static float serverPingCheckInterval = 1.f;
        // Using TCP so only ship history of 2 needed
        static unsigned short maxShipMessageHistory = 2;
        static unsigned short maxPingCheckMessageHistory = 3;
        static unsigned short maxClientConnectToServerTries = 5;
        static unsigned short maxClientReconnectToServerTries = 10;
        static float clientConnectToServerWaitTime = 0.2f;
        static float clientReconnectToServerWaitTime = 1.f;
        static float maximumAllowedClockDrift = .1f;
        static float interpolationDelay = 0.1f;
        static float correctionLerp = 0.15f;
    } // namespace Networking
    namespace Game
    {
        static float shipSpeed = 150.f;
        static float shipFirerate = 0.2f;
        static float projectileDamage = 25.f;
        static float projectileSpeed = 500.f;
        static float projectileLifetime = 1.f;
        static unsigned int respawnGridStep = 10;
        static float minRespawnDistance = 50.f;
        static float respawnTime = 1.f;
        static unsigned int shipHealth = 100;
    } // namespace Game
    namespace ShipInfo
    {
        static sf::Color fontColour = sf::Color::White;
        static unsigned int fontSize = 12;
        static int textYOffset = 60;
        static int healthBarWidth = 75;
        static int healthBarYOffset = 25;
        static int healthBarHeight = 5;
    } // namespace ShipInfo

}; // namespace Settings
