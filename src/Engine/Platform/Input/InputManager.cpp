// InputManager.cpp

#include "InputManager.h"

InputManager::InputManager() {
    set_keybinds (KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_F11);
};


InputManager::InputManager(int mode) {

    switch (mode) {

        case 0: {
            set_keybinds (KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_F11);
            break;
        }

        case 1: {
            set_keybinds (KEY_D, KEY_A, KEY_W, KEY_S, KEY_SPACE, KEY_Q, KEY_E, KEY_LEFT_SHIFT, KEY_F11);
            break;
        }

    }
};

void InputManager::set_keybinds (int _r, int _l, int _u, int _d, int _act, int _can, int _spc, int _men, int _fs) {

    BIND_RDIR                 = _r;
    BIND_LDIR                 = _l;
    BIND_UDIR                 = _u;
    BIND_DDIR                 = _d;
    BIND_ACTION               = _act;
    BIND_CANCEL               = _can;
    BIND_SPECIAL              = _spc;
    BIND_MENU                 = _men;
    BIND_FS                   = _fs;

}

// Update active key values

int InputManager::get_move_axis () { return ( IsKeyDown(BIND_RDIR) - IsKeyDown(BIND_LDIR) ); }

int InputManager::get_vert_axis () { return ( IsKeyDown(BIND_DDIR) - IsKeyDown(BIND_UDIR) ); }

bool InputManager::get_jump_key () { return (IsKeyDown(BIND_UDIR)); }



