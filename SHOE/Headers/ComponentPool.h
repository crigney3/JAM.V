#pragma once
#include <memory>
#include <queue>

#include "GameEntity.fwd.h"
#include "MeshRenderer.h"
#include "Light.h"

constexpr auto POOL_SIZE = 32;

template <typename T>
class ComponentPool
{
public:
	static std::shared_ptr<T> Instantiate(std::shared_ptr<GameEntity> gameEntity);
	static void Free(std::shared_ptr<IComponent> component);
	static int GetActiveCount();
	static std::vector<std::shared_ptr<T>> GetAll();
	static std::vector<std::shared_ptr<T>> GetAllEnabled();
	static void Sort() {};

private:
	static std::vector<std::shared_ptr<T>> allocated;
	static std::queue<std::shared_ptr<T>> unallocated;
};

template <> void ComponentPool<MeshRenderer>::Sort();
template <> void ComponentPool<Light>::Sort();

template<typename T>
std::vector<std::shared_ptr<T>> ComponentPool<T>::allocated = std::vector<std::shared_ptr<T>>();

template<typename T>
std::queue<std::shared_ptr<T>> ComponentPool<T>::unallocated = std::queue<std::shared_ptr<T>>();

/**
 * \brief Binds an unallocated component from the pool to a GameEntity
 * \param gameEntity The GameEntity the component is to be attached to
 * \param hierarchyIsEnabled Whether the GameEntity to be attached to is enabled
 * \return A reference to the newly bound component
 */
template<typename T>
std::shared_ptr<T> ComponentPool<T>::Instantiate(std::shared_ptr<GameEntity> gameEntity)
{
	//Allocates a new pool if there is no available components
	if (unallocated.size() == 0) {
		for (int i = 0; i < POOL_SIZE; i++) {
			unallocated.push(std::make_shared<T>());
		}
	}

	std::shared_ptr<T> component = unallocated.front();
	allocated.emplace_back(component);
	unallocated.pop();
	component->Bind(gameEntity);
	Sort();
	return component;
}

/**
 * \brief Unbinds a given component and marks it free for use
 * \param component The component to unbind
 */
template<typename T>
void ComponentPool<T>::Free(std::shared_ptr<IComponent> component)
{
	component->Free();
	unallocated.push(std::dynamic_pointer_cast<T>(component));
	allocated.erase(std::remove(allocated.begin(), allocated.end(), std::dynamic_pointer_cast<T>(component)), allocated.end());
}

/**
 * \brief Gives the amount of components of this type currently bound
 * \return Total amount of bound components from this pool
 */
template<typename T>
int ComponentPool<T>::GetActiveCount()
{
	return allocated.size();
}

/**
 * \brief Returns a vector of all currently bound components in the pool
 */
template<typename T>
std::vector<std::shared_ptr<T>> ComponentPool<T>::GetAll()
{
	return allocated;
}

/**
 * \brief Returns a vector of all currently bound and enabled components in the pool
 */
template<typename T>
std::vector<std::shared_ptr<T>> ComponentPool<T>::GetAllEnabled()
{
	std::vector<std::shared_ptr<T>> enabled = std::vector<std::shared_ptr<T>>();
	for(std::shared_ptr<T> component : allocated)
	{
		if(component->IsEnabled())
		{
			enabled.emplace_back(component);
		}
	}
	return enabled;
}

template <> void ComponentPool<MeshRenderer>::Sort()
{
	std::sort(allocated.begin(), allocated.end(), [](std::shared_ptr<MeshRenderer> a, std::shared_ptr<MeshRenderer> b) {
		if (a->GetMaterial()->GetTransparent() != b->GetMaterial()->GetTransparent())
			return b->GetMaterial()->GetTransparent();
		return a->GetMaterial() < b->GetMaterial();
		});
}

template <> void ComponentPool<Light>::Sort()
{
	std::sort(allocated.begin(), allocated.end(), [](std::shared_ptr<Light> a, std::shared_ptr<Light> b) {
		return a->GetType() > b->GetType();
		});
}
