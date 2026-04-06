// WindowPile.h

#ifndef WINDOWPILE_H
#define WINDOWPILE_H

#include "../Overhead/Config.h"

#include "raylib.h"

class Window {


    private:

        bool fullscreen;


    public:

        bool editorflag = false;

        Window (const char * title, bool edit = false);

        void ListenFullscreenToggle ();
        
        void EndProgram ();

        bool IsFullscreen () const;

        bool IsRunning () const;



        // SHADERS

        Shader painter;


    // Functions

};


#endif

