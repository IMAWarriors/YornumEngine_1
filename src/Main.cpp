// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.6.2
 *      > E-Build Alpha 1.6.2 [Stabilized Push!]: Fixed tile splitting
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
 *      > Stable working version pushed to Git on 04-04-2026, 9:05 PM 
 *      ======================================================================================================
 *      >>> New to this major build:                
 *      ======================================================================================================
 * 
 *       - Fixed tile splitting
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

struct LaunchRequest {

    bool FLAG_INTO_SCENE    = false;
    bool FLAG_DEBUG         = false;

    std::string INTO_SCENE_PATH = "";

};


int main (int argc, char * argv[]) {

    LaunchRequest launch;


    auto same = [](char* str1, char* str2) -> bool {
        char * ptr1 = str1;
        char * ptr2 = str2;

        while (true) {
            
            // If letters do not match...
            if (*ptr1 != *ptr2) {
                return false;
            }

            // If our letters do match
            if (*ptr1 == '\0' || *ptr2 == '\0') {
                return true;
            }
            ptr1++;
            ptr2++;
        }

        return true;
    };


    // DEFAULT: LOAD INTO EMPTY SCENE
    if (argc<=1) {



    } else {

        
        if (argc > 1) {

            for (int i = 1; i < argc; i++) {
                char * argument = argv[i];

                if (same(argument, "debug")) {
                    // Launch with debug flag mode
                    launch.FLAG_DEBUG = true;

                } else if (same(argument, "scene")) {
                    //  LOAD INTO SPECIFIED SCENE
                    launch.FLAG_INTO_SCENE = true;

                    i++; // Skip extra argument and load that argument as the path
                    if (i>=argc) { launch.FLAG_INTO_SCENE = false; break; }
                    launch.INTO_SCENE_PATH = std::string(argv[i]);
                    
                }
            }
        }
        
    }

    srand(time(NULL));

    CoreApplication engine;
    GameEngine game;

    if (launch.FLAG_DEBUG) {
        game.SetGameFlag(0, true);
    }

    if (launch.FLAG_INTO_SCENE) {
        engine.RunCoreEngine(game, launch.INTO_SCENE_PATH);
    } else {
        engine.RunCoreEngine(game);
    }

    

    
    


    return 0;

}


