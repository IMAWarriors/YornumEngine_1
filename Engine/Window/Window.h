// WindowPile.h

#ifndef WINDOWPILE_H
#define WINDOWPILE_H

#include "../Core/Config.h"

#include "raylib.h"

class Window {


    private:

        bool fullscreen;

    public:

        Window (const char * title);

        void ListenFullscreenToggle ();
        
        void EndProgram ();

        bool IsFullscreen () const;


    // Functions

};


#endif

