#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include <map>
#include <queue>

// glfw (OpenGL)
#define NOMINMAX
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <entt.hpp>

// freetype
#include <ft2build.h>
#include FT_FREETYPE_H

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string fonts_path(const std::string& name) {return data_path() + "/fonts/" + std::string(name);}
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};
inline std::string levels_path(const std::string& name) {return data_path() + "/levels/" + std::string(name);};

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
	void reflect();
};

bool gl_has_errors();

extern entt::registry registry;

// Utility functions to help with mouse movement, specifically swiping
struct Mouse_spell {
	//std::vector<vec2> load_gesture(std::string file_name);
	void reset_swipe_status(std::map < std::string,bool> &map, std::string except_button = "None", std::string except_dir = "None");
	void check_spell(std::queue <std::string> &gesture_queue, std::map < int, std::map <std::string, std::string>> &spellbook, bool flag_fast);
	void update_datastructs(std::map<std::string, bool>& gesture_statuses, std::queue<std::string> &gesture_queue, std::vector<vec2> &gesture_coords, std::string mouse_button, bool &flag_fast, float elapsed_ms);
	void reset_spells(std::map < int, std::map <std::string, std::string>> &spellbook);
};

enum class ProgramState {
	INIT,
	MAIN_MENU,
	OPTIONS,
	RESET_GAME,
	LOAD_GAME,
	RUNNING,
	PAUSED,
	GAME_OVER_WIN,
	GAME_OVER_DEAD,
	EXIT,
	CUTSCENE1
};

extern ProgramState state;

// From main.cpp - now globally accessible
extern int window_width_px;
extern int window_height_px;
extern vec2 global_scaling_vector;
extern vec2 map_scale;
extern int cutscene_selection;

// From world_system.cpp - now globally accessible
extern vec2  player_vel;
extern vec2	 default_player_vel;
extern vec2  enemy_vel;
extern vec2  starting_map_pos;
extern vec2  ending_map_pos;
extern bool  do_generate_path;
extern bool  player_swing;
extern bool  player_is_manually_moving;
extern bool  in_a_cutscene;
extern std::map<int, bool> pressed_keys;
extern entt::entity cutscene_minotaur_entity;
extern entt::entity cutscene_drone_entity;
extern entt::entity cutscene_drone_sad_entity;
extern entt::entity cutscene_drone_laughing_entity;
extern entt::entity cutscene_minotaur_rtx_off_entity;
extern entt::entity cutscene_drone_rtx_off_entity;
extern entt::entity background_space2_entity;
extern entt::entity background_space3_entity;
extern std::vector<entt::entity> background_entities;
extern int speed_counter;
extern int wallbreaker_counter;
extern vec2 position_to_map_coords;
extern bool is_within_bounds;
extern int required_num_of_keys;

// From ai_system.cpp - set to false when world_system.cpp detects death
extern bool do_pathfinding_movement;

extern float game_start_time;
extern bool initial_game;

enum class Event {
	PLAYER_ENEMY_COLLISION,
	PLAYER_PREY_COLLISION
};

//From https://gameprogrammingpatterns.com/observer.html
class Observer
{
public:
	virtual ~Observer() {}
	virtual void onNotify(const entt::entity& entity, const entt::entity& other, Event event) = 0;
};

class Subject
{
private:
	static const int MAX_OBS = 3;
	Observer* observers_[MAX_OBS];
	int numObservers_ = 0;
public:
	void addObserver(Observer* observer);

	void removeObserver(Observer* observer);

protected:
	void notify(const entt::entity& entity, const entt::entity& other, Event event);
};
