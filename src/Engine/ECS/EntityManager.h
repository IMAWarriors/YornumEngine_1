// EntityManager.h


#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "Entity.h"

#include <vector>
#include <iostream>

class EntityManager {

    private:

        std::vector<uint32_t> generations;
        std::vector<uint32_t> free_indicies;

        
        
    public:
        
        Entity create();
        void destroy(Entity _entity);

        bool contains(Entity _entity) const;

        // friend void debugPrint(const EntityManager & _em);
        
        uint32_t get_gen(uint32_t _idx) const;


};






#endif





