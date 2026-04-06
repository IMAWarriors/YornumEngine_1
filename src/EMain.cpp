// Editor.h


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
    
    engine.RunCoreEngineMainEditor(game);

    return 0;

}


