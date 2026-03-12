// SystemManager.h

#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include "System.h"

#include <vector>
#include <memory>

class SystemManager {

    private:
        
        std::vector<std::unique_ptr<System>> systems;

    
    public:

        template<typename T, typename... Args> T* add_system(Args&&... args) {

            T * system = new T(std::forward<Args>(args)...);

            systems.emplace_back(system);

            return system;
        }

        void update_all(Registry & registry, float deltatime) {

            for (auto & system : systems) {

                system->update(registry, deltatime);
                
            }

        }



};









#endif 
