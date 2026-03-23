// Editor.h
/**    =======================================================================================================
 *      > EDITOR MAIN FILE of Version E-Build Alpha 1.0.1
 *      > by IMAWarriors
 *      --------------------------------------
 * 
 *     
 * 
 *    ======================================================================================================= */


// C++ Util Includes
#include <algorithm>
#include <cmath>

#include <cstdlib>   // for rand(), srand()
#include <ctime>

#include "../../Engine/Core/Main/CoreApplication.h"
#include "../../Gamefiles/Game/GameEngine.h"



int main () {

    srand(time(NULL));

    CoreApplication engine;
    GameEngine game;
    
    engine.RunCoreEngineMainEditor(game);

    return 0;

}


