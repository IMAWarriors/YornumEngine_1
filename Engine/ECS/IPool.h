// IPool.h

#ifndef IPOOL_H
#define IPOOL_H

#include "Entity.h"


class IPool {           // abstract class for all ComponentPool<ComponentType> form which
                        // each individual componenent type is a derived class of this IPool


    public:

        virtual ~IPool () = default;

        virtual void remove_entity_refs (Entity _entity) = 0;


};


#endif
