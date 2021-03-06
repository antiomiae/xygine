/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include "xyginext/core/ConfigFile.hpp"
#include "xyginext/graphics/SpriteSheet.hpp"
#include "xyginext/resources/Resource.hpp"

using namespace xy;

SpriteSheet::SpriteSheet()
{

}

//public
bool SpriteSheet::loadFromFile(const std::string& path, TextureResource& textures)
{
    ConfigFile sheetFile;
    if (!sheetFile.loadFromFile(xy::FileSystem::getResourcePath() + path))
    {
        return false;
    }

    m_sprites.clear();
    m_animations.clear();

    std::size_t count = 0;

    sf::Texture* texture = nullptr;
    sf::BlendMode blendMode = sf::BlendAlpha;

    //validate sprites, increase count
    if (auto* p = sheetFile.findProperty("src"))
    {
        m_texturePath = p->getValue<std::string>();
        texture = &textures.get(m_texturePath);
    }
    else
    {
        LOG(sheetFile.getId() + " missing texture property", Logger::Type::Error);
        return false;
    }

    if (auto* p = sheetFile.findProperty("blendmode"))
    {
        std::string mode = p->getValue<std::string>();
        if (mode == "add") blendMode = sf::BlendAdd;
        else if (mode == "multiply") blendMode = sf::BlendMultiply;
        else if (mode == "none") blendMode = sf::BlendNone;
    }

    if (auto* p = sheetFile.findProperty("smooth"))
    {
        texture->setSmooth(p->getValue<bool>());
    }

    const auto& sheetObjs = sheetFile.getObjects();
    for (const auto& spr : sheetObjs)
    {
        if (spr.getName() == "sprite")
        {
            std::string spriteName = spr.getId();
            if (m_sprites.count(spriteName) > 0)
            {
                Logger::log(spriteName + " already exists in sprite sheet", Logger::Type::Error);
                continue;
            }

            Sprite spriteComponent;
            if (texture != nullptr)
            {
                spriteComponent.setTexture(*texture);
            }

            //if (auto* p = spr.findProperty("blendmode"))
            //{
            //    //override sheet mode
            //    std::string mode = p->getValue<std::string>();
            //    if (mode == "add") spriteComponent.setBlendMode(sf::BlendAdd);
            //    else if (mode == "multiply") spriteComponent.setBlendMode(sf::BlendMultiply);
            //    else if (mode == "none") spriteComponent.setBlendMode(sf::BlendNone);
            //}
            //else
            //{
            //    spriteComponent.setBlendMode(blendMode);
            //}

            if (auto* p = spr.findProperty("bounds"))
            {
                spriteComponent.setTextureRect(p->getValue<sf::FloatRect>());
            }

            if (auto* p = spr.findProperty("colour"))
            {
                spriteComponent.setColour(p->getValue<sf::Color>());
            }

            const auto& spriteObjs = spr.getObjects();
            for (const auto& sprOb : spriteObjs)
            {
                if (sprOb.getName() == "animation")
                {
                    auto& anim = spriteComponent.m_animations[spriteComponent.m_animationCount];
                    
                    const auto& properties = sprOb.getProperties();
                    for (const auto& p : properties)
                    {
                        std::string name = p.getName();
                        if (name == "frame")
                        {
                            anim.frames[anim.frameCount++] = p.getValue<sf::FloatRect>();
                        }
                        else if (name == "framerate")
                        {
                            anim.framerate = p.getValue<float>();
                        }
                        else if (name == "loop")
                        {
                            anim.looped = p.getValue<bool>();
                        }
                        else if (name == "loop_start")
                        {
                            anim.loopStart = p.getValue<sf::Int32>();
                        }
                    }

                    auto animId = sprOb.getId();
                    m_animations[spriteName].push_back(animId);
                    animId.copy(anim.id.data(), animId.length());
                    
                    spriteComponent.m_animationCount++;
                }
            }

            m_sprites.insert(std::make_pair(spriteName, spriteComponent));
            count++;
        }
    }

    //LOG("Found " + std::to_string(count) + " sprites in " + path, Logger::Type::Info);
    return count > 0;
}

bool SpriteSheet::saveToFile(const std::string &path)
{
    ConfigFile sheetFile;
    sheetFile.addProperty("src", "\"" + m_texturePath + "\"");
    sheetFile.addProperty("smooth").setValue( m_smooth ? true : false);
    
    for (auto& sprite : m_sprites)
    {
        auto sprObj = sheetFile.addObject("sprite", sprite.first);
        sprObj->addProperty("bounds").setValue(sprite.second.getTextureRect());
        sprObj->addProperty("colour").setValue(sprite.second.getColour());
        
        auto& anims = sprite.second.getAnimations();
        for (auto i(0u); i < sprite.second.getAnimationCount(); i++)
        {
            auto animObj = sprObj->addObject("animation", m_animations[sprite.first][i]);
            animObj->addProperty("framerate").setValue(anims[i].framerate);
            animObj->addProperty("loop").setValue(anims[i].looped);
            
            auto& frames = anims[i].frames;
            for (auto j(0u); j < anims[i].frameCount; j++)
            {
                animObj->addProperty("frame").setValue(frames[j]);
            }
        }
    }
    
    return sheetFile.save(path);
}

const std::string& SpriteSheet::getTexturePath() const
{
    return m_texturePath;
}

void SpriteSheet::setTexturePath(const std::string& path)
{
    m_texturePath = path;
}

Sprite SpriteSheet::getSprite(const std::string& name) const
{
    if (m_sprites.count(name) != 0)
    {
        return m_sprites[name];
    }
    LOG(name + " not found in sprite sheet", Logger::Type::Warning);
    return {};
}

void SpriteSheet::setSprite(const std::string& name, const xy::Sprite& data)
{
    m_sprites[name] = data;
}

void SpriteSheet::removeSprite(const std::string& name)
{
    m_sprites.erase(name);
}

const std::unordered_map<std::string, Sprite>& SpriteSheet::getSprites() const
{
    return m_sprites;
}

std::size_t SpriteSheet::getAnimationIndex(const std::string& name, const std::string& spriteName) const
{
    if (m_animations.count(spriteName) != 0)
    {
        const auto& anims = m_animations[spriteName];
        const auto& result = std::find(anims.cbegin(), anims.cend(), name);
        if (result == anims.cend()) return 0;

        return std::distance(anims.cbegin(), result);
    }
    return 0;
}
