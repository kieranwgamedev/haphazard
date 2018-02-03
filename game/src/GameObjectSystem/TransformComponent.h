/*
FILE: TransformComponent.h
PRIMARY AUTHOR: Kieran

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#pragma once

#include "meta/meta.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>
#include "GameObject.h"

// TODO[Kieran] - Move this somewhere more general.
constexpr float DegToRad(float degrees)
{
	const float PI = 3.14159265358979323846264338327950f;
	const float RADIANS_IN_CIRCLE = 2 * PI;
	const float DEGREES_IN_CIRCLE = 360.0f;

	return degrees * RADIANS_IN_CIRCLE / DEGREES_IN_CIRCLE;
}

class Editor;

class TransformComponent
{
public:
	//--------------
	// Constructors
	//--------------

	TransformComponent(const glm::vec3& position = glm::vec3(0.0f),
					   const glm::vec3& scale = glm::vec3(1.0f),
					   float rotation = 0.0f);

	//---------
	// Getters
	//---------

	void SetParent(GameObject parent);
	void SetParentLua(GameObject parent);
	GameObject GetParent() const;

	float GetRotation() const;

	void SetRotation(const float& rotation);

	glm::vec3& GetRelativePosition();

	void SetZLayer(float layer);
	float GetZLayer() const;

	glm::vec2 GetPosition() const;
	glm::vec2 GetLocalPosition() const;

	void SetPosition(const glm::vec2& position);
	void SetLocalPosition(const glm::vec2& position);

	glm::vec3 GetScale() const;

	void SetScale(const glm::vec3& scale);

	glm::vec2 Scale2D() const;

	glm::mat4 GetMatrix4() const;

private:
	void Unparent();

	glm::vec2 GetParentPosition() const;

	friend void ImGui_Transform(TransformComponent *transform, GameObject object, Editor *editor);

	glm::vec3 m_localPosition;
	glm::vec3 m_scale;
	float m_rotation; // Stored in degrees.

	GameObject m_parent;

	META_REGISTER(TransformComponent)
	{
		META_DefineGetterSetter(TransformComponent, glm::vec2, GetPosition, SetPosition, "position");

		META_DefineGetterSetter(TransformComponent, glm::vec2, GetLocalPosition, SetLocalPosition, "localPosition");

		META_DefineGetterSetter(TransformComponent, glm::vec3, GetScale, SetScale, "scale");

		META_DefineGetterSetter(TransformComponent, float, GetZLayer, SetZLayer, "zLayer");

		META_DefineGetterSetter(TransformComponent, float, GetRotation, SetRotation, "rotation");

		META_DefineGetterSetter(TransformComponent, GameObject, GetParent, SetParentLua, "parent");
	}
};
