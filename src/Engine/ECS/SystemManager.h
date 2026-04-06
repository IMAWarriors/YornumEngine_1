// SystemManager.h

#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include "System.h"
#include "Enum_SystemPhases.h"

#include <vector>
#include <memory>
#include <string>
#include <array>

class SystemManager {

    private:
        
        std::array< std::vector<std::unique_ptr<System>>, (size_t)(Phases::SIZE)> systems;

    
    public:

        template<typename T, typename... Args> T* add_system(Phases phase, Args&&... args) {
            T * system = new T(std::forward<Args>(args)...);
            systems[(int)phase].emplace_back(system);
            return system;
        }



        void execute_phase (Phases phase, Registry & registry, float deltatime) {

            for (auto & system : systems[(int)phase]) {

                system->update(registry, deltatime);
                
            }

        }
};









#endif 
