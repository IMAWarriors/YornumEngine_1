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
        int painter_camera_pos_loc = -1;
        int painter_resolution_loc = -1;
        int painter_viewport_loc = -1;
        int painter_zoom_loc = -1;
        int painter_time_loc = -1;
        bool painter_ready = false;
        bool painter_enabled_game = true;
        bool painter_enabled_editor = false;


    // Functions

};


#endif
