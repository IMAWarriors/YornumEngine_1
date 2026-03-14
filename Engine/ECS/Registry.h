// Registry.h

#ifndef REGISTRY_H
#define REGISTRY_H

#include "ComponentPool.h"
#include "IPool.h"
#include "EntityManager.h"


#include <cstdint>
#include <vector>
#include <stdexcept>

template<typename... T_ComponentTypeArgs> class View;

class Registry {

    private:

        size_t iPENDING_COMPONENT_INDEX = 0;




        EntityManager entity_manager;               // Manages specific entities for ALL component pools

        std::vector<IPool*> component_pools;        // Components owned by specific entities

        bool is_valid(Entity _entity);

    public:
    

        // ENTITY CREATION FUNCTIONS

        Registry() = default;

        Registry(const Registry&) = delete;

        Registry& operator=(const Registry&) = delete;

        Entity create_entity (); 

        void destroy_entity (Entity _entity);

        // COMPONENT ASSIGNMENT FUNCTIONS

        template <typename T_ComponentType> int get_component_pool_index () {

            /*IMPOTANT STATIC ALERT*/ static int i_CONST_MY_COMPNENT_INDEX = iPENDING_COMPONENT_INDEX++;
            // THIS VARIABLE MUST ONLY BE DECLARED ONCE THE FIRST TIME THE TYPENAME IS REFERENCED
            // AS DECLARING THIS VARIABLE FOR ITSELF NOT ONLY IS RENDERED USELESS IF REASSIGNED FOR A COMP POOL,
            // BUT ALSO CALLING THE DECLARATION CAUSES THE GLOBAL NEXT COMPONENT INDEX TO ADVANCE UNDESIRABLY

            return i_CONST_MY_COMPNENT_INDEX; // Main action of this function, above is only called first time template type defined
        }

        template <typename T_ComponentType> bool has_component (Entity _entity) {

            if (!entity_manager.contains(_entity)) {
                return false;
            }

            int component_pool_index = get_component_pool_index<T_ComponentType>();

            if (component_pools.size() <= component_pool_index) {
                 return false; 
                };

            if (component_pools[component_pool_index] != nullptr) {

                 ComponentPool<T_ComponentType> * component_pool_ptr = static_cast<ComponentPool<T_ComponentType>*>(component_pools[component_pool_index]);
                return component_pool_ptr->has(_entity);

            } else {
                return false;
            }
        }

        template <typename T_ComponentType> void apply_component (Entity _entity, T_ComponentType _component) {

            if (!entity_manager.contains(_entity)) {
                return;
            }

            int component_pool_index = get_component_pool_index<T_ComponentType>();
            
            if (component_pool_index >= component_pools.size()) {
                component_pools.resize(component_pool_index + 1, nullptr);
            }

            if (component_pools[component_pool_index] == nullptr) {
                component_pools[component_pool_index] = new ComponentPool<T_ComponentType>();
            }

            ComponentPool<T_ComponentType> * component_pool_ptr = static_cast<ComponentPool<T_ComponentType>*>(component_pools[component_pool_index]);
            
            component_pool_ptr->apply(_entity, _component);
        }

        template <typename T_ComponentType> void remove_component (Entity _entity) {

            if (!entity_manager.contains(_entity)) {
                return;
            }

            int component_pool_index = get_component_pool_index<T_ComponentType>();

            if (component_pool_index >= component_pools.size()) {
                return;
            }

            if (component_pools[component_pool_index] == nullptr) {
                return;
            }

            ComponentPool<T_ComponentType> * component_pool_ptr = static_cast<ComponentPool<T_ComponentType>*>(component_pools[component_pool_index]);

            if (component_pool_ptr->has(_entity)) {
                component_pool_ptr->detach(_entity);
            }

            
        }

        template <typename T_ComponentType> T_ComponentType & get_component (Entity _entity) {

            if (!entity_manager.contains(_entity)) {
                throw std::runtime_error("ERROR: TRIED TO REFERENCE COMPONENT FROM ENTITY THAT DOES NOT EXIST");
            }

            int component_pool_index = get_component_pool_index<T_ComponentType>();

            if (component_pool_index >= component_pools.size()) {
                throw std::runtime_error("ERROR: TRIED TO REFERENCE A COMPONENT POOL FROM AN INDEX WHOSE POOL HAS NOT YET BEEN INITIALIZED");
            }

            ComponentPool<T_ComponentType> * component_pool_ptr = static_cast<ComponentPool<T_ComponentType>*>(component_pools[component_pool_index]);

            if (!component_pool_ptr->has(_entity)) {
                throw std::runtime_error("ERROR: REFERENCED ENTITY DOES NOT HAVE A COMPONENT OF THE TYPE SPECIFIED");
            }

            return component_pool_ptr->get(_entity);

        }

        template <typename T_ComponentType> ComponentPool<T_ComponentType> * get_component_pool () {

            int component_pools_index = get_component_pool_index<T_ComponentType>();

            if (component_pools_index >= component_pools.size()) {
                return nullptr;
            }
            
            return static_cast<ComponentPool<T_ComponentType>*>(component_pools[component_pools_index]);

        }

        template<typename... T_ComponentTypeArgs> View<T_ComponentTypeArgs...> view();

        

        

        // Standard memory leak fix
        ~Registry () {
            for (IPool* pool : component_pools) {
                delete pool;
            }
        }



};




// TO AVOID COMPILER WARNINGS 
#include "View.h"

template<typename... T_ComponentTypeArgs> View<T_ComponentTypeArgs...> Registry::view() {
    return View<T_ComponentTypeArgs...>(this);
}

#endif

