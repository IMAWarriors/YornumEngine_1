// InputState.h

#ifndef COMP_INPUTSTATE_H
#define COMP_INPUTSTATE_H

#include "../../../Engine/Core/Overhead/GameTypes.h"

namespace comp {

    struct InputState {

        int horz_axis;
        int jump_key;
        bool attack_key_down;
        bool attack_key_tapped;

        
    };

}

#endif
