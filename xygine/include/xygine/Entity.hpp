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

//an entity defined by its collection of components

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include <xygine/Log.hpp>
#include <xygine/Command.hpp>
#include <xygine/Component.hpp>

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <memory>
#include <vector>

namespace xy
{
    //class Component;
    class Scene;
    class Entity final : public sf::Transformable, public sf::Drawable
    {
    public:
        using Ptr = std::unique_ptr<Entity>;

        explicit Entity(MessageBus&);
        ~Entity() = default;
        Entity(const Entity&) = delete;
        Entity& operator = (const Entity&) = delete;

        void addChild(Ptr&);
        Ptr removeChild(Entity&);
        const std::vector<Ptr>& getChildren() const;
        Entity* findEntity(sf::Uint64);

        sf::Vector2f getWorldPosition() const;
        sf::Transform getWorldTransform() const;

        void setWorldPosition(sf::Vector2f);

        void update(float dt);

        template <typename T>
        void addComponent(std::unique_ptr<T>& component)
        {
            Component::Ptr c(static_cast<Component*>(component.release()));
            if (c->type() == Component::Type::Drawable)
            {
                //store a reference to drawables so they can be drawn
                m_drawables.push_back(dynamic_cast<sf::Drawable*>(c.get()));
            }
            c->setParentUID(m_uid);
            c->onStart(*this);
            m_pendingComponents.push_back(std::move(c));
        }

        template <typename T>
        T* getComponent(const std::string& name)
        {
            if (name.empty()) return nullptr;
            auto result = std::find_if(m_components.begin(), m_components.end(), FindByName(name));

            if (result == m_components.end())
            {
                result = std::find_if(m_pendingComponents.begin(), m_pendingComponents.end(), FindByName(name));

                if (result == m_pendingComponents.end())
                {
                    return nullptr;
                }
            }
            return dynamic_cast<T*>(result->get());
        }

        //be warned this only returns the first found instance of this type
        template <typename T>
        T* getComponent(Component::UniqueType uniqueType)
        {
            auto result = std::find_if(m_components.begin(), m_components.end(), FindByUniqueType(uniqueType));
            if (result == m_components.end())
            {
                result = std::find_if(m_pendingComponents.begin(), m_pendingComponents.end(), FindByUniqueType(uniqueType));
                if (result == m_pendingComponents.end())
                {
                    return nullptr;
                }
            }
            return dynamic_cast<T*>(result->get());
        }

        void destroy();
        bool destroyed() const;

        sf::Uint64 getUID() const;
        void setUID(sf::Uint64);

        void handleMessage(const Message&);

        void setScene(Scene*);
        Scene* getScene();

        //returns true if command was consumed and should not be passed on
        bool doCommand(const Command&, float);
        void addCommandCategories(sf::Int32);

        //returns the number of entities parented to this, including this
        sf::Uint32 size() const;

        void getVertices(std::vector<sf::Vertex>&);

    private:

        bool m_destroyed;
        sf::Uint64 m_uid;
        MessageBus& m_messageBus;
        sf::Int32 m_commandCategories;

        std::vector<std::unique_ptr<Component>> m_pendingComponents;
        std::vector<std::unique_ptr<Component>> m_components;
        std::vector<sf::Drawable*> m_drawables;

        std::vector<Ptr> m_children;
        Entity* m_parent;
        Scene* m_scene;

        sf::FloatRect globalBounds() const;

        void draw(sf::RenderTarget& rt, sf::RenderStates state) const override;
        void drawSelf(sf::RenderTarget&, sf::RenderStates) const;

        struct FindByName
        {
            FindByName(const std::string& name) : m_name(name){}
            bool operator()(const Component::Ptr& c){ return (c->getName() == m_name); }
        private:
            std::string m_name;
        };

        struct FindByUniqueType
        {
            FindByUniqueType(Component::UniqueType ut) : m_uniqueType(ut){}
            bool operator() (const Component::Ptr& c) { return (c->uniqueType() == m_uniqueType); }
        private:
            Component::UniqueType m_uniqueType;
        };
    };
}
#endif //ENTITY_HPP_