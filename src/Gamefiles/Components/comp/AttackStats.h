// AttackStats.h

#ifndef ATTACKSTATS_H
#define ATTACKSTATS_H

#include <algorithm>
#include <cmath>
#include <deque>
#include <vector>
#include <cassert>

#include "../../../Engine/ECS/Entity.h"

#include "../../../Engine/Core/Overhead/GameTypes.h"

#include "../../Elements/Avatar/Avatar.h"
#include "../../Elements/Avatar/Animation.h"


#include "../../Assets/AssetManager.h"

#include "Transform.h"

namespace hbpos {

    struct RECT {
        Vec2 POS    = { 0.0f,   0.0f    };
        Vec2 SIZE   = { 100.0f, 100.0f  };
    };

    struct Attack {

        // Members
        int frame_count = 0;
        std::vector<RECT> frames;
        float base_speed_ms = 35.0f;

        // Default Constructor
        Attack () {
            frame_count = 0;
            frames.clear();
        }

        // Blank Constructor??
        Attack (int fc) {
            assert(fc >= 0);
            frame_count = fc;
            frames = std::vector<RECT>(fc, { {0.0f,0.0f},{0.0f,0.0f} });
        }

        // Standard Constructor
        Attack (int fc, const std::vector<RECT>& frame_data, float speed_ms) {
            assert((int)frame_data.size() == fc);
            frame_count = fc;
            frames = frame_data;
            base_speed_ms = speed_ms;
        }

    };

    // Right Swing (Aggressor) Hitbox Coordinates
    const Attack STANDARD_SWING ( 8, {
        { {-50.0f, -20.0f}, {30.0f, 100.0f}     }, 
        { {-30.0f, -20.0f}, {50.0f, 100.0f}     },
        { {-10.0f, -20.0f}, {130.0f, 100.0f}    },
        { {20.0f, -20.0f}, {150.0f, 100.0f}    },
        { {40.0f, -20.0f}, {180.0f, 100.0f}     },
        { {60.0f, -20.0f}, {140.0f, 100.0f}     },
        { {90.0f, -20.0f}, {90.0f, 100.0f}    },
        { {120.0f, -20.0f}, {45.0f, 100.0f}    }},
        35.0f
    );

    // Right Backslash (Aggressor) Hitbox Coordinates
    const Attack STANDARD_BACKSLASH ( 8, {
        { {-50.0f, -20.0f}, {30.0f, 100.0f}     }, 
        { {-20.0f, -20.0f}, {50.0f, 100.0f}     },
        { {0.0f, -20.0f}, {130.0f, 100.0f}    },
        { {30.0f, -20.0f}, {150.0f, 100.0f}    },
        { {60.0f, -20.0f}, {180.0f, 100.0f}     },
        { {90.0f, -20.0f}, {140.0f, 100.0f}     },
        { {110.0f, -20.0f}, {90.0f, 100.0f}    },
        { {140.0f, -20.0f}, {45.0f, 100.0f}    }},
        35.0f
    );

    // Right Swipe (Combo Response I) Hitbox Coordinates
    const Attack STANDARD_SWIPE ( 8, {
        { {-50.0f, -20.0f}, {30.0f, 100.0f}     }, 
        { {-20.0f, -20.0f}, {50.0f, 100.0f}     },
        { {0.0f, -20.0f}, {130.0f, 100.0f}    },
        { {30.0f, -20.0f}, {150.0f, 100.0f}    },
        { {60.0f, -20.0f}, {180.0f, 100.0f}     },
        { {90.0f, -20.0f}, {140.0f, 100.0f}     },
        { {110.0f, -20.0f}, {90.0f, 100.0f}    },
        { {140.0f, -20.0f}, {45.0f, 100.0f}    }},
        35.0f
    );

     // Right Backswipe (Combo Response II) Hitbox Coordinates
    const Attack STANDARD_BACKSWIPE ( 8, {
        { {-50.0f, -20.0f}, {30.0f, 100.0f}     }, 
        { {-20.0f, -20.0f}, {50.0f, 100.0f}     },
        { {0.0f, -20.0f}, {130.0f, 100.0f}    },
        { {30.0f, -20.0f}, {150.0f, 100.0f}    },
        { {60.0f, -20.0f}, {180.0f, 100.0f}     },
        { {90.0f, -20.0f}, {140.0f, 100.0f}     },
        { {110.0f, -20.0f}, {90.0f, 100.0f}    },
        { {140.0f, -20.0f}, {45.0f, 100.0f}    }},
        35.0f
    );

    


}


namespace comp {

struct AttackStats {

    hbpos::Attack ilk_id;

    Entity owner;
    int attack_frame = 0;
    bool delete_flag = false;
    float tick_time = 35.0f;    // Time in milliseconds
    float accumulator = 0.0f;
    bool mirror_attack = false;

    AttackStats (const Entity& master, const hbpos::Attack& attack, bool mirror = false) {
        ilk_id = attack;
        owner = master;
        mirror_attack = mirror;
        tick_time = attack.base_speed_ms;
    }


    int tick_one_frame (float deltatime) {

        float adjusted_tick_time = tick_time / 1000.0f;  

        accumulator += deltatime;
        int frames_traveled = 0;

        while (accumulator >= adjusted_tick_time) {
            attack_frame++;
            frames_traveled++;
            accumulator -= adjusted_tick_time;
        }

        if (attack_frame >= ilk_id.frame_count) {
            delete_flag = true;
        }

        return frames_traveled;
    }
    
};

}

namespace tag {

    struct AttackDataNode {

    };

};

#endif