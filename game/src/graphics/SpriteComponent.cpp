/*
FILE: SpriteComponent.cpp
PRIMARY AUTHOR: Max Rauffer

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#include "SpriteComponent.h"
#include "TextureResource.h" // For INVALID_TEXTURE_ID
#include "Engine\Engine.h"

///
// Sprite Component
///

Mesh* SpriteComponent::m_Mesh = nullptr;


void SpriteComponent::ConstructUnitMesh()
{
	m_Mesh = new Mesh(); 
	m_Mesh->AddTriangle(
		//  x,     y, z,    r,    g,    b, a, s, t
		-0.5f, 0.5f, 0, 1.0f, 1.0f, 1.0f, 1, 0, 1, // Top Left
		-0.5f, -0.5f, 0, 1.0f, 1.0f, 1.0f, 1, 0, 0, // Bot Left
		0.5f, -0.5f, 0, 1.0f, 1.0f, 1.0f, 1, 1, 0  // Bot Right
	);
	m_Mesh->AddTriangle(
		//  x,     y, z,    r,    g,    b, a, s, t
		0.5f, -0.5f, 0, 1.0f, 1.0f, 1.0f, 1, 1, 0, // Bot Right
		0.5f, 0.5f, 0, 1.0f, 1.0f, 1.0f, 1, 1, 1, // Top Right
		-0.5f, 0.5f, 0, 1.0f, 1.0f, 1.0f, 1, 0, 1  // Top Left
	);
	m_Mesh->CompileMesh();
}

SpriteComponent::SpriteComponent(Resource *res)
	: m_TextureHandler { res }
{
	if (!m_Mesh)
		ConstructUnitMesh();
}

void SpriteComponent::SetTextureResource(Resource *res)
{
	if (res)
	{
		Assert(res->GetResourceType() == ResourceType::TEXTURE || res->GetResourceType() == ResourceType::ANIMATION);
		m_TextureHandler.SetResource(res);
	}
	else
	{
		m_TextureHandler.SetResource(Resource::GetDefaultResource(ResourceType::TEXTURE));
	}
}


void SpriteComponent::SetTextureID(ResourceID res)
{
	Resource *resource = engine->GetResourceManager().Get(res);
	SetTextureResource(resource);
}

void SpriteComponent::SetRenderData(const TransformComponent* transform, std::vector<float>* data)
{
	data->push_back(m_Color.x);
	data->push_back(m_Color.y);
	data->push_back(m_Color.z);
	data->push_back(m_Color.w);

	glm::vec4 bounds = m_TextureHandler.GetBounds();

	if (m_TextureHandler.IsTiling())
	{
		data->push_back(bounds.x + 1);
		data->push_back(bounds.y + 1);
		data->push_back(bounds.z + 1);
		data->push_back(bounds.w + 1);
	}
	else
	{
		data->push_back(bounds.x);
		data->push_back(bounds.y);
		data->push_back(bounds.z);
		data->push_back(bounds.w);
	}

	// Load data into array
	glm::mat4 matrix = transform->GetMatrix4();
	for (int i = 0; i < 4 * 4; i++)
		data->push_back(matrix[i / 4][i % 4]);


	if (m_TextureHandler.IsTiling())
	{
		data->push_back(transform->GetScale().x * m_TextureHandler.GetTileAmount().x);
		data->push_back(transform->GetScale().y * m_TextureHandler.GetTileAmount().y);
	}
	else
	{
		data->push_back(1);
		data->push_back(1);
	}
}

void SpriteComponent::UpdateTextureHandler(float dt)
{
	m_TextureHandler.Update(dt);
}
