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

#ifndef DEMO_MENU_CALLBACKS_HPP_
#define DEMO_MENU_CALLBACKS_HPP_

#include <xyginext/ecs/Entity.hpp>

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Sprite.hpp>

class HelpMenuCallback final
{
public:
    HelpMenuCallback(const bool& shown)
        : m_shown(shown)
    {

    }

    void operator()(xy::Entity entity, float dt)
    {
        float target = 0.f;
        if (!m_shown)
        {
            //scroll to hidden
            target = HidePosition;
        }
        
        auto position = entity.getComponent<xy::Transform>().getPosition();
        position.y = target - position.y;
        
        if (std::abs(position.y) < 3.f)
        {
            position.y = target;
            entity.getComponent<xy::Transform>().setPosition(position);
        }
        else
        {
            position.x = 0.f;
            entity.getComponent<xy::Transform>().move(position * Speed * dt);
        }
    }

    static constexpr float HidePosition = -890.f;
    static constexpr float ShowPosition = 0.f;

private:
    const bool& m_shown;

    static constexpr float Speed = 5.f;
};

class HelpBackgroundCallback final
{
private:
    static constexpr float MaxTime = 0.5f;
    static constexpr float MaxAlpha = 102.f;

public:
    HelpBackgroundCallback(const bool& shown)
        : m_shown(shown)
    {

    }

    void operator() (xy::Entity entity, float dt)
    {
        if (m_shown)
        {
            m_currentTime = std::min(m_currentTime + dt, 0.5f); //TODO find out why g++ things MaxTime is undef ref
        }
        else
        {
            m_currentTime = std::max(0.f, m_currentTime - dt);
        }

        float target = m_currentTime / MaxTime;
        sf::Uint8 targetAlpha = static_cast<sf::Uint8>(target * MaxAlpha);

        entity.getComponent<xy::Sprite>().setColour({ 255, 255, 255, targetAlpha });
    }

private:
    const bool& m_shown;

    float m_currentTime = 0.f;
};

#endif //DEMO_MENU_CALLBACKS_HPP_