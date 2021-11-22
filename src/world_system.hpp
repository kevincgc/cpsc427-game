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

extern entt::registry registry;
extern std::map < int, std::map <std::string, std::string>> spellbook;

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
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

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;

	// map coords conversion
	// the vec2 functions do both x and y, the float functions will do only one
	static vec2 map_coords_to_position(vec2 position);
	static vec2 position_to_map_coords(vec2 map_coords);
	static bool tile_is_walkable(MapTile tile);
	static MapTile get_map_tile(vec2 map_coords);
	static bool is_within_bounds(vec2 map_coords);

	// restart level
	void restart_game();

private:


	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button(int button, int action, int mods);

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Game state
	RenderSystem* renderer;
	float next_item_spawn;
	float flash_timer;
	entt::entity player_minotaur;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* player_death_sound;
	Mix_Chunk* player_item_sound;
	Mix_Chunk* tada_sound;
	Mix_Chunk* horse_snort_sound;
	Mix_Chunk* drone_were_it_only_so_easy_sound;
	Mix_Chunk* drone_stupid_boy_sound;


	// entity spawning
	std::vector<vec2> spawnable_tiles;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// maze generation
	void recursiveGenerateMaze(std::vector<std::vector<MapTile>> &maze, int begin_x, int begin_y, int end_x, int end_y);
	std::vector<std::vector<MapTile>> generateProceduralMaze(std::string method, int width, int height, vec2 &start_tile);

	void process_entity_node(YAML::Node node, std::function<void(std::string, vec2)> spawn_callback);
};
