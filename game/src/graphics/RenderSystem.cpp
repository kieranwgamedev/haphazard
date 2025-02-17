/*
FILE: RenderSystem.cpp
PRIMARY AUTHOR: Max Rauffer

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#include "graphics\RenderSystem.h"
#include "Engine\Engine.h"
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
#include <set>
#include <vector>

#include <imgui.h>
#include "Imgui\imgui-setup.h"
#include "Background.h"

#include "RenderLayer.h"
#include "Input\Input.h"
#include "EditorGraphic.h"
#include "VertexObjects.h"

static bool resizeCameras = false;
static int width;
static int height;
static int currLayer;

void ToggleFullscreen();

RenderSystem::RenderSystem()
{
}

void RenderSystem::Init()
{
	glDisable(GL_MULTISAMPLE);
	glDisable(GL_DEPTH_TEST); // Don't need depth for 2D. Render things in order.
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Screen::InitScreen();
	Font::InitFonts();
}

void RenderSystem::UpdateCameras(float dt)
{
	ComponentMap<Camera> *cameras = GetGameSpace()->GetComponentMap<Camera>();

	int numCameras = 0;
	for (auto& camera : *cameras)
	{
		numCameras++;
		if (!camera->IsActiveCam()) // Only update the active camera
			continue;

		// Check for valid transform
		ComponentHandle<TransformComponent> transform = camera.GetSiblingComponent<TransformComponent>();
		if (!transform.IsActive())
		{
			continue;
		}
		//Update Cameras

		camera->SetAspectRatio(width / (float)height);
		camera->SetPosition(transform->GetPosition());
		camera->SetRotation(transform->GetRotation());
	}
	resizeCameras = false;

	if (numCameras == 0)
	{
		Logging::Log(Logging::GRAPHICS, Logging::MEDIUM_PRIORITY, "There are no cameras in the scene!");
	}
	else if (Camera::GetActiveCamera())
	{
		Camera::GetActiveCamera()->Use();
	}
	else
	{
		Logging::Log(Logging::GRAPHICS, Logging::MEDIUM_PRIORITY, "No camera set for use, choosing active camera");
		cameras->begin()->Use();
	}
}

void RenderSystem::RenderSprites(float dt)
{
	// Bind sprite shader
	Shaders::spriteShader->Use();

	ComponentMap<SpriteComponent> *sprites = GetGameSpace()->GetComponentMap<SpriteComponent>();

	if (sprites->begin() == sprites->end())
		return;

	// Instancing variables
	std::vector<float> data;
	std::vector<int> tex;
	int numMeshes = 0;

	// All sprites will be the same mesh, so numVerts only has to be set once
	static const int numVerts = SpriteComponent::SpriteMesh()->GetNumVerts();

	int last = -1234;
	for (auto& spriteHandle : *sprites)
	{
		// Only bother with valid transforms.
		ComponentHandle<TransformComponent> transformHandle = spriteHandle.GetSiblingComponent<TransformComponent>();
		if (!transformHandle.IsActive())
		{
			continue;
		}

		int layer = transformHandle->GetZLayer();

		// Switch layer if necessary.
		if (layer != last)
		{
			// Render the current layer.
			if (numMeshes != 0)
			{
				// Bind buffers and set instance data of all sprites
				SpriteComponent::SpriteMesh()->TextureBuffer().SetData(sizeof(int), static_cast<int>(tex.size()), tex.data());
				SpriteComponent::SpriteMesh()->InstanceBuffer().SetData(sizeof(float), static_cast<int>(data.size()), data.data());

				// Bind sprite attribute bindings (all sprites have the same bindings)
				SpriteComponent::SpriteMesh()->UseAttributeBindings();

				// Draw all sprites in this layer
				glDrawArraysInstanced(GL_TRIANGLES, 0, numVerts, numMeshes);
			}

			// Switch to the next layer.
			Screen::GetLayerFrameBuffer(layer)->Use();

			numMeshes = 0;
			data.clear();
			tex.clear();
		}

		// Update animated sprites
		spriteHandle->UpdateTextureHandler(dt);

		// Places vertex data into data vector to be used in Vertex VBO
		spriteHandle->SetRenderData(transformHandle.Get(), &data);

		// Places texture in tex vector to be used in Texture VBO
		tex.push_back(spriteHandle->GetTextureRenderID());

		// Keep count of all meshes used in instancing call
		numMeshes++;

		last = layer;
	}


	if (numMeshes != 0)
	{
		// Bind buffers and set instance data of all sprites
		SpriteComponent::SpriteMesh()->TextureBuffer().SetData(sizeof(int), static_cast<int>(tex.size()), tex.data());
		SpriteComponent::SpriteMesh()->InstanceBuffer().SetData(sizeof(float), static_cast<int>(data.size()), data.data());

		// Bind sprite attribute bindings (all sprites have the same bindings)
		SpriteComponent::SpriteMesh()->UseAttributeBindings();

		// Draw all sprites in this layer
		glDrawArraysInstanced(GL_TRIANGLES, 0, numVerts, numMeshes);
	}

	Screen::GetLayerFrameBuffer(last)->Use();

	numMeshes = 0;
	data.clear();
	tex.clear();
}

void RenderSystem::RenderText(float dt)
{
	ComponentMap<TextComponent> *text = GetGameSpace()->GetComponentMap<TextComponent>();
	int lastLayer = -999;

	for (auto& textHandle : *text)
	{
		// Check for valid transform
		ComponentHandle<TransformComponent> transform = textHandle.GetSiblingComponent<TransformComponent>();
		if (!transform.IsActive())
		{
			continue;
		}

		int layer = static_cast<int>(transform->GetZLayer());
		if (lastLayer != layer)
		{
			Screen::GetLayerFrameBuffer(layer)->Use();
			lastLayer = layer;
		}

		textHandle->Draw(transform->GetMatrix4());
	}
}

void RenderSystem::RenderParticles(float dt)
{
	ComponentMap<ParticleSystem> *particles = GetGameSpace()->GetComponentMap<ParticleSystem>();
	int lastLayer = -999;

	for (auto& particleHandle : *particles)
	{
		ComponentHandle<TransformComponent> transform = particleHandle.GetSiblingComponent<TransformComponent>();
		if (!transform.IsActive())
		{
			continue;
		}

		int layer = static_cast<int>(transform->GetZLayer());
		if (lastLayer != layer)
		{
			Screen::GetLayerFrameBuffer(layer)->Use();
			lastLayer = layer;
		}

		particleHandle->Render(dt, transform->GetPosition(), particleHandle.GetGameObject().Getid());
	}
}

void RenderSystem::RenderBackgrounds(float dt)
{
	glm::vec2 camPos = Camera::GetActiveCamera()->GetPosition();
	int lastLayer = -999;

	ComponentMap<BackgroundComponent> *backgrounds = GetGameSpace()->GetComponentMap<BackgroundComponent>();
	for (auto& bgHandle : *backgrounds)
	{
		ComponentHandle<TransformComponent> transform = bgHandle.GetSiblingComponent<TransformComponent>();
		if (!transform.IsActive())
		{
			continue;
		}

		int layer = static_cast<int>(transform->GetZLayer());
		if (lastLayer != layer)
		{
			Screen::GetLayerFrameBuffer(layer)->Use();
			lastLayer = layer;
		}

		if(bgHandle->IsBackground())
			bgHandle->Render(camPos);
	}
}

void RenderSystem::RenderForegrounds(float dt)
{
	glm::vec2 camPos = Camera::GetActiveCamera()->GetPosition();
	int lastLayer = -999;

	ComponentMap<BackgroundComponent> *backgrounds = GetGameSpace()->GetComponentMap<BackgroundComponent>();
	for (auto& bgHandle : *backgrounds)
	{
		ComponentHandle<TransformComponent> transform = bgHandle.GetSiblingComponent<TransformComponent>();
		if (!transform.IsActive())
		{
			continue;
		}

		int layer = static_cast<int>(transform->GetZLayer());
		if (lastLayer != layer)
		{
			Screen::GetLayerFrameBuffer(layer)->Use();
			lastLayer = layer;
		}

		if (!bgHandle->IsBackground())
			bgHandle->Render(camPos);
	}
}

// Called each frame.
void RenderSystem::Update(float dt)
{
	if (Input::IsPressed(Key::F11))
	{
		ToggleFullscreen();
	}

	// Clear screen and sets correct framebuffer
	Screen::Use();

	//Start Loop
	UpdateCameras(dt);
	RenderBackgrounds(dt);
	RenderSprites(dt);
	RenderParticles(dt);
	RenderForegrounds(dt);

	//End loop
	Screen::Draw(); // Draw to screen and apply post processing effects
	DebugGraphic::DrawAll();
}

void RenderSystem::ResizeWindowEvent(GLFWwindow* window, int w, int h)
{
	width = w;
	height = h;

	Settings::SetScreenSize(w, h);

	Screen::ResizeScreen(width, height);
	resizeCameras = true;
	engine->GetEditor()->ResizeEvent(w, h);
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

void ToggleFullscreen()
{
	static bool isFullscreen = true;

	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);
	if (!isFullscreen)
	{
		// Resize to fullscreen (store old resolution somewhere)
		glfwSetWindowMonitor(engine->GetWindow(), primary, 0, 0, mode->width, mode->height, mode->refreshRate);
		glfwSetInputMode(engine->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		isFullscreen = true;
	}
	else
	{
		// Resize to window (with w=800,h=600)
		glfwSetWindowMonitor(engine->GetWindow(), NULL, mode->width / 2 - 400, mode->height / 2 - 320, 889, 500, mode->refreshRate);
		glfwSetInputMode(engine->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		isFullscreen = false;
	}
}