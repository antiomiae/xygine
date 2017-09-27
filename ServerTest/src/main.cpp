/*********************************************************************
(c) Matt Marchant 2017
http://trederia.blogspot.com

xygineXT - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <xyginext/network/NetHost.hpp>
#include <xyginext/network/NetData.hpp>

#include <SFML/Config.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Sleep.hpp>

#include <iostream>
#include <string>

struct TestStruct final
{
    sf::Uint8 b = 0;
    sf::Uint32 i = 0;
    float f = 0.f;
    char w[6];
};

void threadFunc(const bool* running)
{
    xy::NetHost host;
    host.start("", 40003, 2, 2);

    xy::NetEvent evt;
    while (*running)
    {
        while (host.pollEvent(evt))
        {
            switch (evt.type)
            {
            default: break;
            case xy::NetEvent::ClientConnect:
                std::cout << "Client Connected" << std::endl;
                break;
            case xy::NetEvent::ClientDisconnect:
                std::cout << "Client Disconnected" << std::endl;
                break;
            case xy::NetEvent::PacketReceived:
                std::cout << "Packet received" << std::endl;
                {
                    /*auto packet = evt.packet.as<TestStruct>();
                    std::cout << "ID:" << evt.packet.ID() << ", B: " << packet.b << ", I: " << packet.i << ", F: " << packet.f << std::endl;
                    std::cout << packet.w << std::endl;

                    packet.w[0] = 'f';
                    packet.w[1] = 'l';
                    packet.w[2] = 'a';
                    packet.w[3] = 'p';
                    packet.w[4] = 's';
                    host.broadcastPacket(5, packet, xy::NetFlag::Reliable);*/
                    host.sendPacket(evt.peer, 1, TestStruct(), xy::NetFlag::Unreliable);
                }
                break;
            }
        }
    }
    std::cout << "Quitting..." << std::endl;
    host.stop();
}

int main()
{
    bool running = true;

    sf::Thread thread(&threadFunc, &running);
    thread.launch();
    sf::sleep(sf::milliseconds(100));
    
    std::cout << "Press Q to quit" << std::endl;

    while (running)
    {
        char buns;
        std::cin >> buns;
        if (buns == 'q' || buns == 'Q')
        {
            running = false;
        }
    }
    thread.wait();

    return 0;
}