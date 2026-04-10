// PhysicsBody.h

#ifndef PHYSICSBODY_H
#define PHYSICSBODY_H

#include "../../../Engine/Core/Overhead/GameTypes.h"

namespace comp {

struct PhysicsBody {

    // Colliders
    Vec2 size;
    float skin;
    bool solid;
    bool gravitous;
    float gravity;
    bool render_hitbox;
    

    // State?
    bool inColl;
    bool innerSkinInColl;
    bool onSolidGround;

    int falling;
    int vjump_window;

    int wallPush;
    bool againstWall;
    int walljumpBuffer;
    int lastWalljumpDir;
    int walljumpWindow;
    int lastWallPush;


};








}


#endif