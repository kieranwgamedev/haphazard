/*
FILE: PhysicsSystem.cpp
PRIMARY AUTHOR: Brett Schiff

Handles Movement, Colliders, Collision Detection, and Collision Resolution

Copyright � 2017 DigiPen (USA) Corporation.
*/
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <algorithm>

#include "PhysicsSystem.h"
#include "RigidBody.h"
#include "Collider2D.h"
#include "../../graphics/DebugGraphic.h"
#include "GameObjectSystem\GameObject.h"
#include "PhysicsInternalTools.h"
#include "PhysicsUtils.h"

//TEMP
#include "Raycast.h"

#define MIN x
#define MAX y

bool debugShowHitboxes = true;

void debugDisplayHitboxes(bool hitboxesShown)
{
	debugShowHitboxes = hitboxesShown;
}

struct MinMax
{
	float min;
	float max;

	// see if two sets of min/max intersect
	bool Intersects(const MinMax& other)
	{
		return min < other.max && max > other.min;
	}

	float Overlap(const MinMax& other)
	{
		float overlap1 = abs(other.max - min);
		float overlap2 = abs(min - other.max);

		return std::min(overlap1, overlap2);
	}
};

std::ostream& operator<<(std::ostream& ostream, const BoxCollider& colliderBox)
{
	ostream << "Top Right Corner: (" << colliderBox.m_topRight.x << ", " << colliderBox.m_topRight.y << ")" << std::endl;
	ostream << "Bot Left  Corner: (" << colliderBox.m_botLeft.x << ", " << colliderBox.m_botLeft.y << ")" << std::endl;


	return ostream;
}

BoxCollider::BoxCollider(const glm::vec2& center, const glm::vec3& dimensions, float rotation)
{
	glm::vec3 boxCenter = glm::vec3(center, 0);

	// if the box is axis-aligned, the calculation can be done ignoring angles
	if (rotation == 0)
	{
		m_topRight = boxCenter + (.5f * dimensions);
		m_botLeft = boxCenter - (.5f * dimensions);
		m_rotation = 0;
	}
	else
	{
		m_rotation = rotation;

		// calculate the top right corner
		m_topRight.x = boxCenter.x + (dimensions.x * 0.5f * cos(m_rotation)) - (dimensions.y * 0.5f * sin(m_rotation));
		m_topRight.y = boxCenter.y + (dimensions.x * 0.5f * sin(m_rotation)) + (dimensions.y * 0.5f * cos(m_rotation));

		// calculate the bottom left corner
		m_botLeft.x = boxCenter.x - (dimensions.x * 0.5f * cos(m_rotation)) + (dimensions.y * 0.5f * sin(m_rotation));
		m_botLeft.y = boxCenter.y - (dimensions.x * 0.5f * sin(m_rotation)) - (dimensions.y * 0.5f * cos(m_rotation));
	}
}

MinMax BoxCorners::ProjectOntoAxis(glm::vec2 axis) const
{
	float firstDot = glm::dot(axis, m_corners[0]);
	// we only care about the min and max values
	MinMax minMax = {firstDot, firstDot};

	// project each of the four points onto the axis and record the extrema
	for (int i = 1; i < 4; ++i)
	{
		float dotP = glm::dot(axis, m_corners[i]);
		if (dotP < minMax.min)
		{
			minMax.min = dotP;
		}
		else if (dotP > minMax.max)
		{
			minMax.max = dotP;
		}
	}

	return minMax;
}

std::ostream& operator<<(std::ostream& lhs, glm::vec2 rhs)
{
	lhs << rhs.x << "\t" << rhs.y;

	return lhs;
}

glm::vec3 Collision_SAT(const BoxCorners& Box1, const BoxCorners& Box2)
{
	const int num_projections = 4;
	float smallestOverlap = -1;
	glm::vec2 smallestAxis;

	//!?!? do an optimized AABB check to first rule out distant collisions

	// the vectors onto which each shape will be projected
	glm::vec2 edgeNormals[num_projections] = { { 0,0 } };

	// since these are boxes, we only have to check two of the four side since the rest are parallel, and they are already perpindicular to each other
	edgeNormals[0] = Box1.m_corners[BoxCorners::topRight] - Box1.m_corners[BoxCorners::topLeft];
	edgeNormals[1] = Box1.m_corners[BoxCorners::topRight] - Box1.m_corners[BoxCorners::botRight];
	edgeNormals[2] = Box2.m_corners[BoxCorners::topRight] - Box2.m_corners[BoxCorners::topLeft];
	edgeNormals[3] = Box2.m_corners[BoxCorners::topRight] - Box2.m_corners[BoxCorners::botRight];

	for (int i = 0; i < num_projections; ++i)
	{
		edgeNormals[i] = glm::normalize(edgeNormals[i]);
	}

	// project shapes onto the vectors
	for (int i = 0; i < num_projections; ++i)
	{
		// the axis onto which we will project
		glm::vec2 axis = edgeNormals[i];

		// project corners onto the axis
		MinMax proj1 = Box1.ProjectOntoAxis(axis);
		MinMax proj2 = Box2.ProjectOntoAxis(axis);

		// if the projections are not intersecting, the shapes are not intersecting
		if (!proj1.Intersects(proj2))
		{
			return glm::vec3(0, 0, 0);
		}
		else // else we need to check the overlap for being the smallest escape vector
		{
			float overlap = proj1.Overlap(proj2);

			// if this overlap is less than the last recorded one
			if (overlap < smallestOverlap || smallestOverlap == -1)
			{
				smallestOverlap = overlap;
				smallestAxis = axis;
			}
		}
	}

	// if we get here it is guarunteed that there was a collision and all sides have been tested for the shortest overlap
	glm::vec3 escapeVector(smallestAxis * smallestOverlap, 0);

	//std::cout << "smallestAxis: " << smallestAxis << std::endl;
	//std::cout << "escapeVector: " << escapeVector << std::endl;

	return escapeVector;
}

glm::vec3 Collision_SAT(glm::vec2 position1, float rotation1, Collider2D& collider1, glm::vec2 position2, float rotation2, Collider2D& collider2)
{
	glm::vec2 obj1Center = position1 + static_cast<glm::vec2>(collider1.GetOffset());
	glm::vec2 obj1Dimensions = collider1.GetDimensions();
	float obj1Rotation = rotation1 + collider1.GetRotationOffset();

	glm::vec2 obj2Center = position2 + static_cast<glm::vec2>(collider2.GetOffset());
	glm::vec2 obj2Dimensions = collider2.GetDimensions();
	float obj2Rotation = rotation2 + collider2.GetRotationOffset();

	// get the corners of each of the objects
	BoxCorners Box1(obj1Center, obj1Dimensions, obj1Rotation);
	BoxCorners Box2(obj2Center, obj2Dimensions, obj2Rotation);

	return Collision_SAT(Box1, Box2);
}

glm::vec3 Collision_SAT(ComponentHandle<TransformComponent>& transform1, Collider2D& collider1, ComponentHandle<TransformComponent>& transform2, Collider2D& collider2)
{
	return Collision_SAT(transform1->GetPosition(), transform1->GetRotation(), collider1, transform2->GetPosition(), transform2->GetRotation(), collider2);
}


glm::vec3 Collision_AABBToAABB(BoxCollider& Box1, BoxCollider& Box2)
{
	glm::vec3 penetrationVector(0);
	glm::vec3 minValue(0);

	if (Box1.m_topRight.x <= Box2.m_botLeft.x)
	{
		return glm::vec3(0);
	}
	else
	{
		minValue.x = Box1.m_topRight.x - Box2.m_botLeft.x;
	}
	if (Box1.m_topRight.y <= Box2.m_botLeft.y)
	{
		return glm::vec3(0);
	}
	else
	{
		minValue.y = Box1.m_topRight.y - Box2.m_botLeft.y;
	}
	if (Box1.m_botLeft.x >= Box2.m_topRight.x)
	{
		return glm::vec3(0);
	}
	else
	{
		if (abs(Box1.m_botLeft.x - Box2.m_topRight.x) < abs(minValue.x))
		{
			minValue.x = Box1.m_botLeft.x - Box2.m_topRight.x;
		}
	}
	if (Box1.m_botLeft.y >= Box2.m_topRight.y)
	{
		return glm::vec3(0);
	}
	else
	{
		if (abs(Box1.m_botLeft.y - Box2.m_topRight.y) < abs(minValue.y))
		{
			minValue.y = Box1.m_botLeft.y - Box2.m_topRight.y;
		}
	}

	// if the resolution needs to be primarily along the x axis
	if (abs(minValue.x) < abs(minValue.y))
	{
		penetrationVector.x = minValue.x;
	}
	else // if the resolution needs to be primarily along the y axis
	{
		penetrationVector.y = minValue.y;
	}

	return -penetrationVector;
}

glm::vec3 Collision_AABBToAABB(ComponentHandle<TransformComponent>& AABB1Transform, Collider2D& AABB1Collider, ComponentHandle<TransformComponent>& AABB2Transform, Collider2D& AABB2Collider)
{
	BoxCollider Box1(AABB1Transform->GetPosition(), AABB1Collider.GetDimensions(), AABB1Transform->GetRotation() + AABB1Collider.GetRotationOffset());
	BoxCollider Box2(AABB2Transform->GetPosition(), AABB2Collider.GetDimensions(), AABB2Transform->GetRotation() + AABB2Collider.GetRotationOffset());

	return Collision_AABBToAABB(Box1, Box2);
}

bool Collision_PointToBoxQuick(const glm::vec2& point, const BoxCorners& box, float boxRotation)
{
	// use AABB collision if the box is not rotated
	if (boxRotation == 0)
	{
		// get the top right and bottom left corners of the object
		const glm::vec2 botLeftCorner = box.m_corners[BoxCorners::botLeft];
		const glm::vec2 topRightCorner = box.m_corners[BoxCorners::topRight];

		// if the point is above the object, return false
		if (point.y >= topRightCorner.y)
		{
			return false;
		}
		// if the point is let of the object, return false
		if (point.x <= botLeftCorner.x)
		{
			return false;
		}
		// if the point is right of the object, return false
		if (point.x >= topRightCorner.x)
		{
			return false;
		}
		// if the point is below the object, return false
		if (point.y <= botLeftCorner.y)
		{
			return false;
		}
	}
	else
	{
		glm::vec3 result = Collision_SAT(BoxCorners(point, glm::vec2(.000001f, .000001f), 0), box);

		if (result.x || result.y)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

void printAMatrix(glm::mat3 matrix)
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			std::cout << matrix[i][j] << "   ";
		}
		std::cout << "\n";
	}

}

void ResolveDynDynCollision(glm::vec3* collisionData, ComponentHandle<DynamicCollider2DComponent> collider1, ComponentHandle<TransformComponent> transform1, ComponentHandle<DynamicCollider2DComponent> collider2, ComponentHandle<TransformComponent> transform2)
{
	ComponentHandle<RigidBodyComponent> rigidBody1 = collider1.GetSiblingComponent<RigidBodyComponent>();
	ComponentHandle<RigidBodyComponent> rigidBody2 = collider2.GetSiblingComponent<RigidBodyComponent>();
	// make sure rigidbodies were successfully retrieved
	assert(rigidBody1.IsValid() && rigidBody2.IsValid() && "Rigidbody(s) invalid. See ResolveDynDynCollision in PhysicsSystem.cpp\n");

	glm::vec3 resolutionVector = *collisionData;
	glm::vec3 dividingVector((*collisionData).y, -(*collisionData).x, 0);

	//std::cout << dividingVector.x << " " << dividingVector.y << "\n";

	float a = dividingVector.x;
	float b = dividingVector.y;
	float a2 = dividingVector.x * dividingVector.x;
	float b2 = dividingVector.y * dividingVector.y;

	// get the matrix for reflecting over dividing vector
	glm::mat3 refMtrx(1-((2*b2)/(a2+b2)), (2*a*b)/(a2+b2), 0,
					 ((2*a*b)/(a2+b2)), ((2*b2)/(a2+b2)) - 1, 0,
					 0, 0, 1
					 );

	// check to see if the objects are moving in the same direction
	float xCompare = rigidBody1->Velocity().x / rigidBody2->Velocity().x;
	float yCompare = rigidBody1->Velocity().y / rigidBody2->Velocity().y;

	//!?!? quick cheap solution - replace with a real one later
	glm::vec2 pos1 = transform1->GetPosition();
	glm::vec2 pos2 = transform2->GetPosition();

	glm::vec2 halfresolve = resolutionVector * 0.5f;

	transform1->SetPosition(pos1 + halfresolve);
	transform2->SetPosition(pos2 - halfresolve);

	// if they are not going in the same direction
	if (xCompare < 0 || yCompare < 0)
	{
		glm::vec2 prev = rigidBody1->Velocity();

		rigidBody1->SetVelocity(refMtrx * rigidBody1->Velocity());
		rigidBody2->SetVelocity(refMtrx * rigidBody2->Velocity());

		glm::vec2 post = rigidBody1->Velocity();
		glm::vec2 temp(0, 0);
	}
	else // if they are going in the same direction
	{
		// the magnitude of each object's velocity
		float obj1SquaredMagnitude = (rigidBody1->Velocity().x * rigidBody1->Velocity().x) + (rigidBody1->Velocity().y * rigidBody1->Velocity().y);
		float obj2SquaredMagnitude = (rigidBody2->Velocity().x * rigidBody2->Velocity().x) + (rigidBody2->Velocity().y * rigidBody2->Velocity().y);

		if (obj1SquaredMagnitude < obj2SquaredMagnitude)
		{
			rigidBody1->SetVelocity(rigidBody1->Velocity());
			rigidBody2->SetVelocity(refMtrx * rigidBody2->Velocity());
		}
		else
		{
			rigidBody1->SetVelocity(refMtrx * rigidBody1->Velocity());
			rigidBody2->SetVelocity(rigidBody2->Velocity());
		}
	}
}

void ResolveDynStcCollision(glm::vec3* collisionData, ComponentHandle<DynamicCollider2DComponent> collider1, ComponentHandle<StaticCollider2DComponent> collider2)
{
	ComponentHandle<RigidBodyComponent> rigidBody1 = collider1.GetSiblingComponent<RigidBodyComponent>();
	// make sure rigidbodies were successfully retrieved
	assert(rigidBody1.IsValid() && "Rigidbody invalid. See ResolveDynStcCollision in PhysicsSystem.cpp\n");

	ComponentHandle<TransformComponent> transform1 = collider1.GetSiblingComponent<TransformComponent>();
	assert(rigidBody1.IsValid() && "Transform is invalid. See REsolveDynStcCollision in PhysicsSystem.cpp\n");

	glm::vec2 position = transform1->GetPosition();

	glm::vec3 resolutionVector = *collisionData;

	if (resolutionVector.x)
	{
		transform1->SetPosition(glm::vec2(position.x + resolutionVector.x, position.y));
		rigidBody1->SetVelocity(glm::vec3(0, rigidBody1->Velocity().y, rigidBody1->Velocity().z));
	}
	if (resolutionVector.y)
	{
		transform1->SetPosition(glm::vec2(position.x, position.y + resolutionVector.y));
		rigidBody1->SetVelocity(glm::vec3(rigidBody1->Velocity().x, 0, rigidBody1->Velocity().z));
	}
}

void UpdateMovementData(float dt, ComponentHandle<TransformComponent> transform, ComponentHandle<RigidBodyComponent> rigidBody, glm::vec3 velocity, glm::vec3 acceleration)
{
	transform->SetPosition(glm::vec3(transform->GetPosition(), 0) + velocity * dt);
	rigidBody->AddVelocity(acceleration * dt);
}

void DebugDrawAllHitboxes(ComponentMap<DynamicCollider2DComponent> *allDynamicColliders, ComponentMap<StaticCollider2DComponent> *allStaticColliders)
{
	for (auto tDynamiColliderHandle : *allDynamicColliders)
	{
		ComponentHandle<TransformComponent> transform = tDynamiColliderHandle.GetSiblingComponent<TransformComponent>();
		assert(transform.IsValid() && "Transform invalid in debug drawing, see DebugDrawAllHitboxes in PhysicsSystem.cpp");

		float rotation = transform->GetRotation() + tDynamiColliderHandle->ColliderData().GetRotationOffset();

		DebugGraphic::DrawShape(transform->GetPosition(), tDynamiColliderHandle->ColliderData().GetDimensions(), DegreesToRadians(rotation), glm::vec4(1, 0, 1, 1));
	}
	for (auto tStaticColliderHandle : *allStaticColliders)
	{
		ComponentHandle<TransformComponent> transform = tStaticColliderHandle.GetSiblingComponent<TransformComponent>();
		assert(transform.IsValid() && "Transform invalid in debug drawing, see DebugDrawAllHitboxes in PhysicsSystem.cpp");

		float rotation = transform->GetRotation() + tStaticColliderHandle->ColliderData().GetRotationOffset();

		DebugGraphic::DrawShape(transform->GetPosition(), tStaticColliderHandle->ColliderData().GetDimensions(), DegreesToRadians(rotation), glm::vec4(.1f, .5f, 1, 1));
	}
}

void ClearAllRecordedCollisions(ComponentMap<DynamicCollider2DComponent> *allDynamicColliders, ComponentMap<StaticCollider2DComponent> *allStaticColliders)
{
	for (auto tDynamiColliderHandle : *allDynamicColliders)
	{
		tDynamiColliderHandle->ColliderData().ClearCollidedLayers();
	}
	for (auto tStaticColliderHandle : *allStaticColliders)
	{
		tStaticColliderHandle->ColliderData().ClearCollidedLayers();
	}
}

void MoveAllDynamicObjects(float dt, ComponentMap<RigidBodyComponent>& rigidBodies)
{
	for (auto tRigidBodyHandle : rigidBodies)
	{
		ComponentHandle<TransformComponent> transform = tRigidBodyHandle.GetSiblingComponent<TransformComponent>();

		// update position, velocity, and acceleration using stored values
		UpdateMovementData(dt, transform, tRigidBodyHandle, tRigidBodyHandle->Velocity(), tRigidBodyHandle->Acceleration());
	}
}

// registers collision between two layers upon each other
void RegisterCollision(Collider2D& collider1, Collider2D& collider2)
{
	collisionLayers collider1Layer = collider1.GetCollisionLayer();
	collisionLayers collider2Layer = collider2.GetCollisionLayer();

	collider1.AddCollidedLayer(collider2Layer);
	collider2.AddCollidedLayer(collider1Layer);
}

void PhysicsSystem::Init()
{

}

void PhysicsSystem::Update(float dt)
{
	// get all rigid bodies
	ComponentMap<RigidBodyComponent> *rigidBodies = GetGameSpace()->GetComponentMap<RigidBodyComponent>();

	// get all dynamic colliders
	ComponentMap<DynamicCollider2DComponent> *allDynamicColliders = GetGameSpace()->GetComponentMap<DynamicCollider2DComponent>();

	// get all static colliders
	ComponentMap<StaticCollider2DComponent> *allStaticColliders = GetGameSpace()->GetComponentMap<StaticCollider2DComponent>();

	if (debugShowHitboxes)
	{
		DebugDrawAllHitboxes(allDynamicColliders, allStaticColliders);
	}

	// clear out the recorded collision layers so the recording will be accurate to this frame
	ClearAllRecordedCollisions(allDynamicColliders, allStaticColliders);

	// update the position and velocity of all objects according to their velocity and acceleration
	MoveAllDynamicObjects(dt, *rigidBodies);


	/************************** TEST STUFF **************************/
	float range = 6;
	glm::vec3 castPosition(-2, 2.5, 0);
	
	glm::vec3 normalizedDirection(2, -1.5f, 0);

	normalizedDirection /= glm::length(normalizedDirection);

	Raycast testCast(allDynamicColliders, allStaticColliders, castPosition, normalizedDirection, range);

	DrawSmallBoxAtPosition(castPosition);
	DebugGraphic::DrawShape(castPosition + (normalizedDirection * (testCast.Length() / 2)), glm::vec2(testCast.Length(), .01f), atan2(normalizedDirection.y, normalizedDirection.x), glm::vec4(1, 1, 1, 1));
	DrawSmallBoxAtPosition(testCast.Intersection());

	glm::vec2 testPoint(1, 0);
	glm::vec2 pointEscape = CollidePointOnLayer(allDynamicColliders, allStaticColliders, testPoint);
	testPoint += pointEscape;
	DrawSmallBoxAtPosition(testPoint);
	/****************************************************************/

	for (auto& tRigidBodyHandle : *rigidBodies)
	{
		// get the transform from the same gameobject, and leave the loop if it isn't valid
		ComponentHandle<TransformComponent> transform = tRigidBodyHandle.GetSiblingComponent<TransformComponent>();
		ComponentHandle<DynamicCollider2DComponent> dynamicCollider = tRigidBodyHandle.GetSiblingComponent<DynamicCollider2DComponent>();
		
		// if transform and collider are valid, collide it with things
		if (transform.IsValid() && dynamicCollider.IsValid())
		{
			glm::vec3 resolutionVector(0);

			glm::vec3 collidedAcceleration = tRigidBodyHandle->Acceleration();

			// loop through all dynamic colliders
			for (auto tDynamiColliderHandle : *allDynamicColliders)
			{
				// get the colliders out of the objects
				Collider2D collider1 = dynamicCollider->ColliderData();
				Collider2D collider2 = tDynamiColliderHandle->ColliderData();

				if (dynamicCollider == tDynamiColliderHandle)
				{
					continue;
				}

				if(!collider1.GetCollisionLayer().LayersCollide(collider2.GetCollisionLayer()))
				{
					continue;
				}

				ComponentHandle<TransformComponent> otherTransform = tDynamiColliderHandle.GetSiblingComponent<TransformComponent>();
				assert(otherTransform.IsValid() && "Invalid transform on collider, see PhysicsSystem::Update in PhysicsSystem.cpp");

				float object1Rotation = transform->GetRotation() + collider1.GetRotationOffset();
				float object2Rotation = otherTransform->GetRotation() + collider2.GetRotationOffset();

				// check for collision on non-rotated objects
				if (object1Rotation == 0 && object2Rotation == 0)
				{
					resolutionVector = Collision_AABBToAABB(transform, collider1, otherTransform, collider2);
				}
				else // check for collision on rotated objects
				{
					resolutionVector = Collision_SAT(transform, collider1, otherTransform, collider2);
				}


				// if there was a collision, resolve it
				if (resolutionVector.x || resolutionVector.y)
				{
					// register collision between the layers
					RegisterCollision(dynamicCollider->ColliderData(), tDynamiColliderHandle->ColliderData());
					// resolve the collision
					ResolveDynDynCollision(&resolutionVector, dynamicCollider, transform, tDynamiColliderHandle, otherTransform);
				}
			}

			// loop through all static colliders
			for (auto tStaticColliderHandle : *allStaticColliders)
			{
				// get the colliders out of the objects
				Collider2D collider1 = dynamicCollider->ColliderData();
				Collider2D collider2 = tStaticColliderHandle->ColliderData();

				if (!collider1.GetCollisionLayer().LayersCollide(collider2.GetCollisionLayer()))
				{
					continue;
				}

				ComponentHandle<TransformComponent> otherTransform = tStaticColliderHandle.GetSiblingComponent<TransformComponent>();
				assert(otherTransform.IsValid() && "Some static object's returned an invalid transform in PhysicsSysterm::Update in PhysicsSystem.cpp");
				
				float object1Rotation = transform->GetRotation() + collider1.GetRotationOffset();
				float object2Rotation = otherTransform->GetRotation() + collider2.GetRotationOffset();

				// check for collision on non-rotated objects
				if (object1Rotation == 0 && object2Rotation == 0)
				{
					resolutionVector = Collision_AABBToAABB(transform, collider1, otherTransform, collider2);
				}
				else
				{
					resolutionVector = Collision_SAT(transform, collider1, otherTransform, collider2);
				}

				// if there was a collision, resolve it
				if (resolutionVector.x || resolutionVector.y)
				{
					// register collision between the layers
					RegisterCollision(dynamicCollider->ColliderData(), tStaticColliderHandle->ColliderData());
					// resolve the collision
					ResolveDynStcCollision(&resolutionVector, dynamicCollider, tStaticColliderHandle);
				}
			}
		}
		// if transform is valid and dynamic collider isnt, only update movement
		else if (transform.IsValid() && !dynamicCollider.IsValid())
		{
			// update position, velocity, and acceleration using stored values
			UpdateMovementData(dt, transform, tRigidBodyHandle, tRigidBodyHandle->Velocity(), tRigidBodyHandle->Acceleration());
		}
	}
}

SystemBase *PhysicsSystem::NewDuplicate()
{
	return new PhysicsSystem(*this);
}
