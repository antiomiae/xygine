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

#pragma once

#include "InputBinding.hpp"

#include <string>
#include <array>

struct SharedStateData final
{
    enum
    {
        Host,
        Client
    }hostState = Client;
    std::string remoteIP = "127.0.0.1";
    std::string error;

    std::array<std::string, 2u> scores;
    std::size_t playerCount = 1;
    std::size_t continueCount = 0;

    std::array<InputBinding, 2u> inputBindings;
};
