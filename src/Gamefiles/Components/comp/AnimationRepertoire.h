// AnimationRepertoire.h

#ifndef ANIMATIONREPERTOIRE_H
#define ANIMATIONREPERTOIRE_H



#include "../../../Engine/Core/Overhead/GameTypes.h"

#include "../../Elements/Avatar/Avatar.h"
#include "../../Elements/Avatar/Animation.h"

#include <unordered_map>

namespace comp {


struct AnimationRepertoire {

    std::unordered_map<std::string, Animation*> repertoire;

    AnimationRepertoire () {

    }

    void ImportAnimationAsset (std::string name, Animation* animation) {
        repertoire.emplace(name, animation);
    }


};



}

#endif