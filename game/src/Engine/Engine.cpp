/*
FILE: Engine.cpp
PRIMARY AUTHOR: Sweet

Engine class data, no work really in here yet.

Copyright (c) 2017 DigiPen (USA) Corporation.
*/
#pragma once

#include "Universal.h"
#include "Engine.h"

#include "SOIL\SOIL.h"
#include "Audio/AudioEngine.h"
#include "meta/meta.h"
#include "../Util/FrameCap.h"
#include "Util/Serialization.h"

// Graphics libraries
#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "graphics\Shaders.h" // Shaders need to be initialized
#include "graphics\Settings.h" // Settings needed for window init
#include "graphics\Texture.h"
#include "graphics\RenderLayer.h"

// imgui
#include <imgui.h>

// GameObjects
#include "GameObjectSystem/GameObject.h"

// Component types to register.
#include "GameObjectSystem/TransformComponent.h"
#include "graphics\SpriteComponent.h"
#include "graphics\Texture.h"
#include "Physics\RigidBody.h"
#include "Physics\Collider2D.h"
#include "Scripting\ScriptComponent.h"

#include "input\Input.h"

#include "graphics\Text.h"
#include "graphics\Camera.h"
#include "graphics\Particles.h"
#include "graphics\Background.h"

#define DTLIMIT .033333f

GLFWwindow* WindowInit(); 

// Systems to register.
#include "graphics\RenderSystem.h"
#include "Physics\PhysicsSystem.h"
#include "Scripting\ScriptSystem.h"

extern Engine *engine;


Init_EnginePointer::Init_EnginePointer(Engine *e)
{
	engine = e;
}

				   // Init OpenGL and start window
Engine::Engine() : m_init(this), m_window(WindowInit()), m_editor(this, m_window), m_spaces(), m_windowFocused(true)
{
	m_WindowTitle.reserve(256);

	m_resManager.Init();

	// Load Shaders
	Shaders::Init();

	Audio::Init();
	meta::Init();
	Input::Init(m_window);

	Logging::Log(Logging::CORE, Logging::LOW_PRIORITY, "Engine constructor called. ");


	m_spaces.AddSpace();
	// Initialize the system.
	m_spaces[0]->Init();

	///
	// Loading Screen
	///
	Screen::RenderLoadingScreen();

	// For debug purposes.
	// TODO: Come up with a smarter resource loading strategy.
	m_resManager.LoadAll();

	LoadLevel("defaultLevel.json");
}


void Engine::MainLoop()
{
	Logging::Log(Logging::CORE, Logging::LOW_PRIORITY, "Entering main loop. ");
	while (m_running)
	{
		Update();
	}
	Logging::Exit();
}

// for recording framerate
float timeCounter = 0;
int frameCounter = 0;

#ifdef SHORTSTACK_DEV
static bool editorEnablable = true;
#else
static bool editorEnablable = false;
static float editorEnableTimer = 0.0f;
const static float EDITOR_ENABLE_TIMER = 3.0f;
#endif

// Keep track of if the game is focused.
static bool m_focused = true;


void Engine::Update()
{
	UpdateWindowFocus();

	if (!m_windowFocused)
	{
		glfwPollEvents();
		return;
	}

	Timer frameCap;

	if (m_fileLoadFlag)
	{
		FileLoadInternal(m_fileToLoad.c_str());
		m_fileLoadFlag = false;
	}

	if (glfwWindowShouldClose(m_window))
		m_running = false;

	m_dt = CalculateDt();

	if ( m_paused || (m_editor.GetEditorState().show && m_editor.GetEditorState().freeze))
	{
		m_spaces[0]->Update(0);
	}
	else
		m_spaces[0]->Update(m_dt);

	Input::Update();

	Audio::Update();

	#ifndef SHORTSTACK_DEV
	if ((Input::IsHeldDown(Key::LeftControl) || Input::IsHeldDown(Key::RightControl)) && Input::IsHeldDown(Key::GraveAccent))
	{
		editorEnableTimer += m_dt;
		if (!editorEnablable && editorEnableTimer >= EDITOR_ENABLE_TIMER)
		{
			Audio::PlaySound("default.wav");
			editorEnablable = true;
		}
	}
	else
	{
		editorEnableTimer = 0.0f;
	}
	#endif // !SHORSTACK_DEV


	if (editorEnablable && Input::IsPressed(Key::GraveAccent))
	{
		m_editor.ToggleEditor();
	}
	m_editor.Update(m_dt);
	
	glfwSwapBuffers(m_window);
	glfwPollEvents();

	m_spaces.DeleteObjects();

	frameCap.waitUntil(16666);

	++frameCounter;
	timeCounter += frameCap.timePassed();

	if (timeCounter >= 1'000'000.0f)
	{	
		#ifdef SHORTSTACK_DEV
			if (m_editor.GetSettings().infoOnTitleBar)
			{
				m_WindowTitle += "Shortstack | FrameRate: ";
				m_WindowTitle += std::to_string(frameCounter);
				m_WindowTitle += "    Dt: ";
				m_WindowTitle += std::to_string(Dt());

				m_WindowTitle += ' ';
				m_WindowTitle += m_WindowAppend;
				m_WindowTitle += m_editor.GetSaveTitle();
			}
			else
			{
				m_WindowTitle += "Shortstack";
			}
		#else // !SHORTSTACK_DEV
			m_WindowTitle += "Shortstack";
		#endif
		glfwSetWindowTitle(m_window, m_WindowTitle.c_str());

		m_WindowTitle.clear();
		m_WindowAppend.clear();

		frameCounter = 0;
		timeCounter -= 1000000;
	}

	Input::RecordMousePos();
}


std::string Engine::StringSave()
{
	logger << "Saving Game -> String";
	// Make a document for the allocator.
	// TODO: Figure out how to get an allocator without bothering with a whole document.
	rapidjson::Document doc;

	return JsonToPrettyString(meta::Serialize(*this, doc.GetAllocator()));
}

void Engine::FileSave(const char *fileName)
{
	logger << "Saving Game -> File: " << fileName;

	// Make a document for the allocator.
	// TODO: Figure out how to get an allocator without bothering with a whole document.
	rapidjson::Document doc;

	return JsonToFile(meta::Serialize(*this, doc.GetAllocator()), fileName);
}

void Engine::FileSaveCompact(const char *fileName)
{
	logger << "Saving Game -> File: " << fileName;

	// Make a document for the allocator.
	// TODO: Figure out how to get an allocator without bothering with a whole document.
	rapidjson::Document doc;

	return JsonToFile(meta::Serialize(*this, doc.GetAllocator()), fileName);
}

void Engine::StringLoad(const char *jsonString)
{
	logger << "Loading game -> String";
	rapidjson::Document doc;
	doc.Parse(jsonString);
}

void Engine::FileLoad(const char *fileName)
{
	m_fileLoadFlag = true;
	m_fileToLoad = fileName;
}

float Engine::CalculateDt()
{

	static float last = (float)glfwGetTime();

	float currentFrame = (float)glfwGetTime();
	float dt = currentFrame - last;
	last = currentFrame;

	if (dt > DTLIMIT)
	{
		return DTLIMIT;
	}

	return dt;
}


void Engine::LoggingInit()
{
	Logging::Init();
}




float Engine::Dt() const
{
	return m_dt;
}


float& Engine::GetDtObject()
{
	return m_dt;
}


void Engine::AppendToWindowTitle(std::string& str)
{
	m_WindowAppend += str;
}


void Engine::AppendToWindowTitle(const char *str)
{
	m_WindowAppend += str;
}


bool Engine::IsWindowTitleDirty() const 
{ 
	return timeCounter >= 999990.9f;
}

void Engine::UpdateWindowFocus()
{
	bool windowActuallyFocused = glfwGetWindowAttrib(m_window, GLFW_FOCUSED);

	// If the window just became focused.
	if (windowActuallyFocused && !m_windowFocused)
	{
		Audio::SetMuted(false);
	}

	// If the window just became unfocused.
	if (!windowActuallyFocused && m_windowFocused)
	{
		Audio::SetMuted(true);
	}

	m_windowFocused = windowActuallyFocused;
}

void Engine::FileLoadInternal(const char * fileName)
{
	logger << "Loading Game -> File: " << fileName;
	Screen::RenderLoadingScreen();

	ClearSerializedIdRelationships();
	rapidjson::Document doc = LoadJsonFile(fileName);

	meta::DeserializeAssign(*this, doc);
	ApplySerializedIdUpdates();

	m_currentLevel = fileName;
}



GLFWwindow *WindowInit()
{
	Logging::Log_StartUp("Initializing glfw...", Logging::GRAPHICS, Logging::MEDIUM_PRIORITY);

	if (glfwInit() == false)
	{
		glfwTerminate();
		fprintf(stderr, "Could not init GLFW");
		exit(1);
	}

	Logging::Log_StartUp("Initialized glfw", Logging::GRAPHICS, Logging::MEDIUM_PRIORITY);

	glfwWindowHint(GLFW_SAMPLES, 4); //4 MSAA
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_REFRESH_RATE, 120);

	// Disable resize
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(Settings::ScreenWidth(), Settings::ScreenHeight(), "Shortstack", NULL, NULL);
	
	GLFWimage icons[2];
	icons[0].pixels = SOIL_load_image("assets\\SetupIcon.png", &icons[0].width, &icons[0].height, nullptr, SOIL_LOAD_AUTO);
	icons[1].pixels = SOIL_load_image("assets\\SetupIconSmall.png", &icons[1].width, &icons[1].height, nullptr, SOIL_LOAD_AUTO);
	glfwSetWindowIcon(window, 2, icons);
	SOIL_free_image_data(icons[0].pixels);
	SOIL_free_image_data(icons[1].pixels);

	Logging::Log_StartUp("Window created", Logging::GRAPHICS, Logging::MEDIUM_PRIORITY);

	if (!window)
	{
		glfwTerminate();
		Logging::Log_StartUp("Could not create window", Logging::GRAPHICS, Logging::CRITICAL_PRIORITY);
		exit(1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		Logging::Log_StartUp("Could not init GLEW", Logging::GRAPHICS, Logging::CRITICAL_PRIORITY);
		exit(1);
	}

	glfwSetWindowSizeCallback(window, RenderSystem::ResizeWindowEvent);

	RenderSystem::ResizeWindowEvent(window, Settings::ScreenWidth(), Settings::ScreenHeight());

	// Make the game fullscreen by default in release build.
#ifndef SHORTSTACK_DEV
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);
	glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif
	

	return window;
}
