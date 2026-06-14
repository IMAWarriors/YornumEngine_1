// AttackStats.h

#ifndef ATTACKSTATS_H
#define ATTACKSTATS_H

#include <algorithm>
#include <cmath>
#include <deque>
#include <vector>
#include <cassert>

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

        // Default Constructor
        Attack () {
            int frame_count = 0;
            frames.clear();
        }

        // Blank Constructor??
        Attack (int fc) {
            assert(fc >= 0);
            frame_count = fc;
            frames = std::vector<RECT>(fc, { {0.0f,0.0f},{0.0f,0.0f} });
        }

        // Standard Constructor
        Attack (int fc, const std::vector<RECT>& frame_data) {
            assert((int)frame_data.size() == fc);
            frames = frame_data;
        }

    };

    // Right Swing (Aggressor) Hitbox Coordinates
    const Attack STANDARD_SWING ( 8, {
        { {-50.0f, -20.0f}, {30.0f, 100.0f}     }, 
        { {-20.0f, -20.0f}, {50.0f, 100.0f}     },
        { {0.0f, -20.0f}, {130.0f, 100.0f}    },
        { {30.0f, -20.0f}, {150.0f, 100.0f}    },
        { {60.0f, -20.0f}, {180.0f, 100.0f}     },
        { {90.0f, -20.0f}, {140.0f, 100.0f}     },
        { {110.0f, -20.0f}, {90.0f, 100.0f}    },
        { {140.0f, -20.0f}, {45.0f, 100.0f}    } 
    });

    // Right Backslash (Aggressor) Hitbox Coordinates
    const Attack STANDARD_BACKSLASH ( 8, {
        { {-50.0f, -20.0f}, {30.0f, 100.0f}     }, 
        { {-20.0f, -20.0f}, {50.0f, 100.0f}     },
        { {0.0f, -20.0f}, {130.0f, 100.0f}    },
        { {30.0f, -20.0f}, {150.0f, 100.0f}    },
        { {60.0f, -20.0f}, {180.0f, 100.0f}     },
        { {90.0f, -20.0f}, {140.0f, 100.0f}     },
        { {110.0f, -20.0f}, {90.0f, 100.0f}    },
        { {140.0f, -20.0f}, {45.0f, 100.0f}    } 
    });

    // Right Swipe (Combo Response I) Hitbox Coordinates
    const Attack STANDARD_SWIPE ( 8, {
        { {-50.0f, -20.0f}, {30.0f, 100.0f}     }, 
        { {-20.0f, -20.0f}, {50.0f, 100.0f}     },
        { {0.0f, -20.0f}, {130.0f, 100.0f}    },
        { {30.0f, -20.0f}, {150.0f, 100.0f}    },
        { {60.0f, -20.0f}, {180.0f, 100.0f}     },
        { {90.0f, -20.0f}, {140.0f, 100.0f}     },
        { {110.0f, -20.0f}, {90.0f, 100.0f}    },
        { {140.0f, -20.0f}, {45.0f, 100.0f}    } 
    });

     // Right Backswipe (Combo Response II) Hitbox Coordinates
    const Attack STANDARD_BACKSWIPE ( 8, {
        { {-50.0f, -20.0f}, {30.0f, 100.0f}     }, 
        { {-20.0f, -20.0f}, {50.0f, 100.0f}     },
        { {0.0f, -20.0f}, {130.0f, 100.0f}    },
        { {30.0f, -20.0f}, {150.0f, 100.0f}    },
        { {60.0f, -20.0f}, {180.0f, 100.0f}     },
        { {90.0f, -20.0f}, {140.0f, 100.0f}     },
        { {110.0f, -20.0f}, {90.0f, 100.0f}    },
        { {140.0f, -20.0f}, {45.0f, 100.0f}    } 
    });

    


}


namespace comp {

struct AttackStats {

    hbpos::Attack ilk_id;

    int attack_frame = 0;
    bool delete_flag = false;
    float tick_time = 0.33f;
    float accumulator = 0.0f;

    AttackStats (const hbpos::Attack& attack) {
        ilk_id = attack;
    }


    void tick_one_frame () {
        
        accumulator += tick_time;

        if (accumulator >= tick_time) {
            attack_frame++;
            accumulator -= tick_time;
        }

        if (attack_frame >= ilk_id.frame_count) {
            delete_flag = true;
        }
    }
    




};

}

namespace tag {

    struct AttackDataNode {

    };

};

#endif