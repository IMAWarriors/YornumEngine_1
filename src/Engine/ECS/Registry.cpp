// Registry.cpp

#include "Registry.h"



Entity Registry::create_entity () {
    return entity_manager.create();
}

void Registry::destroy_entity (Entity _entity) {

    for (IPool * pool : component_pools) {

        if (pool) { pool->remove_entity_refs(_entity); }

    }

    entity_manager.destroy(_entity);

}



bool Registry::is_valid(Entity _entity) {
    return entity_manager.get_gen(_entity.ID) == _entity.generation;
}