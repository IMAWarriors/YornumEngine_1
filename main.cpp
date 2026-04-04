// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.5.2
 *      > E-Build Alpha 1.6.0 [Stabilized Push!]: Refactored project structure
 *     -------------------------------------------------------------------------------------------------------
 *      > by IMAWarriors
 *      >  -------------------------------------------
 *          VERSION 
 *         -------------------------------------------
 *          E-Build Alpha 
 *          1.6.0
 *          [Stabilized Push!]
 *          Refactored project structure
 *      ------------------------------------------------------------------------------------------------------
 *      > Stable working version pushed to Git on 04-04-2026, 4:05 PM 
 *      ======================================================================================================
 *      >>> Most Recently Added Features
 *      ======================================================================================================
 * 
 *       - Fixed tileset and scene path loading bugs
 * 
 *       - Refactored project structure
 * 
 *       - Fixed large tileset loading

 * 
 *    ======================================================================================================= */


// C++ Util Includes
#include <algorithm>
#include <cmath>

#include <cstdlib>   // for rand(), srand()
#include <ctime>

#include <string>

#include "Engine/Core/Main/CoreApplication.h"
#include "Gamefiles/Game/GameEngine.h"



int main (int argc, char * argv[]) {

    bool first_scene_location_exists = false;
    std::string scene_path_to_open;

    // DEFAULT: LOAD INTO EMPTY SCENE
    if (argc<=1) {



    } else {

        switch (argc) {

            //  LOAD INTO SPECIFIED SCENE
            case 2: {
                scene_path_to_open = std::string(argv[1]);
                first_scene_location_exists = true;
                break;
            }

        }

    }

    srand(time(NULL));

    CoreApplication engine;
    GameEngine game;
    
    if (!first_scene_location_exists) {

        engine.RunCoreEngine(game);

    } else {

        engine.RunCoreEngine(game, scene_path_to_open);

    }


    return 0;

}


