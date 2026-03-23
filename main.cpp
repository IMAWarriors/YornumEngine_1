// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.2.0
 *      > by IMAWarriors
 *      --------------------------------------
 *      > Stable working version pushed to Git on 03-23-2026, 6:05 PM 
 * 
 *      >>> Most Recently Added Features
 *       - Added asset manager
 *       - Began editor build template
 *       - Imported Dear ImGui and rlImGui
 *       - Fixed debug components
 *       - Added Tileset and Atlas splitting capabilities
 *       - Queue for Component/Entity Management
 *       - Stabilized Camera and Render interpolation
 * 
 *      >>> Working-in-Progress Features (as of this push, goals/objectives)
 *       - Still trying to add more editor related features for easy debug/expansion
 * 
 * 
 *    ======================================================================================================= */


// C++ Util Includes
#include <algorithm>
#include <cmath>

#include <cstdlib>   // for rand(), srand()
#include <ctime>

#include "Engine/Core/Main/CoreApplication.h"
#include "Gamefiles/Game/GameEngine.h"



int main () {

    srand(time(NULL));

    CoreApplication engine;
    GameEngine game;
    
    engine.RunCoreEngine(game);

    return 0;

}


