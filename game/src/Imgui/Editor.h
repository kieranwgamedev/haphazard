/*
          FILE: Editor.h
PRIMARY AUTHOR: Sweet

Copyright � 2017 DigiPen (USA) Corporation.
*/
#pragma once

#include <imgui.h>

#include <vector>
#include <map>
#include <functional>


#include "GameObjectSystem/GameObject.h"
#include "GameObjectSystem/GameSpace.h"

#include <glm/detail/type_vec2.hpp>
#include "Util/DataStructures/Array/Array.h"

#include "meta/meta.h"

#include "graphics/Camera.h"

class Engine;
class TransformComponent;
struct GLFWwindow;

// These Macros allow for hex color codes
// Enter Hex code not including the 0x
// Expands to the parameters of a glm::vec4(R, G, B, A)
#define HexVecA(HEX) (static_cast<float>(0xFF000000 & HEX) / 0xFF000000), \
	                 (static_cast<float>(0x00FF0000 & HEX) / 0x00FF0000), \
	                 (static_cast<float>(0x0000FF00 & HEX) / 0x0000FF00), \
	                 (static_cast<float>(0x000000FF & HEX) / 0x000000FF)

// Expands to the parameters of glm::vec3(R, G, B)
#define HexVec(HEX) (static_cast<float>(0xFF000000 & HEX) / 0xFF000000), \
	                (static_cast<float>(0x00FF0000 & HEX) / 0x00FF0000), \
	                (static_cast<float>(0x0000FF00 & HEX) / 0x0000FF00)

// Maximum number of objects to be selected
#define MAX_SELECT 10

struct EditorAction;
typedef void(*actionFunc)(EditorAction& a);

struct EditorComponentHandle
{
	template <class T>
	explicit operator T() { return T(id, isValid); }

	GameObject_ID id;
	bool isValid;
};

struct EditorAction
{
	// Old Value
	meta::Any save;
	
	// New Value
	meta::Any current;
	
	// Name of field in the class
	const char *name;

	// Handle to the object
	EditorComponentHandle handle;

	// Action to undo/redo the action
	actionFunc func;

	// Sets to true when redo is called
	bool redo;
};


// All of these are Window Positions
enum PopUpPosition
{
	TopLeft,
	TopRight,
	BottomRight,
	BottomLeft,
	Center, 
	Mouse,
	Count
};
struct PopUpWindow
{
	// Constructor with all requirements of data, alpha always starts at 1
	PopUpWindow(const char *msg, float time, PopUpPosition position) 
		: message(msg), timer(time), max_time(time), alpha(1), pos(position) 
	{
		logger << "[EDITOR] PopUp Window: " << msg << "\n";
	}

	// Message to display on screen
	std::string message;

	// Current Life of the window
	float timer;

	// The maximum life of the window
	float max_time;

	// Alpha value
	float alpha;

	// Position on screen
	PopUpPosition pos;
};


class Editor
{
	friend void ImGui_Transform(TransformComponent *transform, GameObject object, Editor *editor);
	friend bool Choose_Parent_ObjectList(Editor *editor, TransformComponent *transform, GameObject child);

	// Editor
	Camera *prev_camera = nullptr;
	Camera *m_editor_cam = nullptr;
	struct EditorState
	{
		bool first_update = true;
		bool show = false;
		bool exiting = false;
		bool reload = false;

		bool freeze = true;

		bool console = false;  // Show/Hide Console
		bool settings = true;
		bool objectList = true;
		bool ppfx = false;

	private:
		friend class Editor;

		// File IO State
		bool fileSave = false;
		bool fileLoad = false;
		bool fileOpened = false;	 // If the file was opened
		bool fileNewFile = true;	 // Is this a new file
		bool fileDirty = false;      // File needs to be saved

		float saveInterval = 5.0f;   // Save Invterval in minutes
		float saveTimer = 0.0f;      // How long since last save

		bool imguiWantMouse = false; // Mouse Over imgui
		bool MouseDragClick = false; // Mouse Click

		bool MouseCameraDragClick   = false; // User is moving camera via mouse
	} m_editorState;


	// System
	// --------------

	// CPU
	Array<float, 30> m_cpu_load = Array<float, 30>(0.0f);
	float m_cpu_peak = 0.0f;

	// Mouse
	glm::vec2 m_prevMouse;



	// Engine
	// --------------
	Engine *m_engine;

	// UI Scale
	// --------------
	static float globalScale;
	void UpdateGlobalScale();

	// Settings
	// --------------
	struct EditorSettings
	{
		bool default_collider_match_scale = true;

		// Camera settings.
		float cameraSpeed = 50.0f;
		float cameraArrowSpeed = 3.8f;
		float cameraZoom  = 3.0f;

		// Snap settings.
		bool snap = false;
		bool absoluteSnap = false;
		float snapInterval = 1.0f;
		float rotationSnapInterval = 15.0f;

		bool selectWithTransformTools = false;

		bool infoOnTitleBar = true;

	} m_editorSettings;

	// Save/Load
	// --------------
	std::string m_filename = "NoFile.json";


	// GameObject Selection
	// --------------
	int m_current_space_index = 0;

	// Current GameObject(s)
	GameObject m_selected_object = 0;
	std::vector<std::pair<GameObject, glm::vec2>> m_multiselect; // vec2 is offset from m_selected_object

	bool m_select_dragging = false;
	glm::vec2 m_drag_select_start;

	// Save spot for string editing
	std::string m_name;

	// List of delimited GameObjects
	std::vector<GameObject_ID> m_objects;

	//GameSpace m_deleted_objects;

	// Undo/Redo Actions
	// --------------
	struct Actions
	{
		std::vector<EditorAction> history;
		size_t size = 0;
	} m_actions;


	// Gizmos
	// --------------
	enum Gizmo
	{
		none,
		Translation, // Move Things
		Scale,       // Make Things bigger/smaller
		Rotation     // Rotate Things
	};

	// Used to determine which direction to scale in
	enum EditorGizmoDirection
	{
		Invalid = -1,
		Dir_X   = 0,
		Dir_Y   = 1,
		Both    = 2
	} m_scaleDir = Both, m_transformDir = Both;

	Gizmo m_gizmo = none;


	// PopUps
	// --------------

	// Update all the popups
	void UpdatePopUps(float dt);

	// Save Location of all popups -- vector works since usually not many popups
	std::vector<PopUpWindow> m_pop_ups;


	// Search Bars for Sprites
	struct SearchBars
	{
		ImGuiTextFilter objects;

		ImGuiTextFilter script;

		ImGuiTextFilter sprite;
		ImGuiTextFilter particles;
		ImGuiTextFilter background;
	} m_searches;

public:
	const EditorSettings& GetSettings()
	{
		return m_editorSettings;
	}

	// Console
	// --------------
	struct Console
	{
	private:

		static void SetInput_Blank(ImGuiTextEditCallbackData *data);
		static bool Command_StrCmp(const char *str1, const char *str2);
		static int Strnicmp(const char* str1, const char* str2, int n);

		// Console Matches PopUp
		bool PopUp(ImVec2& pos, ImVec2& size);

	public:
		static int Input_Callback(ImGuiTextEditCallbackData *data);

		// Draw the Console
		void Draw();

		// Register a command with the console
		void RegisterCommand(const char *command, std::function<void()>&& f);
		
		// Clears log buffers
		void Clear();
		
		// Log with DateTime
		void Log(const char *log_message, ...);

		// Log without DateTime
		void Internal_Log(const char * log_message, ...);


		// Current line
		std::string m_line;

		struct Command
		{
			Command() : Command(nullptr, 0, std::function<void()>()) {}
			Command(const char *cmd, std::function<void()>&& f) : Command(cmd, strlen(cmd), f) {}

			// L-value and R-value
			Command(const char *cmd, size_t len, std::function<void()>& f) : command(cmd), cmd_length(len), func(f) {}
			Command(const char *cmd, size_t len, std::function<void()>&& f) : command(cmd), cmd_length(len), func(f) {}

			// Text to invoke the command
			const char * command = nullptr;

			// Length of command text
			size_t cmd_length = 0;

			// Function/Functor to call
			std::function<void()> func =
				[this]()
			{
				Assert(!command && "No Command created");

				Assert(cmd_length && "Length not set");

				Assert(true && "No function given");
			};
		};

		// Scroll to the bottom of the log
		bool m_scroll = false;

		// Map of the Commands, key is command string hash
		std::map<std::size_t, Command> m_commands;

		// History of lines
		std::vector<std::string> m_log_history;

		// Matches of current input
		ImVector<const char *> m_matches;

		// These set the active input buffer
		void SetActive_Completion(ImGuiTextEditCallbackData *data, int entryIndex);
		void SetActive_History(ImGuiTextEditCallbackData *data, int entryIndex);
		void SetActive(ImGuiTextEditCallbackData *data, size_t entryIndex);

		// State of the popup
		struct State
		{
			bool m_popUp             = false;
			int activeIndex          = -1;
			int clickedIndex         = -1;
			bool m_selection_change  = false;
		} m_state;

		// Save Location for the log
		ImGuiTextBuffer m_log_buffer;

		// Save for filter of log
		ImGuiTextFilter m_log_filter;

		// Offsets of line height
		ImVector<int>   m_offsets;

	} m_Console;

	// Keeps the number of popups at each location
	int m_PopUpCount[PopUpPosition::Count] = { 0 };
	/*
		int TopLeft     = 0;
		int TopRight    = 0;
		int BottomRight = 0;
		int BottomLeft  = 0;
		int Center      = 0;
		int Mouse       = 0;
	*/

	// PPFX
	// --------------
	bool m_show_ppfx = false;

private:
	friend int Editor::Console::Input_Callback(ImGuiTextEditCallbackData *data);

	// Creates a GameObject in the Selected GameObject's space
	void QuickCreateGameObject(const char *name, glm::vec2& pos, glm::vec2& size = glm::vec2(1, 1));
	void QuickCreateGameObject(const char *name);
	
	// ObjectsList Panel
	void ObjectsList();

	// File IO
	void SaveLoad();
	void OpenLevel();
	void SaveLevel();
	void AutoSave(float dt);

	// ImGui
	void MenuBar();
	void SettingsPanel(float dt);
	void PPFX();

	// Click Handling
	void TrySelect(const glm::vec2& mouse);
	void OnClick();
	void DrawDragBox();
	void OnClickRelease();
	void SortObjectList();

	// Keypresses
	void KeyBindings(float dt);

	// Undo/Redo
	void Undo_Action();
	void Redo_Action();

	int GetPopUpCount(PopUpPosition pos);
	void IncrementPopUpCount(PopUpPosition pos);
	void DecrementPopUpCount(PopUpPosition pos);

public:
	Editor(Engine *engine, GLFWwindow *window);
	~Editor();

	// Continues the editor process
	void Update(float dt);

	std::string GetSaveTitle() const;

	// Function call for window size changing
	void ResizeEvent(int w, int h);

	// Works like printf -- for display_date use true
	void Log(const char *log_message, ...);
	
	// No timestamp
	void Internal_Log(const char *log_message, ...);

	// Undo/Redo Fucntions
	void Push_Action(EditorAction&& a);

	// Adds a popups to the list
	void AddPopUp(PopUpWindow&& pop);

	// Change the Selected GameObject
	void SetGameObject(GameObject new_object);

	// Get the editor's current object
	GameObject GetSelectedObject() const { return m_selected_object; }

	// Show the GameObjects in a list
	void PrintObjects();

	// Show/Hide the Editor
	void ToggleEditor();

	// Gizmos
	void Tools();

	// Add a command to the console
	void RegisterCommand(const char *command, std::function<void()>&& f);

	// Default Editor Style
	static void ResetStyle();

	//GameSpace& GetDeletedObjects() { return m_deleted_objects; }

	Editor::SearchBars& GetSearchBars() { return m_searches; }

	// Returns the EditorState for use externally
	EditorState& GetEditorState() { return m_editorState; }

	void Reload() { m_editorState.reload = true; m_editorState.exiting = true; }

	glm::vec2 GetCamPos();

	// Sets/gets the global UI scale.
	static float GetUiScale();
	static void SetUiScale(float scale);

};
