// ComponenetPool.h

#ifndef COMPONENTPOOL_H
#define COMPONENTPOOL_H

#include "IPool.h"
//#include "../../Gamefiles/Components/Components.h"
#include "EntityManager.h"
#include <cstdint>
#include <vector>
#include <stdexcept>

/*
 *  
 *  
 *  
 * 
 * 
*/






template <typename T_ComponentType> class ComponentPool : public IPool {

    private:

        std::vector<T_ComponentType> components;
        std::vector<Entity> entities;

        std::vector<int32_t> entities_to_index;

    public:

        // Apply new user-defined parameters for component structure to an entity
        void apply (Entity _entity, T_ComponentType _component) {

            if (!has(_entity)) {
                entities.push_back(_entity);
                components.push_back(_component);

                if (_entity.ID >= entities_to_index.size()) {      // If the ID as an index fro input entity is larger than size of entities_to_compoenent directive indexing vector...
                    entities_to_index.resize(_entity.ID + 1, -1);
                }

                entities_to_index[_entity.ID] = (components.size() - 1);
            } else {
                components[entities_to_index[_entity.ID]] = _component;
            }

        }

        // Detatch component instance type from an entity
        void detach(Entity _entity) {

            if (!has(_entity)) { return; }

            uint32_t index = entities_to_index[_entity.ID];
            uint32_t last_index = components.size() - 1;

            Entity moved_entity = entities[last_index];

            components[index] = components[last_index];
            entities[index] = moved_entity;

            entities_to_index[moved_entity.ID] = index;

            components.pop_back();
            entities.pop_back();

            entities_to_index[_entity.ID] = -1;

        }

        void remove_entity_refs (Entity _entity) override { // Delete ALL of this components references to a specific Entity
            detach(_entity);
        }

        // Check if an entity contains an instance of this component type (in other words, if this component pools components has an entry w/ Entity)
        bool has (Entity _entity) const {

            if (_entity.ID >= entities_to_index.size()) { return false; }

            return (entities_to_index[_entity.ID] != -1);

        }
        
        // Return values of specific component data structure tied to an entity
        T_ComponentType & get(Entity _entity) {

            if (!has(_entity)) {
                throw std::runtime_error("ERROR: ENTITY DOES NOT HAVE THIS COMPONENT TYPE");
            }

            return components[entities_to_index[_entity.ID]];

        }

        const std::vector<Entity> & get_entities() const { // READ ONLY
            return entities;
        }

        

};

#endif

