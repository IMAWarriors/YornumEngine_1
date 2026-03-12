// EntityManager.cpp

#include "EntityManager.h"



Entity EntityManager::create() {

    uint32_t id;
    uint32_t gen;

    if (free_indicies.size() > 0) {      // REUSE OLD MEM SLOT

        id = free_indicies.back();      // The target EntityID for the new Entity will be the last free index
        free_indicies.pop_back();       // The last element of free indicies is removed, making it no longer free
        gen = generations[id];

    } else {                            // MAKE NEW MEM SLOT

        gen = 0;

        id = generations.size();
        generations.push_back(0);
        

    }


    return Entity({id, gen});


}


void EntityManager::destroy(Entity _entity) {

    if (_entity.ID >= generations.size()) {
        return;
    }
    
    if (_entity.generation != generations[_entity.ID]) {
        return;
    }


    generations[_entity.ID]++;
    free_indicies.push_back(_entity.ID);
    

}

bool EntityManager::contains(Entity _entity) const {

    if (_entity.ID >= generations.size()) {
        return false;
    }

    if (generations[_entity.ID] == _entity.generation) {
        return true;
    } else {
        return false;
    }

}



uint32_t EntityManager::get_gen(uint32_t _idx) const {
    if (_idx >= generations.size()) { return generations[generations.size() - 1]; }
    return generations[_idx];
}

