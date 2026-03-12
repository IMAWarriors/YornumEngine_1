// View.h

#ifndef VIEW_H
#define VIEW_H

class Registry;

#include "ComponentPool.h"

#include <tuple>
#include <limits>
#include <utility>

template<typename... T_ComponentTypeArgs>
class View {

private:

    Registry* registry;

    std::tuple<ComponentPool<T_ComponentTypeArgs>*...> pools;

    const std::vector<Entity>* driving_entities;

    // Check if entity has ALL components
    bool entity_valid(Entity e) const {
        return (registry->template has_component<T_ComponentTypeArgs>(e) && ...);
    }

    // Choose the smallest component pool to drive iteration
    template<size_t... I>
    void select_smallest_pool(std::index_sequence<I...>) {

        size_t smallest_size = std::numeric_limits<size_t>::max();
        const std::vector<Entity>* best = nullptr;

        auto check_pool = [&](auto* pool) {

            if (!pool) return;

            const auto& entities = pool->get_entities();

            if (entities.size() < smallest_size) {
                smallest_size = entities.size();
                best = &entities;
            }

        };

        (check_pool(std::get<I>(pools)), ...);

        if (best)
            driving_entities = best;
        else {
            static const std::vector<Entity> empty;
            driving_entities = &empty;
        }
    }

public:

    View(Registry* _registry)
        : registry(_registry)
    {
        pools = std::make_tuple(
            registry->template get_component_pool<T_ComponentTypeArgs>()...
        );

        select_smallest_pool(
            std::index_sequence_for<T_ComponentTypeArgs...>{}
        );
    }

    class Iterator {

    private:

        const View* view;
        size_t index;

        void skip_invalid() {

            const auto& entities = *view->driving_entities;

            while (index < entities.size() &&
                   !view->entity_valid(entities[index]))
            {
                index++;
            }

        }

    public:

        Iterator(const View* v, size_t i)
            : view(v), index(i)
        {
            skip_invalid();
        }

        bool operator!=(const Iterator& rhs) const {
            return index != rhs.index;
        }

        Iterator& operator++() {
            index++;
            skip_invalid();
            return *this;
        }

        Entity operator*() const {
            return (*view->driving_entities)[index];
        }

    };

    Iterator begin() {
        return Iterator(this, 0);
    }

    Iterator end() {
        return Iterator(this, driving_entities->size());
    }

};

#endif