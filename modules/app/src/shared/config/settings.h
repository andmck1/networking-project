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
        static sf::Vector2u windowSize = {1600, 1280};
        static sf::Vector2u windowOrigin = {0, 0};
        static sf::Color clearColour = sf::Color::Black;
    } // namespace Window

    namespace Camera
    {
        static sf::Vector2f viewSize = {400, 320};
    }

    namespace Actor
    {
        static float shipSpeed = 200.f;
        static sf::Vector2 shipScaling = {1.f, 1.f};
        static float shipTurningRadius = 10.f;
        static float projectileSpeed = 500.f;
        static sf::Vector2f projectileScaling = {1.f, 1.f};
        static float projectileLifetime = .5f;
    } // namespace Actor

    namespace Networking
    {
        static unsigned short serverPort = 6969;
        static unsigned short playerPort = sf::Socket::AnyPort;
    } // namespace Networking

}; // namespace Settings
