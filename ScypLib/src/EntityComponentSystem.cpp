#include "ScypLib/EntityComponentSystem.h"

namespace sl
{
    Scene* EntityComponentSystem::GetCurrentScene()
    {
        return currentScene;
    }

    Scene* EntityComponentSystem::GetScene(const std::string& sceneName)
    {
        return scenes[sceneName].get();
    }

    void EntityComponentSystem::SetCurrentScene(const std::string& sceneName)
    {
        assert(scenes.contains(sceneName));
        currentScene = scenes[sceneName].get();
    }

    void EntityComponentSystem::CreateScene(const std::string& sceneName)
    {
        bool wasEmpty = scenes.empty();
        scenes[sceneName] = std::make_unique<Scene>();
        if (wasEmpty) currentScene = scenes[sceneName].get();
    }

    void EntityComponentSystem::RemoveScene(const std::string& sceneName)
    {
        assert(scenes.contains(sceneName));
        if (currentScene == scenes[sceneName].get()) currentScene = nullptr;
        scenes.erase(sceneName);
    }

    void EntityComponentSystem::Clear()
    {
        currentScene = nullptr;
        scenes.clear();
    }

    EntityId Scene::CreateEntity()
    {
        EntityId ent;

        if (!availableId.empty())
        {
            ent = availableId.top();
            availableId.pop();
        }
        else
        {
            ent = EntityId(entitiesToMask.size());
        }
        entitiesToMask[ent].reset();
        if (!archetypes.contains(0)) archetypes[0] = std::make_unique<Archetype>();
        archetypes[0].get()->AddEntity(ent);
        assert(archetypes[0].get()->entityIdToIndex.contains(ent));
        return ent;
    }

    bool Scene::IsEntityValid(EntityId entity) const
    {
        return entitiesToMask.contains(entity);
    }

    void Scene::MoveEntity(EntityId entity, const ArchetypeMask& newMask)
    {
        if (archetypes[newMask].get() && archetypes[newMask].get()->entityIdToIndex.contains(entity)) return;
        ArchetypeMask currentMask = entitiesToMask[entity];
        Archetype* newArchetype = archetypes[newMask].get();
        assert(newArchetype);
        assert(currentMask == entitiesToMask[entity]);

        newArchetype->AddEntity(entity);
        size_t newIndex = newArchetype->entityIdToIndex[entity];

        Archetype* currentArchetype = archetypes[currentMask].get();
        assert(currentArchetype);
        if (currentMask != 0)
        {
            size_t currentIndex = currentArchetype->entityIdToIndex[entity];

            for (auto& [componentId, idx] : currentArchetype->componentIdToIndex)
            {
                if (newArchetype->componentIdToIndex.contains(componentId))
                {
                    newArchetype->components[newArchetype->componentIdToIndex[componentId]].get()->CopyFrom(newIndex,
                        currentArchetype->components[idx].get(), currentIndex);
                }
            }
        }
        currentArchetype->RemoveEntity(entity);
        if (currentArchetype->Empty() && !userCreatedArchetypes.contains(currentMask)) archetypes.erase(currentMask);
        entitiesToMask[entity] = newMask;
    }

    void Scene::RunSystems(float dt)
    {
        for (System* system : systemOrder)
        {
            if(!suspendedSystems.contains(system)) system->Run(dt, *this);
        }
        for (EntityId id : entitiesToBeDestroyed)
        {
            archetypes[entitiesToMask[id]].get()->RemoveEntity(id);
            entitiesToMask.erase(id);
            availableId.push(id);
        }
    }

    EventBus& Scene::GetEventBus()
    {
        return eventBus;
    }

    void Scene::DestroyEntity(EntityId entity)
    {
        entitiesToBeDestroyed.push_back(entity);
    }

    ComponentId Scene::GenerateComponentId()
    {
        static ComponentId id = 0;
        return id++;
    }

    void Scene::Clear()
    {
        entitiesToMask.clear();
        while (!availableId.empty()) availableId.pop();
        archetypes.clear();
        userCreatedArchetypes.clear();
        systems.clear();
    }

    SystemId Scene::GenerateSystemId()
    {
        static SystemId id = 0;
        return id++;
    }

    Scene::Archetype::Archetype(const Archetype& other)
    {
        componentIdToIndex = other.componentIdToIndex;
        indexToComponentId = other.indexToComponentId;
        for (auto& comp : other.components)
        {
            components.push_back(comp.get()->CloneEmpty());
        }
    }

    void Scene::Archetype::RemoveComponent(ComponentId componentId)
    {
        size_t index = componentIdToIndex[componentId];
        size_t lastIndex = componentIdToIndex.size() - 1;
        if (index != lastIndex)
        {
            std::swap(components[index], components[lastIndex]);
            ComponentId swappedId = indexToComponentId[lastIndex];
            indexToComponentId[index] = swappedId;
            componentIdToIndex[swappedId] = index;
        }
        components.pop_back();
        indexToComponentId.pop_back();
        componentIdToIndex.erase(componentId);
    }

    void Scene::Archetype::AddEntity(EntityId entity)
    {
        size_t index = entityIdToIndex.size();
        entityIdToIndex[entity] = index;
        indexToEntityId.push_back(entity);

        for (auto& c : components)
        {
            c->Resize(index + 1);
            assert(c->Size() != 0);
        }
    }

    void Scene::Archetype::RemoveEntity(EntityId entity)
    {
        assert(entityIdToIndex.contains(entity));
        size_t index = entityIdToIndex[entity];
        size_t lastIndex = entityIdToIndex.size() - 1;
        EntityId lastEntity = indexToEntityId[lastIndex];

        for (auto& comp : components)
        {
            comp.get()->Remove(index);
        }

        if (index != lastIndex)
        {
            entityIdToIndex[lastEntity] = index;
            indexToEntityId[index] = lastEntity;
        }

        entityIdToIndex.erase(entity);
        indexToEntityId.pop_back();
    }

    bool Scene::Archetype::Empty() const
    {
        return entityIdToIndex.empty();
    }
    EventId EventBus::GenerateEventId()
    {
        static EventId id = 0;
        return id++;
    }
    void EventBus::DispatchAll()
    {
        for (auto& [id, eventQ] : events)
        {
            eventQ->Dispatch();
        }
    }
    void EventBus::Clear()
    {
        events.clear();
    }
}