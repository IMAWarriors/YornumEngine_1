// System.h

#ifndef SYSTEM_H
#define SYSTEM_H

#include "Registry.h"

class System {

    public:
    
        virtual ~System () = default;

        virtual void update (Registry & _registry, float _deltatime) = 0;


};








#endif
