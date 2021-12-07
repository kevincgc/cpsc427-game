#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "components.hpp"

// yaml
#include "yaml-cpp/yaml.h"
#include "yaml_types/vec.hpp"
#include "yaml_types/enums.hpp"

extern entt::registry registry;
extern std::map < int, std::map <std::string, std::string>> spellbook;
extern std::map<ItemType, int> inventory;
extern Item most_recent_collected_item;
extern std::map<std::string, ItemType> item_to_enum;
extern ItemType most_recent_used_item;

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem: public Observer
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// camera
	static vec2 camera;

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);
	void play_sounds();

	// Handle observer events
	void onNotify(const entt::entity& entity, const entt::entity& other, Event event);

	// Should the game be over ?
	bool is_over()const;

	// map coords conversion
	// the vec2 functions do both x and y, the float functions will do only one
	static vec2 map_coords_to_position(vec2 position);
	static vec2 position_to_map_coords(vec2 map_coords);
	static bool tile_is_walkable(MapTile tile);
	static MapTile get_map_tile(vec2 map_coords);
	static bool is_within_bounds(vec2 map_coords);
	void use_wall_breaker();
	//void add_extra_life();
	void use_teleport();
	void use_speed_boost();
	void postItemUse(entt::entity& player);
	// Cutscenes
	void do_cutscene();
	// HUD
	void do_HUD();
	// Timers
	void do_timers(float elapsed_ms_since_last_update);
	// Exit
	void do_exit();
	// Death and Endgame
	bool do_death_and_endgame(float elpased_ms_since_last_update);
	// Remove Off-screen Entities
	void do_cleanup();
	// Tutorial
	void do_tutorial(float elpased_ms_since_last_update);

	// restart level
	void restart_game();

	void save_game();
	void load_game();

	std::vector<std::string> get_leaderboard();
	std::string get_player_time();
	std::string get_level_info();

	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button(int button, int action, int mods);

private:

	void start_game();

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// game time
	double game_time_ms;

	// Game state
	RenderSystem* renderer;
	float next_item_spawn;
	float flash_timer;
	entt::entity player_minotaur;

	// music references
	Mix_Music* background_music;
	std::array<Mix_Chunk *, sound_effect_count> sound_effects;

	// entity spawning
	std::vector<vec2> spawnable_tiles;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// maze generation
	void recursiveGenerateMaze(std::vector<std::vector<MapTile>> &maze, int begin_x, int begin_y, int end_x, int end_y);
	std::vector<std::vector<MapTile>> generateProceduralMaze(std::string method, int width, int height, vec2 &start_tile);

	void process_entity_node(YAML::Node node, std::function<void(std::string, vec2)> spawn_callback, float multiplier);

	double current_finish_time;
	std::vector<std::string> current_leaderboard;
};