// CoreApplication.h

/** CORE APPLICATION CLASS
 *  ==============================================================================================================
 *  ||  This class should be responsible for all Window setup, rendering setup, and other things responsible    ||
 *  ||  for initializing the game environment; this class should have NO gameplay dependencies                  ||
 *  ============================================================================================================== 
 */

#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include "../Overhead/Config.h"                    // Get the CONFIGURATION CONSTNATS
#include "../Overhead/DebugStats.h"
#include "../Window/Window.h"                      // Get the WINDOW APPLICATION ASSETS
#include "../Rendering/Renderer.h"                 // Get the RENDERER PIPELINE ASSETS
#include "../../Platform/Input/InputManager.h"     // Get the INPUTS LAYER INFO
#include "../../ECS/Enum_SystemPhases.h"

#include "../../../Gamefiles/Game/GameEngine.h"



class GameEngine;

class CoreApplication {


    private:

        GameEngine game;



    public:

        void RunCoreEngine (GameEngine & game);





};




#endif









