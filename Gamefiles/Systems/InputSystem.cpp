// InputSystem.cpp

#include "InputSystem.h"


void InputSystem::update (Registry & registry, float deltatime)  {

    for (Entity entity : registry.view<comp::InputState>()) {

        comp::InputState & inputState = registry.get_component<comp::InputState>(entity);

        inputState.horz_axis = (float)(input.get_move_axis());
        inputState.jump_key = input.get_jump_key();

    }

}





