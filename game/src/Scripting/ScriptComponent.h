/*
FILE: LuaScript.h
PRIMARY AUTHOR: Kieran

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#pragma once

#include "Universal.h"
#include "meta\meta.h"
#include <vector>

#include "LuaScript.h"
struct ScriptComponent
{
	ScriptComponent()
	{
	}

	ScriptComponent(const LuaScript& script)
	{
		scripts.push_back(script);
	}

	void CallCollision(GameObject collidedObj)
	{
		for (auto& script : scripts)
		{
			luabridge::push(script.GetLuaState(), collidedObj);
			script.RunFunction("OnCollisionEnter", 1, 0);
		}
	}

	std::vector<LuaScript> scripts;

	static rapidjson::Value ScriptComponentSerializeFunction(const void *scriptPtr, rapidjson::Document::AllocatorType& allocator)
	{
		// Const cast away is fine because we're not really changing anything.
		ScriptComponent& scriptComponent = *reinterpret_cast<ScriptComponent *>(const_cast<void *>(scriptPtr));

		// Setup the object to store the scripts in.
		rapidjson::Value scriptArray;
		scriptArray.SetArray();

		for (LuaScript& script : scriptComponent.scripts)
		{
			scriptArray.PushBack(meta::Serialize(script, allocator), allocator);
		}

		return scriptArray;
	}

	static void ScriptComponentDeserializeAssign(void *scriptPtr, rapidjson::Value& jsonScripts)
	{
		// Get the engine.
		ScriptComponent& scriptComponent = *reinterpret_cast<ScriptComponent *>(scriptPtr);

		// Get rid of all the scripts we have.
		scriptComponent.scripts.clear();

		// We should be passed the array of scripts.
		assert(jsonScripts.IsArray());
		for (rapidjson::Value& jsonScript : jsonScripts.GetArray())
		{
			meta::Any script(jsonScript);
			scriptComponent.scripts.push_back(script.GetData<LuaScript>());
		}
	}

	META_REGISTER(ScriptComponent)
	{
		META_DefineType(ScriptComponent);

		META_DefineSerializeFunction(ScriptComponent, ScriptComponentSerializeFunction);
		META_DefineDeserializeAssignFunction(ScriptComponent, ScriptComponentDeserializeAssign);
	}
};