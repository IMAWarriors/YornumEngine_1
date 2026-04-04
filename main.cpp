// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.6.1
 *      > E-Build Alpha 1.6.1 [Stabilized Push!]: Fixed minor bugs
 *     -------------------------------------------------------------------------------------------------------
 *      > by IMAWarriors
 *      >  -------------------------------------------
 *          VERSION 
 *         -------------------------------------------
 *          E-Build Alpha 
 *          1.6.1
 *          [Stabilized Push!]
 *          Refactored project structure
 *      ------------------------------------------------------------------------------------------------------
 *      > Stable working version pushed to Git on 04-04-2026, 7:25 PM 
 *      ======================================================================================================
 *      >>> New to this major build:                
 *      ======================================================================================================
 * 
 *       - Added shader support and fixed import bugs
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


