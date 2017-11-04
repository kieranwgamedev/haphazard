/*
FILE: Editor.cpp
PRIMARY AUTHOR: Sweet

Copyright (c) 2017 DigiPen (USA) Corporation.
*/

// http://www.ariel.com/images/animated-messages-small2.gif

#include "Editor.h"
#include "Type_Binds.h"

#include <string>
#include <algorithm>

#include "../Imgui/imgui-setup.h"

#include "GameObjectSystem\GameSpace.h"
#include "GameObjectSystem/GameObject.h"
#include "Engine\Physics\RigidBody.h"
#include "graphics\SpriteComponent.h"
#include "Engine\Physics\Collider2D.h"

#include "Util/Logging.h"

#include "Engine/Engine.h"

#include "Input/Input.h"

#include <locale>
#include <ctype.h>

#include "graphics\DebugGraphic.h"

#include <Windows.h>
#include <psapi.h>


Editor::Editor(Engine *engine, GLFWwindow *window) : m_engine(engine), m_show_editor(false), m_objects(), m_state{ false, -1, -1, false }
{
	m_objects.reserve(256);

	// Style information
	ImGuiStyle * style = &ImGui::GetStyle();

	// Setup the imgui data
	ImGui_ImplGlfwGL3_Init(window, true);

	// Window styles
	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	// Window Colors
	style->Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.75f, 0.48f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.15f, 0.17f, 1.00f);
	style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.29f, 0.37f, 0.94f);

	style->Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.40f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.30f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);

	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.82f, 0.28f, 0.25f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.28f, 0.25f, 1.00f);

	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.32f, 0.22f, 0.22f, 1.00f);

	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.80f, 0.50f, 0.50f, 0.40f);

	style->Colors[ImGuiCol_ComboBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.99f);

	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
	
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
	
	style->Colors[ImGuiCol_Button] = ImVec4(0.82f, 0.28f, 0.25f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.82f, 0.28f, 0.25f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.85f, 0.51f, 0.33f, 1.00f);
	
	style->Colors[ImGuiCol_Header] = ImVec4(0.82f, 0.28f, 0.25f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.82f, 0.28f, 0.25f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.85f, 0.51f, 0.33f, 1.00f);
	
	style->Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
	
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	
	style->Colors[ImGuiCol_CloseButton] = ImVec4(0.50f, 0.50f, 0.90f, 0.50f);
	style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.70f, 0.70f, 0.90f, 0.60f);
	style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	
	style->Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);


	auto help = [this]()
	{
		Editor::Internal_Log("    Commands Avaiable: \n");
		
		for (auto& cmd : m_commands)
		{
			Editor::Internal_Log("     - %s \n", cmd.second.command);
		}
	};
	
	// Help screen bs
	RegisterCommand("?", help);
	RegisterCommand("help", help);

	// Create gameobject
	RegisterCommand("create", [this]() 
	{ 
		std::string name = m_line.substr(strlen("create "));
		QuickCreateGameObject(name.c_str()); 
	});
	
	// Clear the log
	RegisterCommand("clear", [this]() { Clear(); });
	RegisterCommand("cls", [this]() { Clear(); });

	// Exit the game, probably should be editor?
	RegisterCommand("exit", [this]() { m_engine->Exit(); });

	// Display past commands
	RegisterCommand("history",
	[this]()
	{
		Editor::Internal_Log("    Command History: \n");

		for (auto& history : m_log_history)
		{
			Editor::Internal_Log("     - %s \n", history.c_str());
		}
	});

	// Log something to the file
	// RegisterCommand("log", 
	// [this]()
	// {
	// 	Logging::Log(m_line.substr(strlen("log")).c_str());
	// });

	// Display the current active objects
	RegisterCommand("objects",
	[this]()
	{
		Editor::Internal_Log("    Current Objects: \n");
		for (auto& object : m_objects)
		{
			Editor::Internal_Log("     - %d : %s \n", object, GameObject(object).GetComponent<ObjectInfo>().Get()->m_name.c_str());
		}
	});

	// Select an object
	RegisterCommand("select",
	[this]()
	{
		int seleted_id = atoi(m_line.substr(strlen("select ")).c_str());

		for (auto object : m_objects)
		{
			if (object == seleted_id)
			{
				m_selected_object = object;
				break;
			}
		}
	});

	m_log_history.reserve(100);
}


Editor::~Editor()
{
	ImGui_ImplGlfwGL3_Shutdown();
}


void Editor::Update()
{
	// Check if Editor is being shown
	if (m_show_editor)
	{
		if (m_freeze_time)
		{
			m_engine->GetDtObject() = 0;
		}
		else
		{
			m_engine->GetDtObject() = m_engine->CalculateDt();
		}

		// Check for click events
		OnClick();

		// Start a new frame in imgui
		ImGui_ImplGlfwGL3_NewFrame();

		// Get all the active gameobjects
		m_engine->GetSpaceManager()->CollectAllObjectsDelimited(m_objects);

		UpdatePopUps(1 / 60.0f);

		// Top Bar
		MenuBar();

		// Render the console
		if (m_show_settings)
		{
			SettingsPanel(1 / 60.0f);
		}
		
		if (m_show_console)
		{
			Console();
		}

		if (Input::IsPressed(Key::Y))
		{
			m_tool = Editor::Tool::Translation;
		}

		// Move, Scale, Rotate
		Tools();

		// ImGui::ShowTestWindow();

		// Display
		ObjectsList();

		// Pass the current object in the editor
		if (m_multiselect.m_size)
		{
			for (size_t i = 0; i < m_multiselect.m_size; i++)
			{
				ImGui_GameObject_Multi(m_multiselect, this);
			}
		}
		else
		{
			ImGui_GameObject(GameObject(m_selected_object), this);
		}

		#ifdef _DEBUG
			// Please don't delete, super important
			if (rand() % 1000000 == 0)
			{
				extern const char *ErrorList[];
				AddPopUp(PopUpWindow(ErrorList[0], 5.0f, PopUpPosition::Center));
			}
		#endif

		ImGui::Render();
	}
}


void Editor::KeyBindings()
{
	if (Input::IsHeldDown(Key::LeftControl) && Input::IsPressed(Key::Z))
	{
		Undo_Action();
	}

	if (Input::IsPressed(Key::LeftControl) && Input::IsPressed(Key::Y))
	{
		Redo_Action();
	}
}


// Register a command using a lambda
void Editor::RegisterCommand(const char *command, std::function<void()>&& f)
{
	m_commands.emplace(hash(command), Command(command, strlen(command), f));
}


// External Log that displays date
void Editor::Log(const char *log_message, ...)
{
	char time_buffer[64];
	char buffer[512];
	va_list args;
	va_start(args, log_message);

	auto t = std::time(nullptr);
	
	// This is the unsafe function warning
	#pragma warning(disable : 4996)
	strftime(time_buffer, 64, "[%H:%M:%S]", std::localtime(&t));

	vsnprintf(buffer, 512, log_message, args);

	m_log_buffer.append("%s - %s\n", time_buffer, buffer);

	va_end(args);

	m_offsets.push_back(m_log_buffer.size() - 1);

	Logging::Log_Editor(Logging::Channel::CORE, Logging::Priority::MEDIUM_PRIORITY, buffer);

	m_scroll = true;
}


// Log Data, works like printf
// This internal log doesnt show datetime
void Editor::Internal_Log(const char * log_message, ...)
{
	// Add to the log buffer
	va_list args;
	va_start(args, log_message);
	m_log_buffer.appendv(log_message, args);
	va_end(args);
	m_offsets.push_back(m_log_buffer.size() - 1);
	m_scroll = true;
}


// Adds an action to the history
//   Order is Old Value, New Value, Field Name, handle to component, action function
void Editor::Push_Action(EditorAction&& a)
{
	// Size keeps track of the number of actions
	if (m_actions.size)
	{
		// Check if any undo has been done
		if (m_actions.size == m_actions.history.size())
		{
			m_actions.history.emplace_back(a);
			m_actions.size = m_actions.history.size();
		}
		else
		{
			// We need to catch back up to the actual size
			m_actions.history.emplace(m_actions.history.begin() + m_actions.size, a);
			++m_actions.size;
		}
	}
	else
	{
		// Zero actions saved, so just start at the beginning
		m_actions.history.emplace(m_actions.history.begin(), a);
		++m_actions.size;
	}
}


void Editor::Undo_Action()
{
	// Go to previous action
	m_actions.history[m_actions.size - 1].redo = false;

	// Call the resolve function
	m_actions.history[m_actions.size - 1].func(m_actions.history[m_actions.size - 1]);
	--m_actions.size;
}


void Editor::Redo_Action()
{
	// Set the redo flag
	m_actions.history[m_actions.size].redo = true;

	// Call the resolve function
	m_actions.history[m_actions.size].func(m_actions.history[m_actions.size]);

	// We redid an action, so it is valid to be undone
	++m_actions.size;
}


void Editor::QuickCreateGameObject(const char *name, glm::vec2& pos, glm::vec2& size)
{
	if (m_selected_object)
	{
		GameObject object = m_engine->GetSpace(GameObject(m_selected_object).GetIndex())->NewGameObject(name);

		// Add a transform component
		object.AddComponent<TransformComponent>(glm::vec3(pos.x, pos.y, 1.0f), glm::vec3(size.x, size.y, 1.0f));

		m_selected_object = object.Getid();
	}
}


void Editor::SetGameObject(GameObject new_object)
{
	m_selected_object = new_object.Getid();
}


void Editor::OnClick()
{
	// Check for mouse 1 click
	if (Input::IsPressed(Key::Mouse_1) && !ImGui::IsMouseHoveringAnyWindow())
	{
		const glm::vec2 mouse = Input::GetMousePos_World();

		for (auto id : m_objects)
		{
			if (id)
			{
				GameObject object = id;
				ComponentHandle<TransformComponent> transform = object.GetComponent<TransformComponent>();
				const glm::vec2 scale = transform.Get()->GetScale();
				const glm::vec2 pos = transform.Get()->GetPosition();

				if (mouse.x < pos.x + (scale.x / 2) && mouse.x > pos.x - (scale.x / 2))
				{
					if (mouse.y < pos.y + (scale.y / 2) && mouse.y > pos.y - (scale.y / 2))
					{
						// Save the GameObject data
						m_selected_object = transform.GetGameObject().Getid();
					}
				}
			}
		}
	}
}


static glm::vec2 Lerp_Pos(Array<GameObject_ID, MAX_SELECT>& objects)
{
	glm::vec2 lerp;

	for (size_t i = 0; i < objects.m_size; i++)
	{
		// f = t * b + a(1 - t)
		lerp += GameObject(objects[i]).GetComponent<TransformComponent>()->GetPosition();
	}
	
	return lerp / glm::vec2(static_cast<float>(objects.m_size));
}


void Editor::Tools()
{
	if (m_selected_object && GameObject(m_selected_object).GetSpace())
	{
		glm::vec2 pos;
		if (m_multiselect.m_size)
		{
			pos = GameObject(m_selected_object).GetComponent<TransformComponent>().Get()->GetPosition();
		}
		else
		{
			pos = Lerp_Pos(m_multiselect);
		}

		switch (m_tool)
		{
		case Tool::Translation:
			DebugGraphic::DrawShape(pos, glm::vec2(5, 5));
			break;

		case Tool::Scale:
			DebugGraphic::DrawShape(pos, glm::vec2(1, 1), 0.0f, glm::vec4(HexVec(0x64d622), 1));
			break;

		case Tool::Rotation:
			DebugGraphic::DrawShape(pos, glm::vec2(1, 1), 0.0f, glm::vec4(HexVec(0xc722d6), 1));
			break;

		default:
			break;
		}
	}
}


void Editor::AddPopUp(PopUpWindow&& pop)
{
	m_pop_ups.emplace_back(pop);
}


void Editor::UpdatePopUps(float dt)
{
	int width = 0;
	int height = 0;
	glfwGetWindowSize(m_engine->GetWindow(), &width, &height);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_ShowBorders |
		ImGuiWindowFlags_NoScrollbar;


	for (int i = 0; i < m_pop_ups.size(); ++i)
	{
		PopUpWindow& popup = m_pop_ups[i];

		// float offset = m_pop_ups.size() - i;
		glm::vec2 padding(65, 65);
		
		ImVec2 pos;
		ImVec2 size(ImGui::CalcTextSize(popup.message).x * 1.49f, 42);
		float text_padding = size.x;  //strlen(popup.message) * 5.75f;

		// Find where to draw the popup
		switch (popup.pos)
		{
		case PopUpPosition::BottomLeft:
			pos = ImVec2(padding.x, static_cast<float>(height) - padding.y);
			break;

		case PopUpPosition::BottomRight:
			pos = ImVec2(static_cast<float>(width) - padding.x - text_padding, static_cast<float>(height) - padding.y);
			break;

		case PopUpPosition::TopRight:
			pos = ImVec2(static_cast<float>(width) - padding.x - text_padding, padding.y);
			break;

		case PopUpPosition::TopLeft:
			pos = ImVec2(padding.x, padding.y);
			break;

		case PopUpPosition::Center:
			pos = ImVec2(width / 2.0f, height / 2.0f);
			break;
		case PopUpPosition::Mouse:
			pos = Input::GetMousePos();
			break;
		}

		// Check if the popup needs to be removed
		if (popup.timer > 0)
		{
			// f = t * b + a(1 - t)
			constexpr float factor = 0.25f;

			// Do fading when we are close to the end, but none until then
			float alpha = 1.0f;
			if (popup.timer < 0.25f)
			{
				alpha = (popup.timer / popup.max_time) * factor + popup.alpha * (1 - factor);
			}

			popup.timer -= dt;

			// Setup next window, including a special alpha value
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);

			ImGui::Begin(popup.message, nullptr, flags);
			ImGui::PushAllowKeyboardFocus(false);

			ImGui::Text(popup.message);

			// Save the alpha for next frame
			popup.alpha = alpha;

			// Clean Up
			ImGui::PopAllowKeyboardFocus();
			ImGui::End();
			ImGui::PopStyleVar(1);
		}
		else
		{
			m_pop_ups.erase(m_pop_ups.begin() + i);
		}
	}

	// Clean Up
	ImGui::PopStyleVar(1);
}


void Editor::PrintObjects()
{
	// Get all the names of the objects
	char name_buffer[128] = { 0 };
	GameObject object(0);

	for (auto& object_id : m_objects)
	{
		// Use invalid gameObject as a delimiter
		if (object_id == INVALID_GAMEOBJECT_ID)
		{
			ImGui::Separator();
			continue;
		}
		object = object_id;
		std::string& name = object.GetComponent<ObjectInfo>().Get()->m_name;

		// Save the buffer based off name size, max name size is 8
		if (name.size() > 8)
		{
			snprintf(name_buffer, sizeof(name_buffer),
				"%-5.5s... - %d : %d", name.c_str(), object.GetObject_id(), object.GetIndex());
		}
		else
		{
			snprintf(name_buffer, sizeof(name_buffer),
				"%-8.8s - %d : %d", name.c_str(), object.GetObject_id(), object.GetIndex());
		}

		// Multiselect with Left Control + LClick
		if (Input::IsHeldDown(Key::LeftControl))
		{
			// Draw each object
			if (ImGui::Selectable(name_buffer))
			{
				if (m_multiselect.m_size < MAX_SELECT)
				{
					m_multiselect.push_back(object);
				}

				SetGameObject(object);
			}
		}
		else
		{
			if (ImGui::Selectable(name_buffer))
			{
				m_multiselect.clear();
				SetGameObject(object);
				break;
			}
		}		
	}
}


void Editor::ObjectsList()
{
	using namespace ImGui;

	SetNextWindowSize(ImVec2(260, 400));
	SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Once);
	Begin("Objects", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	if (Button("Create"))
	{
		ImGui::OpenPopup("Create GameObject###CreateGameObject");
	} 

	if (ImGui::BeginPopup("Create GameObject###CreateGameObject"))
	{
		static char name[512] = { 'N', 'o', ' ', 'N', 'a', 'm', 'e', '\0' };
		if (ImGui::InputText("Name", name, sizeof(name), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			GameObject object = m_engine->GetSpace(m_current_space_index)->NewGameObject(name);

			// Add a transform component
			object.AddComponent<TransformComponent>();

			m_selected_object = object.Getid();
		}

		ImGui::SliderInt("Space", &m_current_space_index, 0, static_cast<int>(m_engine->GetSpaceManager()->GetSize()) - 1);

		if (ImGui::Button("Create###createObjectListButton"))
		{
			GameObject object = m_engine->GetSpace(m_current_space_index)->NewGameObject(name);

			// Add a transform component
			object.AddComponent<TransformComponent>();

			m_selected_object = object.Getid();
		}

		ImGui::EndPopup();
	}
	
	SameLine();

	if (Button("Add Space"))
	{
		engine->GetSpaceManager()->AddSpace();
	}

	PrintObjects();

	End();
}


void Editor::SetActive(ImGuiTextEditCallbackData *data, size_t entryIndex)
{
	// Copy in the data  from the command
	memmove(data->Buf, m_commands[entryIndex].command, m_commands[entryIndex].cmd_length);

	// Update the Buffer data
	data->Buf[m_commands[entryIndex].cmd_length] = '\0';
	data->BufTextLen = static_cast<int>(m_commands[entryIndex].cmd_length);
	data->BufDirty = true;
}


void Editor::SetActive_History(ImGuiTextEditCallbackData *data, int entryIndex)
{
	// Copy in the data  from the command
	memmove(data->Buf, m_log_history[entryIndex].c_str(), m_log_history[entryIndex].size());

	// Update the Buffer data
	data->Buf[m_log_history[entryIndex].size()] = '\0';
	data->BufTextLen = static_cast<int>(m_log_history[entryIndex].size());
	data->BufDirty = true;
}


void Editor::SetActive_Completion(ImGuiTextEditCallbackData *data, int entryIndex)
{
	// Copy in the data  from the command
	memmove(data->Buf, m_matches[entryIndex], strlen(m_matches[entryIndex]));

	// Update the Buffer data
	data->Buf[strlen(m_matches[entryIndex])] = '\0';
	data->BufTextLen = static_cast<int>(strlen(m_matches[entryIndex]));
	data->BufDirty = true;
}


void SetInput_Blank(ImGuiTextEditCallbackData *data)
{
	// Copy in the data  from the command
	// memmove(data->Buf, "", 0);

	// Update the Buffer data
	data->Buf[0] = '\0';
	data->BufTextLen = 0;
	data->BufDirty = true;
}


// Alternative StrCmp
bool Command_StrCmp(const char *str1, const char *str2)
{
	// Check if the current letters are the same
	while (!(*str1 ^ *str2++))

		// If end of str1 is reached, return true
		if (!*str1++)
			return true;

	return false;
}


// This is from imgui, it returns how much alike two strings are
static int Strnicmp(const char* str1, const char* str2, int n) 
{ 
	int d = 0; 
	while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) 
	{ 
		str1++; 
		str2++;
		n--; 
	} 
	return d; 
}


int Input_Editor(ImGuiTextEditCallbackData *data)
{
	Editor *editor = reinterpret_cast<Editor *>(data->UserData);

	switch (data->EventFlag)
	{
		// When you hit tab
	case ImGuiInputTextFlags_CallbackCompletion:
		if (editor->m_matches.size())
		{
			// Delete the input buffer
			data->DeleteChars(0, static_cast<int>(strlen(data->Buf)));

			// Insert the command into the input buffer
			data->InsertChars(data->CursorPos, editor->m_matches[0], editor->m_matches[0] + strlen(editor->m_matches[0]));
		}
		break;

		// History based data, when you hit up or down arrow keys
	case ImGuiInputTextFlags_CallbackHistory:
			// editor->m_state.m_popUp = true;
			
			// Check if arrow keys are pressed and if the current item is at the end
			if (data->EventKey == ImGuiKey_UpArrow && (editor->m_state.activeIndex < static_cast<int>(editor->m_log_history.size() - 1)))
			{
				// Increase the active index and copy in the command
				editor->m_state.activeIndex++;
				editor->SetActive_History(data, static_cast<int>(editor->m_log_history.size() - editor->m_state.activeIndex) - 1);
			}
			else if (data->EventKey == ImGuiKey_DownArrow && (editor->m_state.activeIndex > -1))
			{
				// Decrease the state and copy in "" if nothing is left, otherwise copy in the command
				editor->m_state.activeIndex--;

				if (editor->m_state.activeIndex == -1)
				{
					SetInput_Blank(data);
				}
				else
				{
					editor->SetActive_History(data, static_cast<int>(editor->m_log_history.size() - editor->m_state.activeIndex) - 1);
				}
			}
		break;

		// This happens all the time
	case ImGuiInputTextFlags_CallbackAlways:
			// Clear the data in the matches vector, but dont free the alloc'd memory
			editor->m_matches.clear();

			// Make sure there is data to read from
			if (data->Buf && data->BufTextLen > 0)
			{
				// If there is data then the popup needs to be drawn
				editor->m_state.m_popUp = true;

				// Get the word from the data
				const char *word_end = data->Buf + data->CursorPos;
				const char *word_start = word_end;

				// Find the beginning and end of the word
				while (word_start > data->Buf)
				{
					// Start at the last letter
					const char c = word_start[-1];

					// End of word
					if (c == '\t' || c == ',' || c == ';')
						break;

					// Move back a letter
					--word_start;
				}


				for (auto& command : editor->m_commands)
				{
					if (Strnicmp(command.second.command, word_start, static_cast<int>((word_end - word_start))) == 0)
					{
						editor->m_matches.push_back(command.second.command);
					}
				}


				// Check if there were matches
				if (editor->m_matches.Data)
				{
					// Get the length of the match
					int length = static_cast<int>(word_end - word_start);

					// Check for length on matches
					while (length)
					{
						char character = 0;
						bool all_matches_made = true;

						for (int i = 0; i < editor->m_matches.Size && all_matches_made; ++i)
						{
							if (i == 0 && length < editor->m_matches.Size)
							{
								character = editor->m_matches[i][length];
							}
							else if (character == 0 || character != editor->m_matches[i][length])
							{
								all_matches_made = false;
							}
						}

						if (!all_matches_made)
						{
							break;
						}
						length++;
					}

					if (length > 0)
					{
						data->DeleteChars(static_cast<int>(word_start - data->Buf), static_cast<int>(word_end - word_start));
						data->InsertChars(data->CursorPos, editor->m_matches[0], editor->m_matches[0] + length);
					}
				}
			}

			// Get the clicked indexed item and reset the popup state
			if (editor->m_state.clickedIndex != -1)
			{
				editor->SetActive(data, editor->m_state.clickedIndex);
				editor->m_state.activeIndex = -1;
				editor->m_state.clickedIndex = -1;
				editor->m_state.m_popUp = false;
			}
		break;

		// When you type a new character; here for possible future use
	//case ImGuiInputTextFlags_CallbackCharFilter:
		//break;

	default:
		break;
	}

	return 0;
}


void Editor::ToggleEditor()
{
	if (m_freeze_time)
	{
		float& dt = m_engine->GetDtObject();

		dt = dt ? 0 : (1 / 60.0f);
	}

	m_show_editor = !m_show_editor;
}


void Editor::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				m_save = true;
			}

			if (ImGui::MenuItem("Load"))
			{
				m_load = true;
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (m_actions.size)
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z"))
				{
					Undo_Action();
				}
			}
			else
			{
				ImGui::MenuItem("Undo", "CTRL+Z", false, false);
			}
			
			if (m_actions.history.size() != m_actions.size)
			{
				if (ImGui::MenuItem("Redo", "CTRL+Y"))
				{
					Redo_Action();
				}
			}
			else
			{
				ImGui::MenuItem("Redo", "CTRL+Y", false, false);
			}

			ImGui::EndMenu();
		}

		if (ImGui::Button("Settings"))
		{
			m_show_settings = !m_show_settings;
		}

		SaveLoad();

		ImGui::EndMainMenuBar();
	}

}


static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
	static unsigned long long _previousTotalTicks = 0;
	static unsigned long long _previousIdleTicks = 0;

	unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
	unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

	float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? static_cast<float>(idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

	_previousTotalTicks = totalTicks;
	_previousIdleTicks = idleTicks;
	return ret;
}


void Editor::SaveLoad()
{
	if (m_save)
	{
		ImGui::OpenPopup("##menu_save_pop_up");
	}
	else if (m_load)
	{
		ImGui::OpenPopup("##menu_load_pop_up");
	}


	if (ImGui::BeginPopup("##menu_save_pop_up"))
	{
		ImGui::PushItemWidth(180);
		if (ImGui::InputText("Filename", m_filename, 128, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			engine->FileSave(m_filename);
			AddPopUp(PopUpWindow("Game Saved", 2.0f, PopUpPosition::BottomRight));
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Save"))
		{
			engine->FileSave(m_filename);
			AddPopUp(PopUpWindow("Game Saved", 2.0f, PopUpPosition::BottomRight));
			ImGui::CloseCurrentPopup();
		}
		m_save = false;
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("##menu_load_pop_up"))
	{
		ImGui::PushItemWidth(180);
		if (ImGui::InputText("Filename", m_filename, 128, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			engine->FileLoad(m_filename);
			AddPopUp(PopUpWindow("Loaded", 2.0f, PopUpPosition::Mouse));
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Load"))
		{
			engine->FileLoad(m_filename);
			AddPopUp(PopUpWindow("Loaded", 2.0f, PopUpPosition::Mouse));
			ImGui::CloseCurrentPopup();
		}
		m_load = false;
		ImGui::EndPopup();
	}
}


static unsigned long long FileTimeToInt64(const FILETIME & ft) 
{
	return (static_cast<unsigned long long>(ft.dwHighDateTime) << 32) | static_cast<unsigned long long>(ft.dwLowDateTime); 
}

// Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
// You'll need to call this at regular intervals, since it measures the load between
// the previous call and the current one.  Returns -1.0 on error.
float GetCPULoad()
{
	FILETIME idleTime, kernelTime, userTime;
	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
}


void Editor::SettingsPanel(float dt)
{
	using namespace ImGui;
	static float timer = 0.0f;
	static float CPU = GetCPULoad() * 100.0f;

	SetNextWindowSize(ImVec2(250, 400));
	Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.WorkingSetSize;

	Text("RAM: %u MB", virtualMemUsedByMe / (1024 * 1024));
	Text("CPU: %f%%", m_cpu_load[0]);

	PlotLines("", m_cpu_load.m_array, _countof(m_cpu_load.m_array), 0, nullptr, 0, 100, ImVec2(0, 50));

	timer += dt;
	if (timer > 0.75f)
	{
		m_cpu_load.push_back_pop_front(GetCPULoad() * 100.0f);
		if (m_cpu_peak < m_cpu_load[m_cpu_load.m_size - 1])
		{
			m_cpu_peak = m_cpu_load[m_cpu_load.m_size - 1];
		}
		timer = 0.0f;
	}

	if (Button("Save##editor_settings"))
	{
		m_save = true;
	}
	SameLine();
	if (Button("Load##editor_settings"))
	{
		m_load = true;
	}

	if (Button("Play/Pause##editor_panel"))
	{
		m_freeze_time = !m_freeze_time;
	}
	SameLine();
	if (ImGui::Button("Console"))
	{
		m_show_console = !m_show_console;
	}

	End();
}


bool Editor::PopUp(ImVec2& pos, ImVec2& size)
{
	// Make sure the popup needs to be seen
	if (!m_state.m_popUp || !m_matches.size())
		return false;

	ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoTitleBar          | 
        ImGuiWindowFlags_NoResize            |
        ImGuiWindowFlags_NoMove              |
        ImGuiWindowFlags_HorizontalScrollbar |
        ImGuiWindowFlags_NoSavedSettings     |
        ImGuiWindowFlags_ShowBorders;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

	// Popup begin
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::Begin("console_popup", nullptr, flags);
	
	ImGui::PushAllowKeyboardFocus(false);

	// Go through the matches found from the input text
	for (auto i = 0; i < m_matches.size(); ++i)
	{
		// Check if the item is the hovered or active for colors
		bool isActiveIndex = m_state.activeIndex == i;
		if (isActiveIndex)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(HexVecA(0x072f70FF)));
		}
		
		// Display the options as a selectable text button
		ImGui::PushID(i);
		if (ImGui::Selectable(m_matches[i], m_state.activeIndex))
		{
			m_state.clickedIndex = i;
		}
		ImGui::PopID();

		if (isActiveIndex)
		{
			if (m_state.m_selection_change)
			{
				ImGui::SetScrollHere();
				m_state.m_selection_change = false;
			}

			ImGui::PopStyleColor(1);
		}
	}

	bool isFocuesed = ImGui::IsRootWindowFocused();

	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	ImGui::PopStyleVar(1);
	return isFocuesed;
}


void Editor::Console()
{
	// Setup a char * buffer and tell imgui to draw the console the first time in the center
	char command_buffer[1024] = { 0 };
	ImGui::SetNextWindowPosCenter(ImGuiSetCond_FirstUseEver);

	// Setup the console window flags
	int winflags = 0;
	if (m_state.m_popUp)
	{
		winflags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	}

	ImGui::SetNextWindowSize(ImVec2(800, 550), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Console", nullptr, winflags);

	if (ImGui::Button("Clear Log"))
	{
		Clear();
	}

	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
	if (ImGui::Button("Copy"))
	{
		ImGui::LogToClipboard();
	}
	ImGui::PopStyleVar();
	
	ImGui::SameLine();
	m_log_filter.Draw("Filter", -100.0f);

	// The log box itself
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::BeginChild("scrolling", ImVec2(size.x * 0.95f, size.y * 0.8f));

	// Check if any filters are active and need to be applied
	if (m_log_filter.IsActive())
	{
		// Get the start of the data
		const char *buf_begin = m_log_buffer.begin();
		const char *line = buf_begin;

		// Go through the data and 
		for (int line_no = 0; line != nullptr; line_no++)
		{
			// Get the data
			const char *line_end = (line_no < m_offsets.Size) ? buf_begin + m_offsets[line_no] : nullptr;

			// Filter the text
			if (m_log_filter.PassFilter(line, line_end))
			{
				ImGui::TextUnformatted(line, line_end);
			}

			line = line_end && line_end[1] ? line_end + 1 : nullptr;
		}
	}
	else
	{
		// Put the text out without a filter
		ImGui::TextUnformatted(m_log_buffer.begin());
	}
	if (m_scroll)
	{
		ImGui::SetScrollHere(1.0f);
	}
	m_scroll = false;

	ImGui::EndChild();
	ImGui::Separator();



	ImGui::Text("Command:");
	ImGui::SameLine();

	int flags = ImGuiInputTextFlags_EnterReturnsTrue     |
				 ImGuiWindowFlags_NoSavedSettings         |
				 ImGuiInputTextFlags_CallbackAlways       |
				 ImGuiInputTextFlags_CallbackCharFilter   |
				 ImGuiInputTextFlags_CallbackCompletion   |
				 ImGuiInputTextFlags_CallbackHistory;

	if (ImGui::InputText("", command_buffer, 512, flags, Input_Editor, this))
	{
		// Get the line data and save it for parameters
		m_line = command_buffer;

		// Make sure there is data in the line
		if (m_line.size() > 0)
		{
			// Skip leading spaces
			auto first_of_not_space = m_line.find_first_not_of(' ');

			// Extract the command from the line
			std::string command = m_line.substr(first_of_not_space, m_line.find_first_of(' '));

			// Make Everything uppercase
			std::transform(command.begin(), command.end(), command.begin(), ::tolower);

			// Get the hash of the command
			size_t str_hash = hash(command.c_str());

			// Log the command and display it on screen
			if (first_of_not_space != std::string::npos)
			{
				m_line = m_line.substr(first_of_not_space);
				this->Log(m_line.c_str());

				// Save the command for history
				if (m_log_history.size() < 100)
				{
					m_log_history.emplace_back(m_line.c_str());
				}
			}

			// Check if the popup was clicked and copy the data from the clicked item to here
			if (m_state.m_popUp && m_state.clickedIndex != -1)
			{
				if (m_commands.find(str_hash) != m_commands.end())
				{
					memmove(command_buffer, m_commands[str_hash].command, m_commands[str_hash].cmd_length);
				}
			}
			else
			{
				if (m_commands.find(str_hash) != m_commands.end())
				{
					m_commands.find(str_hash)->second.func();
				}
			}

			// Reset keyboard focus to the console input and reset the state of the popup
			m_state.m_popUp = false;
			m_state.activeIndex = -1;
			ImGui::SetKeyboardFocusHere(-1);
		}
	}

	if (m_state.clickedIndex != -1)
	{
		ImGui::SetKeyboardFocusHere(-1);
		m_state.m_popUp = false;
	}

	if ((ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
	{
		ImGui::SetKeyboardFocusHere(-1);
	}

	// Draw PopUp
	ImVec2 pop_pos(ImGui::GetItemRectMin());
	ImVec2 pop_size(ImGui::GetItemRectSize().x - 60, ImGui::GetItemsLineHeightWithSpacing() * 4);
	bool windowFocus = ImGui::IsRootWindowFocused();

	pop_pos.y += ImGui::GetItemRectSize().y;

	bool popUpFocus = PopUp(pop_pos, pop_size);

	if (!windowFocus && !popUpFocus)
	{
		m_state.m_popUp = false;
	}

	ImGui::End();
}


void Editor::Clear()
{
	if (!m_log_buffer.empty())
	{
		m_log_buffer.clear();
	}
}
