//#include "Szczur/Utility/Modules/Module.hpp"
#pragma once
#include "Szczur/Modules/Window/Window.hpp"
#include "Szczur/Modules/Input/Input.hpp"
#include "Szczur/Utility/Logger.hpp"

#include "PPContainer.hpp"

namespace rat
{

class PrepScreen : public Module <Window, Input>
{
public:
    void init()
    {
        
    }

    PrepScreen()
    {
        LOG_INFO(this, " : PrepScreen initing...");
        init();
        LOG_INFO(this, " : PrepScreen inited.");
    }

};
}