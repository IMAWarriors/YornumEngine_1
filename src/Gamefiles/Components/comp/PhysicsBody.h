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

    // THIS DIRECTION VARIABLE IS USED FOR ANIMATION ONLY
    // This direction variable ONLY tracks what key
    // was pressed last LEFT or RIGHT for the player,
    // NOT necessarily which direction the player is
    // moving. Wall jumping forces an immediate turn around,
    // but other than that, pressing LEFT makes this -1
    // and pressing RIGHT makes this 1. 
    // This so so that if you begin moving RIGHT
    // for example and then start trying to move 
    // to the LEFT, you may be going RIGHT for a few
    // frames while your left over RIGHTWARD velocity
    // gradually moves towards a negative number,
    // but direction instantly snaps to -1 since we want
    // the player animation to look like he's trying to go
    // left to increase responsiveness feel even if he is
    // techinically still moving right
    int direction = 1;


};








}


#endif