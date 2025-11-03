#pragma once
#include<functional>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<queue>
#include<memory>
#include<algorithm>
#include<cassert>
#include<bitset>

namespace sl
{
	constexpr size_t MAX_COMPONENTS = 512;
	using EntityId = uint32_t;
	using ArchetypeMask = std::bitset<MAX_COMPONENTS>;
	using ComponentId = uint32_t;
	using SystemId = uint32_t;
	using EventId = uint32_t;

	class EventBus
	{
	private:
		struct EventQueueBase
		{
			EventQueueBase() = default;
			virtual void Dispatch() = 0;
		};

		template<typename EventType>
		struct EventQueue : public EventQueueBase
		{
			void Dispatch() override;
			std::queue<EventType> events;
			std::vector<std::function<void(EventType&)>> subscribers;
		};

	private:
		EventId GenerateEventId();
		template<typename EventType>
		EventId GetEventId();
	public:
		void DispatchAll();
		template<typename EventType>
		void Dispatch();
		template<typename EventType>
		void Subscribe(std::function<void(EventType& callback)>);
		template<typename EventType>
		void Emit(const EventType& event);
		template<typename EventType>
		EventQueue<EventType>* GetEventQueue();
		void Clear();
	public:
		std::unordered_map<EventId, std::unique_ptr<EventQueueBase>> events;
	};

	class System;
	class Scene
	{
	private:
		struct ComponentArrayBase
		{
			virtual ~ComponentArrayBase() = default;
			virtual void Resize(size_t newSize) = 0;
			virtual void Swap(size_t index1, size_t index2) = 0;
			virtual void PopBack() = 0;
			virtual void PushBack(void* src) = 0;
			virtual void Insert(void* src, size_t index) = 0;
			virtual void Remove(size_t index) = 0;
			virtual size_t Size() = 0;
			virtual std::unique_ptr<ComponentArrayBase> CloneEmpty() const = 0;
			virtual void CopyFrom(size_t destIndex, ComponentArrayBase* src, size_t srcIndex) = 0;
		};
		template<typename T>
		struct ComponentArray : ComponentArrayBase
		{
			void Resize(size_t newSize) override;
			void Swap(size_t index1, size_t index2) override;
			void PopBack() override;
			void PushBack(void* src) override;
			void Insert(void* src, size_t index) override;
			void Remove(size_t index) override;
			size_t Size() override;
			std::unique_ptr<ComponentArrayBase> CloneEmpty() const override;
			void CopyFrom(size_t destIndex, ComponentArrayBase* src, size_t srcIndex) override;
			std::vector<T> components;
		};

		class Archetype
		{
		public:
			Archetype() = default;
			Archetype(const Archetype& other);
			template<typename Component>
			void AddComponent(ComponentId componentId);
			void RemoveComponent(ComponentId componentId);

			~Archetype() = default;
			void AddEntity(EntityId entity);
			void RemoveEntity(EntityId entity);
			bool Empty() const;
		public:
			std::unordered_map<ComponentId, size_t> componentIdToIndex;
			std::vector<EntityId> indexToEntityId;
			std::unordered_map<EntityId, size_t> entityIdToIndex;
			std::vector<ComponentId> indexToComponentId;
			std::vector<std::unique_ptr<ComponentArrayBase>> components;
		};
	public:
		Scene() = default;
		EntityId CreateEntity();
		template<typename Component>
		void AddComponent(EntityId entity, Component&& comp);
		template<typename SystemType>
		void RegisterSystem();
		template<typename ...ComponentTypes>
		void RegisterArchetype();

		template<typename T>
		ComponentId GetComponentId();
		template<typename T>
		SystemId GetSystemId();
		template<typename ...Types>
		ArchetypeMask GetArchetypeMask();
		template<typename Component>
		Component& GetComponent(EntityId entity);
		bool IsEntityValid(EntityId entity) const;
		bool IsEntityAlive(EntityId entity) const;
		template<typename Component>
		bool HasComponent(EntityId entity);
		void MoveEntity(EntityId entity, const ArchetypeMask& newMask);
		template<typename ...Components, typename Func>
		void ForEach(Func&& func);
		void RunSystems(float dt);
		EventBus& GetEventBus();
		template<typename SystemType>
		void SuspendSystem();
		template<typename SystemType>
		void UnsuspendSystem();

		void DestroyEntity(EntityId entity);
		template<typename Component>
		void RemoveComponent(EntityId entity);
		template<typename SystemType>
		void UnregisterSystem();
		template<typename ...ComponentTypes>
		void UnregisterArchetype();
		void Clear();
	private:
		SystemId GenerateSystemId();
		ComponentId GenerateComponentId();
	public:
		std::unordered_map<EntityId, ArchetypeMask> entitiesToMask;
		std::priority_queue<EntityId, std::vector<EntityId>, std::greater<EntityId>> availableId;
		std::unordered_map<ArchetypeMask, std::unique_ptr<Archetype>> archetypes;
		std::unordered_set<ArchetypeMask> userCreatedArchetypes;
		std::unordered_map<SystemId, std::unique_ptr<System>> systems;
		std::vector<System*> systemOrder;
		std::unordered_set<System*> suspendedSystems;
		EventBus eventBus;
		std::unordered_set<EntityId> entitiesToBeDestroyed;
	};

	class System
	{
	public:
		virtual ~System() = default;
		virtual void Run(float dt, Scene& scene) = 0;
	};

	class EntityComponentSystem
	{
	public:
		EntityComponentSystem() = default;
		~EntityComponentSystem() = default;

		Scene* GetCurrentScene();
		Scene* GetScene(const std::string& sceneName);
		void SetCurrentScene(const std::string& sceneName);
		void CreateScene(const std::string& sceneName);
		void RemoveScene(const std::string& sceneName);
		void Clear();
	private:
		std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
		Scene* currentScene = nullptr;
	};

	template<typename Component>
	inline void Scene::AddComponent(EntityId entity, Component&& comp)
	{
		assert(entitiesToMask.contains(entity));
		ArchetypeMask currentMask = entitiesToMask[entity];

		if (currentMask.test(GetComponentId<Component>())) return;

		ArchetypeMask newMask = currentMask;
		newMask.set(GetComponentId<Component>());
		Archetype* newArchetype = archetypes[newMask].get();

		if (!newArchetype)
		{
			if (archetypes[currentMask])
			{
				archetypes[newMask] = std::make_unique<Archetype>(*archetypes[currentMask]);
			}
			else
			{
				archetypes[newMask] = std::make_unique<Archetype>();
			}

			newArchetype = archetypes[newMask].get();
			newArchetype->AddComponent<Component>(GetComponentId<Component>());
		}

		assert(newArchetype);
		MoveEntity(entity, newMask);

		GetComponent<Component>(entity) = std::forward<Component>(comp);
	}

	template<typename SystemType>
	inline void Scene::RegisterSystem()
	{
		systems[GetSystemId<SystemType>()] = std::make_unique<SystemType>();
		systemOrder.push_back(systems[GetSystemId<SystemType>()].get());
	}

	template<typename ...ComponentTypes>
	inline void Scene::RegisterArchetype()
	{
		ArchetypeMask mask = GetArchetypeMask<ComponentTypes...>();
		if (archetypes.contains(mask)) return;

		std::unique_ptr<Archetype> archetype = std::make_unique<Archetype>();
		((archetype->AddComponent<ComponentTypes>(GetComponentId<ComponentTypes>())), ...);
		archetypes[mask] = std::move(archetype);

		userCreatedArchetypes.insert(mask);
	}

	template<typename Component>
	inline Component& Scene::GetComponent(EntityId entity)
	{
		assert(IsEntityValid(entity));
		assert(HasComponent<Component>(entity));
		Archetype* arch = archetypes[entitiesToMask[entity]].get();
		assert(arch);
		size_t compIdx = arch->componentIdToIndex[GetComponentId<Component>()];
		assert(static_cast<ComponentArray<Component>*>(arch->components[compIdx].get())->Size() > arch->entityIdToIndex[entity]);
		return static_cast<ComponentArray<Component>*>(arch->components[compIdx].get())->components[arch->entityIdToIndex[entity]];
	}

	template<typename Component>
	inline bool Scene::HasComponent(EntityId entity)
	{
		ArchetypeMask mask = entitiesToMask[entity];
		return mask.test(GetComponentId<Component>());
	}

	template<typename... Components, typename Func>
	void Scene::ForEach(Func&& func)
	{
		ArchetypeMask desiredMask = GetArchetypeMask<Components...>();

		for (auto& [mask, archPtr] : archetypes)
		{
			if ((mask & desiredMask) != desiredMask) continue;

			Archetype* arch = archPtr.get();
			size_t entityCount = arch->indexToEntityId.size();

			for (size_t i = 0; i < entityCount; i++)
			{
				EntityId entity = arch->indexToEntityId[i];

				func(entity, GetComponent<Components>(entity)...);
			}
		}
	}

	template<typename SystemType>
	inline void Scene::SuspendSystem()
	{
		suspendedSystems.insert(systems[GetSystemId<SystemType>()].get());
	}

	template<typename SystemType>
	inline void Scene::UnsuspendSystem()
	{
		SystemId id = GetSystemId<SystemType>();
		assert(suspendedSystems.contains(id));
		suspendedSystems.erase(id);
	}

	template<typename Component>
	inline void Scene::RemoveComponent(EntityId entity)
	{
		assert(IsEntityValid(entity));
		assert(HasComponent<Component>(entity));
		ArchetypeMask currentMask = entitiesToMask[entity];
		ArchetypeMask newMask = currentMask;
		newMask.reset(GetComponentId<Component>());

		Archetype* newArchetype = archetypes[newMask].get();

		if (!newArchetype)
		{
			if (archetypes[currentMask])
			{
				archetypes[newMask] = std::make_unique<Archetype>(*archetypes[currentMask]);
			}
			else
			{
				archetypes[newMask] = std::make_unique<Archetype>();
			}
			newArchetype = archetypes[newMask].get();
			newArchetype->RemoveComponent(GetComponentId<Component>());
		}

		MoveEntity(entity, newMask);
	}

	template<typename SystemType>
	inline void Scene::UnregisterSystem()
	{
		SystemId id = GetSystemId<SystemType>();
		assert(systems.contains(id));
		if (suspendedSystems.contains(id)) suspendedSystems.erase(id);
		auto it = systems.find(id);
		System* ptr = it->second.get();
		auto vecIt = std::find(systemOrder.begin(), systemOrder.end(), ptr);
		suspendedSystems.erase(ptr);
		systems.erase(it);
	}

	template<typename ...ComponentTypes>
	inline void Scene::UnregisterArchetype()
	{
		ArchetypeMask mask = GetArchetypeMask<ComponentTypes...>();
		assert(archetypes.contains(mask) && userCreatedArchetypes.contains(mask));
		assert(archetypes[mask].get()->Empty());
		archetypes.erase(mask);
		userCreatedArchetypes.erase(mask);
	}

	template<typename Component>
	inline void Scene::Archetype::AddComponent(ComponentId componentId)
	{
		size_t index = components.size();
		components.push_back(std::make_unique<ComponentArray<Component>>());
		componentIdToIndex[componentId] = index;
		indexToComponentId.push_back(componentId);
		components[index]->Resize(entityIdToIndex.size());
	}

	template<typename T>
	inline ComponentId Scene::GetComponentId()
	{
		static ComponentId id = GenerateComponentId();
		return id;
	}

	template<typename T>
	inline SystemId Scene::GetSystemId()
	{
		static SystemId id = GenerateSystemId();
		return id;
	}

	template<typename ...Types>
	inline ArchetypeMask Scene::GetArchetypeMask()
	{
		ArchetypeMask mask;
		(mask.set(GetComponentId<Types>()), ...);
		return mask;
	}

	template<typename T>
	inline void Scene::ComponentArray<T>::Resize(size_t newSize)
	{
		components.resize(newSize);
	}

	template<typename T>
	inline void Scene::ComponentArray<T>::Swap(size_t index1, size_t index2)
	{
		assert(index1 < components.size() && index2 < components.size());
		std::swap(components[index1], components[index2]);
	}

	template<typename T>
	inline void Scene::ComponentArray<T>::PopBack()
	{
		assert(!components.empty());
		components.pop_back();
	}

	template<typename T>
	inline void Scene::ComponentArray<T>::PushBack(void* src)
	{
		assert(src);
		components.push_back(*static_cast<T*>(src));
	}

	template<typename T>
	inline void Scene::ComponentArray<T>::Insert(void* src, size_t index)
	{
		assert(src);
		components[index] = *static_cast<T*>(src);
	}

	template<typename T>
	inline void Scene::ComponentArray<T>::Remove(size_t index)
	{
		assert(index < components.size());
		if (index != components.size() - 1) components[index] = components.back();
		components.pop_back();
	}

	template<typename T>
	inline size_t Scene::ComponentArray<T>::Size()
	{
		return components.size();
	}

	template<typename T>
	inline std::unique_ptr<Scene::ComponentArrayBase> Scene::ComponentArray<T>::CloneEmpty() const
	{
		return std::make_unique<ComponentArray<T>>();
	}

	template<typename T>
	inline void Scene::ComponentArray<T>::CopyFrom(size_t destIndex, ComponentArrayBase* src, size_t srcIndex)
	{
		assert(src);
		assert(destIndex < components.size() && srcIndex < src->Size());
		components[destIndex] = static_cast<ComponentArray<T>*>(src)->components[srcIndex];
	}

	template<typename EventType>
	inline EventId EventBus::GetEventId()
	{
		static EventId id = GenerateEventId();
		return id;
	}

	template<typename EventType>
	inline void EventBus::Dispatch()
	{
		EventId id = GetEventId<EventType>();
		events[id].get()->Dispatch();
	}

	template<typename EventType>
	inline void EventBus::Subscribe(std::function<void(EventType&)> callback)
	{
		EventId id = GetEventId<EventType>();
		if (!events.contains(id))
		{
			events[id] = std::make_unique<EventQueue<EventType>>();
		}

		auto queue = static_cast<EventQueue<EventType>*>(events[id].get());
		queue->subscribers.push_back(std::move(callback));
	}

	template<typename EventType>
	inline void EventBus::Emit(const EventType& event)
	{
		EventId id = GetEventId<EventType>();
		assert(events.contains(id));
		static_cast<EventQueue<EventType>*>(events[id].get())->events.push(event);
	}

	template<typename EventType>
	inline EventBus::EventQueue<EventType>* EventBus::GetEventQueue()
	{
		EventId id = GetEventId<EventType>();
		assert(events.contains(id));
		return events[id].get();
	}

	template<typename EventType>
	inline void EventBus::EventQueue<EventType>::Dispatch()
	{
		while (!events.empty())
		{
			EventType event = events.front();
			events.pop();

			for (auto& func : subscribers)
			{
				func(event);
			}
		}
	}
}