/*********************************************************************
Matt Marchant 2014 - 2015
http://trederia.blogspot.com

xygine - Zlib license.

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

//message bus to allow inter-component communication

#ifndef MESSAGE_BUS_HPP_
#define MESSAGE_BUS_HPP_

#include <xygine/State.hpp>
#include <xygine/Difficulty.hpp>

#include <SFML/Config.hpp>

#include <vector>
#include <cassert>
#include <type_traits>

namespace xy
{
    class Component;
    class Entity;

    class Message final
    {
        friend class MessageBus;
    public:
        using Id = sf::Int32;
        enum Type
        {
            AudioMessage = 0,
            PhysicsMessage,
            EntityMessage,
            UIMessage,
            PlayerMessage,
            ComponentSystemMessage,
            NetworkMessage,
            Count
        };

        struct AudioEvent
        {
            sf::Uint64 entityId = 0;
        };

        struct PhysicsEvent
        {
            enum
            {
                Collision
            }event;

            sf::Uint64 entityId[2];
        };

        struct EntityEvent
        {
            enum
            {
                Destroyed,
                ChangedDirection
            }action;
            Entity* entity = nullptr;
            sf::Int32 direction = 0;
        };

        struct UIEvent
        {
            enum
            {
                RequestVolumeChange,
                RequestAudioMute,
                RequestAudioUnmute,
                MenuOpened,
                MenuClosed,
                RequestState,
                RequestDifficultyChange,
                RequestControllerEnable,
                RequestControllerDisable,
                ResizedWindow
            }type;
            float value = 0.f;
            StateId stateId = -1;
            Difficulty difficulty;
        };

        struct PlayerEvent
        {
            enum
            {
                Died,
                Spawned
            }action;
            sf::Int32 timestamp = 0;
            sf::Uint64 entityID = 0;
        };

        struct ComponentEvent
        {
            enum
            {
                Deleted
            }action;
            sf::Uint64 entityId = 0;
            Component* ptr = nullptr;
        };

        struct NetworkEvent
        {
            enum
            {
                RequestStartServer,
                RequestJoinServer,
                RequestDisconnect,
                RequestWorldState,
                ConnectSuccess,
                ConnectFail,
                StartReady
            }action;
            StateId stateID = -1;
        };

        Id id = -1;

        template <typename T, typename std::enable_if<std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value>::type...>
	//template <typename T>
        const T& getData() const
        {
            //for some reason this isn't working on MSVC
            //static_assert(std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value, "");
            assert(sizeof(T) == m_dataSize);
            return *static_cast<T*>(m_data);
        }

    private:
        void* m_data;
        std::size_t m_dataSize;
    };

    class MessageBus final
    {
    public:
        MessageBus();
        ~MessageBus() = default;
        MessageBus(const MessageBus&) = delete;
        const MessageBus& operator = (const MessageBus&) = delete;

        //read and despatch all messages on the message stack
        const Message& poll();
        //places a message on the message stack, and returns a pointer to the data
        //of type T, which needs to be filled in
        template <typename T, typename std::enable_if<std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value>::type...>
	//template <typename T>
        T* post(Message::Id id)
        {
            auto dataSize = sizeof(T);
            static auto msgSize = sizeof(Message);
            assert(dataSize < 128); //limit custom data to 128 bytes
            assert(m_pendingBuffer.size() - (m_inPointer - m_pendingBuffer.data()) > (dataSize + msgSize)); //make sure we have enough room in the buffer

            Message* msg = new (m_inPointer)Message();
            m_inPointer += msgSize;
            msg->id = id;
            msg->m_dataSize = dataSize;
            msg->m_data = new (m_inPointer)T();
            m_inPointer += dataSize;

            m_pendingCount++;
            return static_cast<T*>(msg->m_data);
        }

        bool empty();

        std::size_t pendingMessageCount() const; //used for stats

    private:

        std::vector<char> m_currentBuffer;
        std::vector<char> m_pendingBuffer;
        char* m_inPointer;
        char* m_outPointer;
        std::size_t m_currentCount;
        std::size_t m_pendingCount;
    };
}
#endif
