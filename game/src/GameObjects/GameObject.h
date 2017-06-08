/*
FILE: GameObject.h
PRIMARY AUTHOR: Sweet

Entry point into the program.
Currently just a place to play around in.

Copyright � 2017 DigiPen (USA) Corporation.
*/
#pragma once


#include <cstdint>
#include <unordered_map>
#include <map>
#include "Components/Component.h"

// size_t for GameObjectID
typedef size_t GameObjectID_t;

// Defining void * so it is actually something when read
typedef void * Component_Index;
typedef void * Component_Maps;

template <typename T>
struct COMPONENT_GEN
{
	constexpr static Component_Index Func()
	{
		return reinterpret_cast<Component_Index>(Func);
	}
};


template <typename T>
struct Component_Map
{
	std::map<GameObjectID_t, T> mMap;
};



class GameObject_Space
{
public:

	template <typename T>
	void RegisterComponentMap()
	{
		mSpace.emplace(COMPONENT_GEN<T>::Func, new std::map<GameObjectID_t, T>);
	}


	template <typename T>
	void Add(GameObjectID_t id, T & component)
	{
		reinterpret_cast<std::map<GameObjectID_t, T> *>(mSpace.at(COMPONENT_GEN<T>::Func))->emplace(id, component);
	}


	template <typename T>
	T & Find(GameObjectID_t id)
	{
		return reinterpret_cast<std::map<GameObjectID_t, T> *>(mSpace.at(COMPONENT_GEN<T>::Func))->at(id);
	}


	GameObjectID_t AssignID()
	{
		return mCurrentID++;
	}


	// WIP function
	~GameObject_Space()
	{
		for (auto & iter : mSpace)
		{
			// This still leaks
			// delete reinterpret_cast<std::map<GameObjectID_t, decltype(iter.second)> *>(iter.second);
		}
	}


private:
	// Component_Index is static func pointer
	// Component_Maps is the map of the component
	std::unordered_map<Component_Index, Component_Maps> mSpace;

	// The id to assign to the next gameobject
	GameObjectID_t mCurrentID = 0;

};



class GameObject
{
public:

	explicit GameObject(GameObject_Space & space);


	// Gets the ID of the object
	GameObjectID_t id() const;

	GameObject_Space & GetSpace() const;

	// L-value SetComponent
	template <typename T>
	void SetComponent(T & component)
	{
		mContainingSpace.Add<T>(mID, component);
	}

	// R-value SetComponent
	template <typename T>
	void SetComponent(T && component)
	{
		mContainingSpace.Add<T>(mID, component);
	}


	// Adds custom components to an object
	template <typename First, typename ...Rest>
	void SetComponent(First & first, Rest & ...args)
	{
		mContainingSpace.Add<First>(mID, first);
		SetComponent(args...);
	}


	// Adds custom components to an object
	template <typename First, typename ...Rest>
	void SetComponent(First && first, Rest && ...args)
	{
		mContainingSpace.Add<First>(mID, first);
		SetComponent(args...);
	}


	// Adds default components to an object.
	template <typename ...Args>
	void SetComponent()
	{
		using expand = int[];
		expand
		{
			0, (SetComponent(Args()), 0)...
		};
	}


	// GetComponent, will throw NoComponent in Debug if no component was found on an object
	template <typename T>
	T & GetComponent()
	{
		#ifdef _DEBUG
			try
			{
				return mContainingSpace.Find<T>(mID);
			}
			catch(std::out_of_range)
			{
				throw NoComponent("No Component was found for this object.");
			}
		#else
			return mContainingSpace.Find<T>(mID);
		#endif
	}


private:

	// The parent of the game object
	GameObject_Space & mContainingSpace;

	// Class to throw if no object was found
	class NoComponent { public: explicit NoComponent(const char * error) { (void)error; } };

	// ID of the game object
	GameObjectID_t mID = 0;
};









