/*
FILE: RenderSystem.cpp
PRIMARY AUTHOR: Max Rauffer

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#include "graphics\RenderSystem.h"
#include "GameObjectSystem\GameSpace.h"
#include "graphics\SpriteComponent.h"
#include "GameObjectSystem\TransformComponent.h"
#include "GameObjectSystem\GameObject.h"
#include "Screen.h"
#include "GLFW\glfw3.h"
#include "Camera.h"
#include "Settings.h"
#include "DebugGraphic.h"
#include "Text.h"
#include "Particles.h"

#include <imgui.h>
#include "Imgui\imgui-setup.h"

static bool resizeCameras = false;
static int width;
static int height;

RenderSystem::RenderSystem()
{
}

void RenderSystem::Init()
{
	Screen::InitScreen();
	Font::InitFonts();

//	Screen::GetView().AddEffect(FX::EDGE_DETECTION);
//	Screen::GetView().AddEffect(FX::BLOOM);
//	Screen::GetView().SetBlurAmount(0.9f);
}

void RenderSystem::UpdateCameras(float dt)
{
	ComponentMap<Camera> *cameras = GetGameSpace()->GetComponentMap<Camera>();

	for (auto& camera : *cameras)
	{
		// Check for valid transform
		ComponentHandle<TransformComponent> transform = camera.GetSiblingComponent<TransformComponent>();
		if (!transform.IsValid())
		{
			continue;
		}
		//Update Cameras

		if (resizeCameras)
		{
			// Screen resized, update camera matrices
			camera->SetAspectRatio(width / (float)height);
		}

		// If transform moved, update camera matrices
		if (transform->GetPosition() != camera->GetPosition())
			camera->SetPosition(transform->GetPosition());
	}
	resizeCameras = false;
}

void RenderSystem::RenderSprites(float dt)
{
	ComponentMap<SpriteComponent> *sprites = GetGameSpace()->GetComponentMap<SpriteComponent>();

	// Instancing variables
	std::vector<float> data;
	std::vector<int> tex;
	int numMeshes = 0;
	static int numVerts = 0; // All sprites will be the same size, so numVerts only has to be set once

	for (auto& spriteHandle : *sprites)
	{
		// Check for valid transform
		ComponentHandle<TransformComponent> transform = spriteHandle.GetSiblingComponent<TransformComponent>();
		if (!transform.IsValid())
		{
			continue;
		}

		// Update animated sprites
		spriteHandle->UpdateAnimatedTexture(dt);

		// Places vertex data into data vector to be used in Vertex VBO
		spriteHandle->SetRenderData(transform->GetMatrix4(), &data);

		// Places texture in tex vector to be used in Texture VBO
		tex.push_back(spriteHandle->GetRenderTextureID());

		// Keep count of all meshes used in instancing call
		numMeshes++;

		if (numVerts == 0) 
			numVerts = spriteHandle->NumVerts();
	}

	// Bind sprite shader
	Shaders::spriteShader->Use();

	// Bind buffers and set instance data of all sprites
	Mesh::BindTextureVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(int) * tex.size(), tex.data(), GL_STATIC_DRAW);

	Mesh::BindInstanceVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);

	// Bind first sprite VAO, all sprite have the same vertex attributes
	sprites->begin()->BindVAO();

	// Draw all sprites
	glDrawArraysInstanced(GL_TRIANGLES, 0, numMeshes * numVerts, numMeshes);
}

void RenderSystem::RenderText(float dt)
{
	ComponentMap<TextComponent> *text = GetGameSpace()->GetComponentMap<TextComponent>();

	for (auto& textHandle : *text)
	{
		// Check for valid transform
		ComponentHandle<TransformComponent> transform = textHandle.GetSiblingComponent<TransformComponent>();
		if (!transform.IsValid())
		{
			continue;
		}

		textHandle->Draw(transform->GetMatrix4());
	}
}

void RenderSystem::RenderParticles(float dt)
{
	ComponentMap<ParticleSystem> *particles = GetGameSpace()->GetComponentMap<ParticleSystem>();
	for (auto& particleHandle : *particles)
	{
		ComponentHandle<TransformComponent> transform = particleHandle.GetSiblingComponent<TransformComponent>();
		if (!transform.IsValid())
		{
			continue;
		}
		particleHandle->Render(dt, transform->GetPosition());
	}
}

// Called each frame.
void RenderSystem::Update(float dt)
{
	// Clear screen and sets correct framebuffer
	Screen::Use();

	//Start Loop
	UpdateCameras(dt);
	RenderSprites(dt);
	RenderText(dt);
	RenderParticles(dt);
	
	//End loop
	glBlendFunc(GL_ONE, GL_ZERO); // Disable blending for debug and screen rendering
	DebugGraphic::DrawAll();
	Screen::Draw(); // Draw to screen and apply post processing effects
}

void RenderSystem::ResizeWindowEvent(GLFWwindow* window, int w, int h)
{
	width = w;
	height = h;

	Screen::ResizeScreen(width, height);
	resizeCameras = true;
}

SystemBase *RenderSystem::NewDuplicate()
{
	return new RenderSystem(*this);
}

// Simply returns the default priority for this system.
size_t RenderSystem::DefaultPriority()
{
	return 999;
}
