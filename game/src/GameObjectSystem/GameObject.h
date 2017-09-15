/*
FILE: GameObject.h
PRIMARY AUTHOR: Kieran

Copyright � 2017 DigiPen (USA) Corporation.
*/
#pragma once

class GameSpace;

template <typename T>
class ComponentHandle;


typedef size_t GameObject_ID;
typedef int    dummy;

class GameObject
{
public:
	GameObject(GameObject_ID id, GameSpace *gameSpace) : m_objID(id), m_gameSpace(gameSpace)
	{
	}

	template <typename T, typename... Args>
	void addComponent(Args&&... args)
	{
		m_gameSpace->emplaceComponent<T>(m_objID, std::forward<Args>(args)...);
	}

	template <typename T>
	ComponentHandle<T> getComponent()
	{
		// Make sure the component exists before we hand it off.
		if (m_gameSpace->getInternalComponent<T>(m_objID) != nullptr)
		{
			// Why the hell does constructing a ComponentHandle work?
			// We never forward declare the constructor and we shouldn't know what sizeof(ComponentHandle<T>) is, right? -Kieran
			return ComponentHandle<T>(m_objID, m_gameSpace);
		}
		else
		{
			return ComponentHandle<T>(0, nullptr, false);
		}
	}

	static GameObject_ID GenerateID()
	{
		static GameObject_ID lastGeneratedID = 0;

		return lastGeneratedID++;
	}

	GameObject_ID Getid() const
	{
		return m_objID;
	}

	// Dummy Template param since GameSpace are forward delcared here
	template <typename dummy>
	GameObject Duplicate()
	{
		return m_gameSpace->Duplicate(m_objID, m_gameSpace->NewGameObject().m_objID);
	}

private:
	GameObject_ID m_objID;
	GameSpace *m_gameSpace;
};
