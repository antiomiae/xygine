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

#ifndef DEMO_GAME_MENU_STATE_HPP_
#define DEMO_GAME_MENU_STATE_HPP_

#include <xyginext/core/State.hpp>
#include <xyginext/core/ConfigFile.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/resources/Resource.hpp>

#include "StateIDs.hpp"
#include "SharedStateData.hpp"

namespace xy
{
    class PostBlur;
    class Transform;
}

class LoadingScreen;

class MenuState final : public xy::State
{
public:
    MenuState(xy::StateStack&, xy::State::Context, SharedStateData&, LoadingScreen&);

    xy::StateID stateID() const override { return StateID::MainMenu; }

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

private:
    xy::Scene m_scene;
    xy::Scene m_helpScene;
    xy::FontResource m_fontResource;
    xy::TextureResource m_textureResource;

    SharedStateData& m_sharedStateData;
    LoadingScreen& m_loadingScreen;

    bool m_helpShown;
    xy::PostBlur* m_blurEffect;

    xy::ConfigFile m_keyBinds;

    sf::Vector2f m_menuTarget;
    sf::Vector2f m_leftMenuTarget;
    sf::Vector2f m_rightMenuTarget;

    void loadKeybinds();
    void createScene();
    void createMenu();
    void createHelp();
    void showHelpMenu();

    void createFirstMenu(xy::Transform&, sf::Uint32, sf::Uint32, sf::Font&);
    void createSecondMenu(xy::Transform&, sf::Uint32, sf::Uint32, sf::Font&);
    void createThirdMenu(xy::Transform&, sf::Uint32, sf::Uint32, sf::Font&);

    void updateLoadingScreen(float, sf::RenderWindow&) override;
};

#endif //DEMO_GAME_MENU_STATE_HPP_