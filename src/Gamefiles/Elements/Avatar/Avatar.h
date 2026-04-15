// Avatar.h

#ifndef AVATAR_H
#define AVATAR_H

#include "../../../Engine/Core/Overhead/GameTypes.h"

#include <string>
#include <vector>

struct Joint {
    std::string name;
    Vec2 origin;
    Vec2 direction;
};

struct KeyFrame {
    Vec2 origin;
    Vec2 direction;
};

struct Animation {
    std::string name;
    int frame_count = 0;
    std::vector<KeyFrame> frames;
};

class Avatar {

public:

    Vec2 position;
    std::vector<Joint> origin_joints;
    std::vector<KeyFrame> active_frame;
    std::vector<Animation> animations;

    Avatar () {
        position = {0.0f, 0.0f};
        origin_joints.clear();
        active_frame.clear();
        animations.clear();
    }

    void wipe_joints () {
        origin_joints.clear();
    }

    void wipe_active_frame () {
        active_frame.clear();
    }
    
    void wipe_animations () {
        animations.clear();
    }

    void new_joint (const std::string & name, Vec2 origin, Vec2 direction) {
        origin_joints.push_back({name, origin, direction});
    }

    

    




private:

    

};


#endif