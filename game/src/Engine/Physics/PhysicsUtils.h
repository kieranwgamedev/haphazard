/*
FILE: PhysicsUtils.h
PRIMARY AUTHOR: Brett Schiff

Physics Utilities that are useful outside the physics system

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#pragma once
#include "glm/glm.hpp"
#include "CollisionLayer.h"
#include "Collider2D.h"

#include "./GameObjectSystem/Component.h"

glm::vec2 CollidePointOnLayer(ComponentMap<DynamicCollider2DComponent>* allDynamicColliders, ComponentMap<StaticCollider2DComponent>* allStaticColliders,
						 const glm::vec2& position, collisionLayers layer = collisionLayers::allCollision);

void debugSetDisplayRaycasts(bool raycastsShown);

bool debugAreRaycastsDisplayed();

void debugSetDisplayHitboxes(bool hitboxesShown);

bool debugAreHitBoxesDisplayed();
