/*********************************************************************
Matt Marchant 2014 - 2016
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

#include <xygine/tilemap/Tileset.hpp>
#include <xygine/tilemap/FreeFuncs.hpp>
#include <xygine/parsers/pugixml.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Log.hpp>

#include <ctype.h>

using namespace xy;
using namespace xy::tmx;

Tileset::Tileset(const std::string& workingDir)
    : m_workingDir  (workingDir),
    m_firstGID      (0),
    m_spacing       (0),
    m_margin        (0),
    m_tileCount     (0),
    m_columnCount   (0)
{

}

//public
void Tileset::parse(pugi::xml_node node, xy::TextureResource& textureResource)
{
    std::string attribString = node.name();
    if (attribString != "tileset")
    {
        Logger::log(attribString + ": not a tileset node! Node will be skipped.", Logger::Type::Warning);
        return;
    }
    
    m_firstGID = node.attribute("firstgid").as_int();
    if (m_firstGID == 0)
    {
        Logger::log("Invalid first GID in tileset. Tileset node skipped.", Logger::Type::Warning);
        return;
    }

    pugi::xml_document tsxDoc; //need to keep this in scope
    if (node.attribute("source"))
    {
        //parse TSX doc
        std::string path = node.attribute("source").as_string();
        path = resolveFilePath(path);

        //see if doc can be opened
        
        auto result = tsxDoc.load_file(path.c_str());
        if (!result)
        {
            Logger::log("Failed opening tsx file for tile set, tile set will be skipped", Logger::Type::Error);
            return reset();
        }

        //if it can then replace the current node with tsx node
        node = tsxDoc.child("tileset");
        if (!node)
        {
            Logger::log("tsx file does not contain a tile set node, tile set will be skipped", Logger::Type::Error);
            return reset();
        }
    }

    m_name = node.attribute("name").as_string();
    LOG("found tile set " + m_name, Logger::Type::Info);

    m_tileSize.x = node.attribute("tilewidth").as_int();
    m_tileSize.y = node.attribute("tileheight").as_int();
    if (m_tileSize.x == 0 || m_tileSize.y == 0)
    {
        Logger::log("Invalid tile size found in tile set node. Node will be skipped.", Logger::Type::Error);
        return reset();
    }

    m_spacing = node.attribute("spacing").as_int();
    m_margin = node.attribute("margin").as_int();
    m_tileCount = node.attribute("tilecount").as_int();
    m_columnCount = node.attribute("columns").as_int();

    const auto& children = node.children();
    for (const auto& node : children)
    {
        std::string name = node.name();
        if (name == "image")
        {
            m_texture = parseImageNode(node, textureResource);
        }
        else if (name == "tileoffset")
        {
            parseOffsetNode(node);
        }
        else if (name == "properties")
        {
            parsePropertyNode(node);
        }
        else if (name == "terraintypes")
        {
            parseTerrainNode(node);
        }
        else if (name == "tile")
        {
            parseTileNode(node, textureResource);
        }
    }
}

//private
void Tileset::reset()
{
    m_firstGID = 0;
    m_source = "";
    m_name = "";
    m_tileSize = { 0,0 };
    m_spacing = 0;
    m_margin = 0;
    m_tileCount = 0;
    m_columnCount = 0;
    m_tileOffset = { 0,0 };
    m_properties.clear();
    m_texture = sf::Texture();
    m_terrainTypes.clear();
    m_tiles.clear();
}

sf::Texture Tileset::parseImageNode(const pugi::xml_node& node, xy::TextureResource& tr)
{
    //LOG("found image node!", Logger::Type::Info);
    //TODO check format attrib and data node.
    //currently I can't see how to export embedded images
    //from tiled so I don't know how to test this

    std::string file = node.attribute("source").as_string();
    if (file.empty())
    {
        Logger::log("Tileset image node has missing source property.");
        tr.setFallbackColour(sf::Color::Magenta);
        return tr.get("missing_tileset");
    }

    //file is a relative path so we need to resolve directory
    //movement to working directory
    std::string imagePath = resolveFilePath(file);
    sf::Texture texture = tr.get(imagePath);

    //if we have a transparency colour then set the
    //matching pixels alpha to zero
    if (node.attribute("trans"))
    {
        std::string colourStr = node.attribute("trans").as_string();
        sf::Color colour = colourFromString(colourStr);

        sf::Image img = texture.copyToImage();
        img.createMaskFromColor(colour);
        texture.update(img);
        LOG("Set tile set transparency colour to " + colourStr, Logger::Type::Info);
    }
    return texture;
}

void Tileset::parseOffsetNode(const pugi::xml_node& node)
{
    //LOG("found offset node!", Logger::Type::Info);
    m_tileOffset.x = node.attribute("x").as_int();
    m_tileOffset.y = node.attribute("y").as_int();
}

void Tileset::parsePropertyNode(const pugi::xml_node& node)
{
    //LOG("found property node!", Logger::Type::Info);
    const auto& children = node.children();
    for (const auto& child : children)
    {
        m_properties.emplace_back();
        m_properties.back().parse(child);
    }
}

void Tileset::parseTerrainNode(const pugi::xml_node& node)
{
    //LOG("found terrain node!", Logger::Type::Info);
    const auto& children = node.children();
    for (const auto& child : children)
    {
        std::string name = child.name();
        if (name == "terrain")
        {
            m_terrainTypes.emplace_back();
            auto& terrain = m_terrainTypes.back();
            terrain.name = child.attribute("name").as_string();
            terrain.tileID = child.attribute("tile").as_int();
            auto properties = child.child("properties");
            if (properties)
            {
                for (const auto& p : properties)
                {
                    name = p.name();
                    if (name == "property")
                    {
                        terrain.properties.emplace_back();
                        terrain.properties.back().parse(p);
                    }
                }
            }
        }
    }
}

void Tileset::parseTileNode(const pugi::xml_node& node, xy::TextureResource& tr)
{
    //LOG("found tile node!", Logger::Type::Info);
    Tile tile;
    tile.ID = node.attribute("id").as_int();
    if (node.attribute("terrain"))
    {
        std::string data = node.attribute("terrain").as_string();
        bool lastWasChar = true;
        std::size_t idx = 0u;
        for (auto i = 0u; i < data.size() && idx < tile.terrainIndices.size(); ++i)
        {
            if (isdigit(data[i]))
            {
                tile.terrainIndices[idx++] = std::atoi(&data[i]);
                lastWasChar = false;
            }
            else
            {
                if (!lastWasChar)
                {
                    lastWasChar = true;
                }
                else
                {
                    tile.terrainIndices[idx++] = -1;
                    lastWasChar = false;
                }
            }
        }
        if (lastWasChar)
        {
            tile.terrainIndices[idx] = -1;
        }
    }

    tile.probability = node.attribute("probability").as_int(100);

    const auto& children = node.children();
    for (const auto& child : children)
    {
        std::string name = child.name();
        if (name == "properties")
        {
            for (const auto& prop : child.children())
            {
                tile.properties.emplace_back();
                tile.properties.back().parse(prop);
            }
        }
        else if (name == "objectgroup")
        {
            tile.objectGroup.parse(child);
        }
        else if (name == "image")
        {
            tile.texture = parseImageNode(child, tr);
        }
        else if (name == "animation")
        {
            for (const auto& frameNode : child.children())
            {
                Tile::Animation::Frame frame;
                frame.duration = frameNode.attribute("duration").as_int();
                frame.tileID = frameNode.attribute("tileid").as_int();
                tile.animation.frames.push_back(frame);
            }
        }
    }
    m_tiles.push_back(tile);
}

std::string Tileset::resolveFilePath(std::string path)
{
    static const std::string match("../");
    std::size_t result = path.find(match);
    std::size_t count = 0;
    while (result != std::string::npos)
    {
        count++;
        path = path.substr(result + match.size());
        result = path.find(match);
    }

    std::string outPath = m_workingDir;
    for (auto i = 0u; i < count; ++i)
    {
        result = outPath.find_last_of('/');
        if (result != std::string::npos)
        {
            outPath = outPath.substr(0, result);
        }
    }
    return std::move(outPath += '/' + path);
}