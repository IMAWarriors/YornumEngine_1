// InputManager.h

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "raylib.h"

class InputManager {

    private:

        // CONTROL BINDS

        int BIND_RDIR                 = KEY_RIGHT;
        int BIND_LDIR                 = KEY_LEFT;
        int BIND_UDIR                 = KEY_UP;
        int BIND_DDIR                 = KEY_DOWN;
        int BIND_ACTION               = KEY_Z;
        int BIND_CANCEL               = KEY_X;
        int BIND_SPECIAL              = KEY_C;
        int BIND_MENU                 = KEY_V;

        int BIND_FS                   = KEY_F11;


        

        


    public:

        // Constructor
        InputManager();
        InputManager(int mode);

        // Update active key values

        void set_keybinds (int _r, int _l, int _u, int _d, int _act, int _can, int _spc, int _men, int _fs);


        
        // void get_raw_keystates ();

        // Key Accessors

        int get_move_axis ();

        bool get_jump_key ();

        


};





#endif 

