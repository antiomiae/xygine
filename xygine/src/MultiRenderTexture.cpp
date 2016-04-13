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

#include "GLExtensions.hpp"

#include <xygine/MultiRenderTexture.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Log.hpp>
#include <xygine/detail/GLCheck.hpp>
#include <xygine/App.hpp>

#include <vector>

using namespace xy;

namespace
{
    const std::size_t MAX_TEXTURES = 4u;
}

MultiRenderTexture::MultiRenderTexture()
    : m_textureCount(0u),
    m_fbo           (0u),
    m_depthbuffer   (0u)
{

}

MultiRenderTexture::~MultiRenderTexture()
{
    ensureGlContext();

    if (m_depthbuffer)
    {
        GLuint depthbuffer = static_cast<GLuint>(m_depthbuffer);
        glCheck(glDeleteRenderbuffersEXT(1, &depthbuffer));
    }

    if (m_fbo)
    {
        GLuint fbo = static_cast<GLuint>(m_fbo);
        glCheck(glDeleteFramebuffersEXT(1, &fbo));
    }
}

//public
bool MultiRenderTexture::create(sf::Uint32 width, sf::Uint32 height, std::size_t count, bool depthBuffer)
{    
    if (ogl_ext_EXT_framebuffer_object == 0)
    {
        Logger::log("OpenGL extensions required for MRT are unavailble", Logger::Type::Error, Logger::Output::All);
        return false;
    }
    
    XY_ASSERT(count > 0, "You need to create at least one texture");
    XY_ASSERT(count <= MAX_TEXTURES, "Currently up to 4 textures are supported");
    XY_WARNING(count == 1, "Only having one texture may be sub-optimal. Consider using sf::RenderTexture instead.");

    m_textureCount = count;

    for (auto i = 0u; i < count; ++i)
    {
        if (!m_textures[i].create(width, height))
        {
            Logger::log("Failed creating MRT texture", Logger::Type::Error, Logger::Output::All);
            return false;
        }
        m_textures[i].setSmooth(false);
    }

    {
        m_context = std::make_unique<sf::Context>();
        GLuint fbo = 0;
        glCheck(glGenFramebuffersEXT(1, &fbo));
        m_fbo = static_cast<unsigned int>(fbo);
        if (m_fbo == 0)
        {
            Logger::log("Failed creating FBO for MRT", xy::Logger::Type::Error, xy::Logger::Output::All);
            return false;
        }
        glCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo));

        if (depthBuffer)
        {
            GLuint depth = 0;
            glCheck(glGenRenderbuffersEXT(1, &depth));
            m_depthbuffer = static_cast<unsigned int>(depth);
            if (m_depthbuffer == 0)
            {
                Logger::log("Failed creating depth buffer for MRT", Logger::Type::Error, Logger::Output::All);
                return false;
            }
            glCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthbuffer));
            glCheck(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height));
            glCheck(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthbuffer));
        }
        //attach textures
        for (auto i = 0u; i < count; ++i)
        {
            glCheck(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, m_textures[i].getNativeHandle(), 0));
        }

        //tell draw buffers which textures to use
        std::vector<GLenum> buffers;
        for (auto i = 0u; i < count; ++i)
        {
            buffers.push_back(GL_COLOR_ATTACHMENT0_EXT + i);
        }
        glCheck(glDrawBuffers(count, buffers.data()));

        //check everything went OK
        GLenum result;
        glCheck(result = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
        if (result != GL_FRAMEBUFFER_COMPLETE_EXT)
        {
            glCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
            Logger::log("Failed to attach texture to FBO", xy::Logger::Type::Error, xy::Logger::Output::All);
            return false;
        }
    }

    sf::RenderTarget::initialize();
    return true;
}

void MultiRenderTexture::setSmooth(bool smooth)
{
    for (auto i = 0u; i < m_textureCount; ++i)
    {
        m_textures[i].setSmooth(smooth);
    }
}

bool MultiRenderTexture::isSmooth() const
{
    return m_textures[0].isSmooth();
}

void MultiRenderTexture::setRepeated(bool repeat)
{
    for (auto i = 0u; i < m_textureCount; ++i)
    {
        m_textures[i].setRepeated(repeat);
    }
}

bool MultiRenderTexture::isRepeated() const
{
    return m_textures[0].isRepeated();
}

bool MultiRenderTexture::setActive(bool active)
{
    return m_context->setActive(active);
}

void MultiRenderTexture::display()
{
    if (setActive(true))
    {
        glCheck(glFlush());
        //TODO we need to mark textures as pixelsFlipped - but we don't have access here
    }
}

sf::Vector2u MultiRenderTexture::getSize() const
{
    return m_textures[0].getSize();
}

const sf::Texture& MultiRenderTexture::getTexture(std::size_t index) const
{
    XY_ASSERT(index < m_textureCount, "MultiRenderTexture index out of range.");
    return m_textures[index];
}

//private
bool MultiRenderTexture::activate(bool active)
{
    return setActive(active);
}