// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "ai_system.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include "physics_system.hpp"

// myLibs
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <iterator>
#include <string>
#include <chrono>
using Clock = std::chrono::high_resolution_clock;

// yaml
#include "yaml-cpp/yaml.h"

// Game configuration
const size_t MAX_TURTLES     = 1;
const size_t MAX_FISH        = 1;
const size_t TURTLE_DELAY_MS = 2000 * 3;
const size_t FISH_DELAY_MS   = 5000 * 3;
const size_t ITEM_DELAY_MS   = 3000 * 3;
vec2 WorldSystem::camera     = {0, 0};
extern float player_vel      = 300.f;
extern float enemy_vel		 = 100.f;
float default_player_vel	 = 300.f;

// My Settings
auto t = Clock::now();
bool flag_right   = false;
bool flag_left    = false;
bool flag_fast    = false;
bool active_spell = false;
float spell_timer = 6000.f;

// For pathfinding feature
bool do_generate_path = false;
vec2 path_target_map_pos;
vec2 starting_map_pos;
vec2 ending_map_pos; 

// !!! hardcoded to 75.f, to be optimized, need to be the same with sprite scale
extern float softshell_scale = 75.f; 

std::queue<std::string> gesture_queue;
std::vector <vec2> gesture_coords_left;
std::vector <vec2> gesture_coords_right;
std::map <std::string, bool> gesture_statuses{
	{"gesture_LMB_up", false},
	{"gesture_LMB_down", false},
	{"gesture_LMB_right", false},
	{"gesture_LMB_left", false},
	{"gesture_RMB_up", false},
	{"gesture_RMB_down", false},
	{"gesture_RMB_right", false},
	{"gesture_RMB_left", false},
};
std::map < int, std::map <std::string, std::string>> spellbook = {
	{1, {
			 {"name", "speedup"},
			 {"speed", "fast"},
			 {"combo_1", "gesture_RMB_right"},
			 {"combo_2", "gesture_RMB_right"},
			 {"active", "false"},
		}
	},
	//{2, {
	//		{"name", "slowdown"},
	//		{"speed", "slow"},
	//		{"combo_1", "gesture_LMB_down"},
	//		{"combo_2", "gesture_RMB_down"},
	//		{"active", "false"}
	//	}
	//},
	{3, {
			{"name", "invincibility"},
			{"speed", "none"},
			{"combo_1", "gesture_LMB_down"},
			{"combo_2", "gesture_RMB_left"},
			{"active", "false"}
		}
	}
};

// helper function to check collision with wall

extern bool collision_with_wall(vec2 position, float scale_x, float scale_y) {
	vec2 corners[] = {
		// upper right
		WorldSystem::position_to_map_coords(position + vec2(scale_x / 2, - scale_y / 2)),

		// upper left
		WorldSystem::position_to_map_coords(position + vec2(-scale_x / 2, -scale_y / 2)),

		// lower left
		WorldSystem::position_to_map_coords(position + vec2(-scale_x / 2, scale_y / 2)),

		// lower right
		WorldSystem::position_to_map_coords(position + vec2(scale_x/ 2, scale_y/ 2)),
	};

	bool collision = false;

	for (const auto corner : corners) {
		const MapTile tile = WorldSystem::get_map_tile(corner);

		if (tile != MapTile::FREE_SPACE || corner.x < 0 || corner.y < 0) {
			collision = true;
			break;
		}
	}

	return collision;
}

// Access mouse_spell helper functions
Mouse_spell mouse_spell;

//Debugging
vec2 debug_pos = { 0,0 };

// Create the world
WorldSystem::WorldSystem()
	: points(0) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window(int width, int height) {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(width, height, "Salmon Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button( _0, _1, _2 ); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("salmon_eat.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("salmon_dead.wav").c_str(),
			audio_path("salmon_eat.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Get the screen dimensions
	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// setting coordinates of camera
	camera.x = registry.get<Motion>(player_minotaur).position.x - screen_width / 2;
	camera.y = registry.get<Motion>(player_minotaur).position.y - screen_height / 2;

	// Remove debug info from the last step
        //while (registry.debugComponents.entities.size() > 0)
        //	registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto motions= registry.view<Motion>();

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	//for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
	//	Motion& motion = motions_registry.components[i];
	//	if (motion.position.x + abs(motion.scale.x) < 0.f) {
	//		registry.remove_all_components_of(motions_registry.entities[i]);
	//	}
	//}

	for (auto entity: motions) {
		//if (entity != player_minotaur) {
		Motion& motion = motions.get<Motion>(entity);
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			registry.destroy(entity);
		}
	 //}
	}

	// Spawning new turtles
	next_turtle_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.view<HardShell>().size() < MAX_TURTLES && next_turtle_spawn < 0.f) {
		// Reset timer
		next_turtle_spawn = (TURTLE_DELAY_MS / 2) + uniform_dist(rng) * (TURTLE_DELAY_MS / 2);
		// init position of enemy
		vec2 position;
		bool collision = true;
		// if the enemy is created on the wall, change it's position till it's not
		while (collision) {
			position = vec2(screen_width -200.f,
				50.f + uniform_dist(rng) * (screen_height - 100.f));
			collision = collision_with_wall(position, softshell_scale, softshell_scale);
		}
		// Create turtle
		entt::entity entity = createTurtle(renderer, position);
		// Setting random initial position and constant velocity
		Motion& motion = registry.get<Motion>(entity);
		motion.mass = 200;
		motion.coeff_rest = 0.9f;

		motion.velocity = vec2(-100.f, 0.f);
	}

	// Slowdown Spell: Adjust enemy type 1 (previously turtle) speed
	//if (gesture_statuses["gesture_LMB_down"] && gesture_statuses["gesture_RMB_down"]) {
	//	for (entt::entity turtle : registry.view<HardShell>()) {
	//		Motion& motion = registry.get<Motion>(turtle);
	//		motion.velocity.x = motion.velocity.x / 2.f;
	//		motion.velocity.y = motion.velocity.y / 2.f;
	//	}
	//}
	//else {
	//	for (entt::entity turtle : registry.view<HardShell>()) {
	//		Motion& motion = registry.get<Motion>(turtle);
	//		//bool positive_x = motion.velocity.x >= 0;
	//		//bool positive_y = motion.velocity.y >= 0;
	//		//if (positive_x) { motion.velocity.x = enemy_vel; }
	//		//else { motion.velocity.x = -1 * enemy_vel; }
	//		//if (positive_y) { motion.velocity.y = enemy_vel; }
	//		//else { motion.velocity.y = -1 * enemy_vel; }

	//		//motion.velocity = vec2(-100.f, 0.f);
	//		// motion.velocity = vec2( (uniform_dist(rng) - 0.5f) * 200,
	//		// 	  (uniform_dist(rng) - 0.5f) * 200);
	//	}
	//}

	// Spawning new fish
	next_fish_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.view<SoftShell>().size() < MAX_FISH && next_fish_spawn < 0.f) {
		// !!!  TODO A1: Create new fish with createFish({0,0}), as for the Turtles above
		next_fish_spawn = (FISH_DELAY_MS / 2) + uniform_dist(rng) * (next_fish_spawn / 2);
		vec2 position;
		bool collision = true;
		// if the enemy is created on the wall, change it's position till it's not
		while (collision) {
			position = vec2(screen_width -200.f,
				50.f + uniform_dist(rng) * (screen_height - 100.f));
			collision = collision_with_wall(position, softshell_scale, softshell_scale);
		}
		entt::entity fish = createFish(renderer, position);
		// Setting random initial position and constant velocity
		Motion& motion = registry.get<Motion>(fish);
		motion.velocity = vec2(-200.f, 0.f);
	}

    float min_counter_ms = 3000.f;
	for (entt::entity entity: registry.view<DeathTimer>()) {
		// progress timer
		DeathTimer& counter = registry.get<DeathTimer>(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.remove<DeathTimer>(entity);
            restart_game();
			return true;
		}
	}

	// Temporary implementation: General timer for spell duration. Later implementation will have spell-specific timers
	// Deactivate spells based on time
	for (auto &spell : spellbook) {
		if (spell.second["active"] == "true") { active_spell = true; }
		if (active_spell) {
			spell_timer -= elapsed_ms_since_last_update;
			if (spell_timer < 0) {
				std::cout << "Spell exhausted" << std::endl;
				mouse_spell.reset_spells(spellbook);
				spell_timer = 10000;
				active_spell = false;
			}
		}
	}

	// Temporary implementation: Handle speed-up spell: Player moves faster
	player_vel = spellbook[1]["active"] == "true" ? 800.f : default_player_vel;

	// process player flash timer
	flash_timer -= elapsed_ms_since_last_update;
	if (flash_timer <= 0) {
		registry.remove<Flash>(player_minotaur);
	}

	return true;
}

// based on the explanation from https://en.wikipedia.org/wiki/Maze_generation_algorithm (yes, really)
void WorldSystem::recursiveGenerateMaze(std::vector<std::vector<MapTile>> &maze, int begin_x, int begin_y, int end_x, int end_y) {
	int size_x = end_x - begin_x;
	int size_y = end_y - begin_y;

	if (size_x < 3 && size_y < 3) return;

	int split_x = begin_x + 1;
	int split_y = begin_y + 1;

	if (size_y > size_x) {
		// split horizontally
    	std::uniform_int_distribution<int> rand_spot(begin_x, end_x - 1);
		const int passage = rand_spot(rng);
		for (int i = begin_x; i < end_x; i++) {
			if (passage == i) {
				maze[split_y][i] = MapTile::FREE_SPACE;
			} else {
				maze[split_y][i] = MapTile::BREAKABLE_WALL;
			}
		}

		recursiveGenerateMaze(maze, begin_x, begin_y, end_x, split_y);
		recursiveGenerateMaze(maze, begin_x, split_y + 1, end_x, end_y);
	} else {
		// split vertically
		std::uniform_int_distribution<int> rand_spot(begin_y, end_y - 1);
		const int passage = rand_spot(rng);
		for (int i = begin_y; i < end_y; i++) {
			if (passage == i) {
				maze[i][split_x] = MapTile::FREE_SPACE;
			} else {
				maze[i][split_x] = MapTile::BREAKABLE_WALL;
			}
		}

		recursiveGenerateMaze(maze, begin_x, begin_y, split_x, end_y);
		recursiveGenerateMaze(maze, split_x + 1, begin_y, end_x, end_y);
	}
}

std::vector<std::vector<MapTile>> WorldSystem::generateProceduralMaze(std::string method, int width, int height, vec2 &start_tile) {
	fprintf(stderr, "Generating %s procedural maze %d x %d\n", method.c_str(), width, height);

	// create vector
	std::vector<std::vector<MapTile>> maze;
	maze.resize(height, std::vector<MapTile>(width));
	if (method != "recursive") { // recursive is additive
		for (int i = 0; i < height; i++) {
			std::fill(maze[i].begin(), maze[i].end(), MapTile::BREAKABLE_WALL);
		}
	}

	// walls
	std::fill(maze[0].begin(), maze[0].end(), MapTile::UNBREAKABLE_WALL);
	std::fill(maze[height-1].begin(), maze[height-1].end(), MapTile::UNBREAKABLE_WALL);
	for (int i = 0; i < height; i++) {
		maze[i][0] = maze[i][width-1] = MapTile::UNBREAKABLE_WALL;
	}

	if (method == "recursive") {
		recursiveGenerateMaze(maze, 1, 1, width - 1, height - 1);
	} else if (method == "binarytree") {
		// carve passages
		// based on https://hurna.io/academy/algorithms/maze_generator/binary.html, modified for efficiency and format
		for (int i = 1; i < height; i += 2) {
			for (int j = 1; j < width; j += 2) {
				bool up = i > 1 && maze[i - 2][j] == MapTile::FREE_SPACE;
				bool left = j > 1 && maze[i][j - 2] == MapTile::FREE_SPACE;

				maze[i][j] = MapTile::FREE_SPACE;
				if (up && left) {
					// there can only be one
					if (uniform_dist(rng) < 0.5) {
						up = false;
					} else {
						left = false;
					}
				}

				if (up) {
					maze[i - 1][j] = MapTile::FREE_SPACE;
				} else if (left) {
					maze[i][j - 1] = MapTile::FREE_SPACE;
				}
			}
		}
	}

	// random start position
	std::vector<int> possible_start_positions;
	for (int i = 0; i < height; i++) {
		if (tile_is_walkable(maze[i][1])) possible_start_positions.push_back(i);
	}

	int ind = std::uniform_int_distribution<int>(0, possible_start_positions.size() - 1)(rng);
	const int start_position = possible_start_positions[ind];
	maze[start_position][0] = MapTile::ENTRANCE;
	start_tile = vec2(0.0, (float) start_position);

	// random end position
	std::vector<int> possible_end_positions;
	for (int i = 0; i < height; i++) {
		if (tile_is_walkable(maze[i][width - 2])) possible_end_positions.push_back(i);
	}

	// start at 3, because 0, 1 and 2 are too easy for some algorithms
	ind = std::uniform_int_distribution<int>(3, possible_end_positions.size() - 1)(rng);
	const int end_position = possible_end_positions[ind];
	maze[end_position][width - 1] = MapTile::EXIT;

	return maze;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// delete old map, if one exists
	game_state.level.map_tiles.clear();

	// TODO set this up in a menu
	game_state.level_id = "testing1";

	YAML::Node level_config = YAML::LoadFile(levels_path(game_state.level_id + "/level.yaml"));
	const std::string level_name = level_config["name"].as<std::string>();
	const std::string level_type = level_config["type"].as<std::string>();

	fprintf(stderr, "Started loading level: %s - %s (%s)\n", game_state.level_id.c_str(), level_name.c_str(), level_type.c_str());
	if (level_type == "premade") {
		// load map
		fprintf(stderr, "Loading premade map\n");
		std::ifstream file(levels_path(game_state.level_id + "/map.txt"));
		if (file.is_open()) {
			std::string line;
			while (std::getline(file, line)) { // read one line from file
				std::istringstream str_stream(line);

				int tile;
				std::vector<MapTile> row;
				while (str_stream >> tile) { // read all tiles from this line
					row.push_back((MapTile) tile);

					if (tile == MapTile::ENTRANCE) {
						// current is entrance
						game_state.level.start_position = vec2(row.size() - 1, game_state.level.map_tiles.size());
					}
				}

				// push this map row to the final vector
				game_state.level.map_tiles.push_back(row);
			}
		} else assert(false);
		file.close();

		fprintf(stderr, "Loaded premade map\n");
	} else  if (level_type == "procedural") {
		fprintf(stderr, "Loading procedural map\n");

		const auto procedural_options = level_config["procedural_options"];
		const std::string method = procedural_options["method"].as<std::string>();
		const std::vector<int> size = procedural_options["size"].as<std::vector<int>>();

		assert(size[0] >= 5 && size[1] >= 5); // needs to be larger than 5
		assert(size[0] % 2 != 0 && size[1] % 2 != 0); // needs to be odd number
		game_state.level.map_tiles = generateProceduralMaze(method, size[0], size[1], game_state.level.start_position);

		fprintf(stderr, "Loaded procedural map\n");
	}

	for (const auto vec : game_state.level.map_tiles) {
		for (const auto v2 : vec) {
			printf("%d ", v2);
		}
		printf("\n");
	}

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all spikes, drones, ... but that would be more cumbersome
	registry.clear();

	const YAML::Node enemies = level_config["enemies"];
	if (enemies) {
		// find spawnable tiles
		std::vector<vec2> spawnable_tiles;
		auto maze = game_state.level.map_tiles;
		for (uint i = 0; i < maze.size(); i++) {
			auto row = maze[i];
			for (uint j = 0; j < row.size(); j++) {
				if (row[j] == MapTile::FREE_SPACE) {
					// inverted coordinates
					spawnable_tiles.push_back({j, i});
				}
			}
		}

		// subnodes of enemies
		for (YAML::const_iterator it = enemies.begin(); it != enemies.end(); it++) {
			std::string enemy_type = it->first.as<std::string>();
			int enemy_count = it->second.as<int>();

			while (enemy_count--) { // spawn enemy_count enemies
				int pos_ind = std::uniform_int_distribution<int>(0, spawnable_tiles.size() - 1)(rng);
				vec2 position = map_coords_to_position(spawnable_tiles[pos_ind]);
				position += vec2(map_scale / 2, map_scale / 2); // to spawn in the middle of the tile
				spawnable_tiles.erase(spawnable_tiles.begin() + pos_ind);

				entt::entity entity;
				if (enemy_type == "spikes") {
					entity = createSpike(renderer, position);
				} else if (enemy_type == "drones") {
					entity = createDrone(renderer, position);
				} else {
					assert(false); // unsupported enemy
					return;
				}

				// TODO this should be controlled by AI, not an initial velocity
				Motion& motion = registry.get<Motion>(entity);
				motion.mass = 200;
				motion.coeff_rest = 0.9f;
				motion.velocity = vec2(-100.f, 0.f);
			}
		}
	}

	// Create a new Minotaur
	player_minotaur = createMinotaur(
		renderer,
		WorldSystem::map_coords_to_position(game_state.level.start_position)
		+ vec2(map_scale / 2, map_scale / 2) // this is to make it spawn on the center of the tile
	);
	registry.emplace<Colour>(player_minotaur, vec3(1, 0.8f, 0.8f));

	// reset player flash timer
	flash_timer = 1000.f;
	registry.emplace<Flash>(player_minotaur);

	fprintf(stderr, "Loaded level: %s - %s (%s)\n", game_state.level_id.c_str(), level_name.c_str(), level_type.c_str());
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto collisions = registry.view<Collision>();
	for (entt::entity entity : collisions) {
		// The entity and its collider
		entt::entity entity_other = collisions.get<Collision>(entity).other;

		// For now, we are only interested in collisions that involve the salmon
		if (registry.view<Player>().contains(entity)) {

			// Checking Player - Enemy collisions
			if (registry.view<Enemy>().contains(entity_other)) {
				// initiate death unless already dying
				if (!registry.view<DeathTimer>().contains(entity) && spellbook[3]["active"] == "false") {
					// Scream, reset timer, and make the salmon sink
					Motion& m = registry.get<Motion>(entity);
					registry.emplace<DeathTimer>(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);
					Colour& c = registry.get<Colour>(entity);
					c.colour = vec3( 0.27, 0.27, 0.27 );


					// Reset player speed/movement to 0
					m.velocity.x = 0;
					m.velocity.y = 0;

					// Stop pathfinding movement
					do_pathfinding_movement = false;

					// Below is the acceleration/flag-based movement implementation
					//move_right = false;
					//move_left = false;
					//move_up = false;
					//move_down = false;
				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.clear<Collision>();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	static std::map<int, bool> pressed_keys = std::map<int, bool>();

	if (action == GLFW_PRESS) {
		pressed_keys.insert({ key, true });
	}
	else if (action == GLFW_RELEASE && pressed_keys.find(key) != pressed_keys.end()) {
		pressed_keys.erase(key);
	} // not GLFW_REPEAT

	entt::entity player = registry.view<Player>().begin()[0];
	Motion& motion = registry.get<Motion>(player);

	if (!registry.view<DeathTimer>().contains(player)) {

		if (key == GLFW_KEY_SPACE) {
			// minotaur attack mode on spack key
			if (action == GLFW_PRESS && !registry.view<Attack>().contains(player))
			{
				registry.emplace<Attack>(player);
			}

			if (action == GLFW_RELEASE) {
				registry.remove<Attack>(player);
			}
		}

		if (action != GLFW_REPEAT) {
			motion.velocity = { 0, 0 };

			if (pressed_keys.find(GLFW_KEY_UP) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_W) != pressed_keys.end()) {
				do_pathfinding_movement = false;
				motion.velocity.y = -1 * player_vel;
			}

			if (pressed_keys.find(GLFW_KEY_LEFT) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_A) != pressed_keys.end()) {
				do_pathfinding_movement = false;
				motion.velocity.x = -1 * player_vel;
			}

			if (pressed_keys.find(GLFW_KEY_RIGHT) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_D) != pressed_keys.end()) {
				do_pathfinding_movement = false;
				motion.velocity.x = player_vel;
			}

			if (pressed_keys.find(GLFW_KEY_DOWN) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_S) != pressed_keys.end()) {
				do_pathfinding_movement = false;
				motion.velocity.y = player_vel;
			}

		}
		// Resetting game
		if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			restart_game();
		}

	}
}

// Pathfinding: Variables for pathfinding feature used only in on_mouse_button
double x_pos_press, y_pos_press, x_pos_release, y_pos_release;

void WorldSystem::on_mouse_button(int button, int action, int mods) {

	// ========= Feature: Pathfinding =========
	// To separate mouse click from gestures, we need to make sure it's just a click, not a swipe
	// Latency-friendly implementation:
	//		A click means the distance between press and release is small. We choose 'small' instead 
	//      of 0 because sometimes the pressing phase of clicks are a little long for humans. 
	//		It's only made longer with lag.

	// Capture press position
	if (action == GLFW_PRESS   && button == GLFW_MOUSE_BUTTON_LEFT) { glfwGetCursorPos(window, &x_pos_press, &y_pos_press); }
	if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {

		// Capture release position
		glfwGetCursorPos(window, &x_pos_release, &y_pos_release);

		// If it's truly a click...
		float click_threshold = 100;
		if (abs(x_pos_release - x_pos_press) < click_threshold && abs(y_pos_release - y_pos_press) < click_threshold) {

			// Implementation: There's a difference between camera coords and world coords.
			// glfwGetCursorPos gets the position of the cursor in the window. So clicking
			// the center of the window returns, for example, 1200/2=600 and 800/2=400.
			// But the player's coords are different. Even though the player is in the
			// center of the window, it's **world** coords are actually (initially) (75,225).
			// So we need to get the coords relative to the player for pathfinding.

			entt::entity player = registry.view<Player>().begin()[0];
			Motion& player_motion = registry.get<Motion>(player);

			// Get cursor screen coords
			vec2 cursor_screen_pos = { float(x_pos_release - window_width_px/2), float(y_pos_release - window_height_px/2) };

			// Get cursor world coords
			vec2 target_world_pos = { player_motion.position.x + cursor_screen_pos.x, player_motion.position.y + cursor_screen_pos.y };

			// Get cursor map coords (returns something like (0,1)) representing column 0, row 1.
			vec2 target_map_pos = position_to_map_coords(target_world_pos);

			// If clicked a traversable node (i.e. not a wall)...
			if (get_map_tile(target_map_pos) == FREE_SPACE) {

				// Store starting and ending positions for ai position to look at
				vec2 player_map_pos = position_to_map_coords(player_motion.position);
				starting_map_pos    = player_map_pos;
				ending_map_pos      = target_map_pos;

				// Trigger a flag and let ai_system.cpp handle the rest
				do_generate_path    = true;
			}

			// Did not clcik a traversable node...
			else { std::cout << "Clicked on a wall!" << std::endl; }

		}

	}
	// ========================================
	// Gestures: The following blocks are for the gesture feature

	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			flag_right = true;
			t = Clock::now();
		}
		else if (action == GLFW_RELEASE) {
			flag_right = false;

			// Capture elapsed time
			auto now = Clock::now();
			float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;

			if (!gesture_coords_right.empty()) {
				// Modify datastructs
				mouse_spell.update_datastructs(gesture_statuses, gesture_queue, gesture_coords_right, "RMB", flag_fast, elapsed_ms);
				// Check Spell Cast
				mouse_spell.check_spell(gesture_queue, spellbook, flag_fast);
				// reset fast_flag;
				flag_fast = false;
			}
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			flag_left = true;
			t = Clock::now();
		}
		else if (action == GLFW_RELEASE) {
			flag_left = false;
			// Capture elapsed time
			auto now = Clock::now();
			float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;

			if (!gesture_coords_left.empty()) {
				// Modify datastructs
				mouse_spell.update_datastructs(gesture_statuses, gesture_queue, gesture_coords_left, "LMB", flag_fast, elapsed_ms);
				// Check Spell Cast
				mouse_spell.check_spell(gesture_queue, spellbook, flag_fast);
				// reset fast_flag;
				flag_fast = false;
			}
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// Capture mouse movement coords into vector if LMB or RMB is pressed
	if (flag_right) { gesture_coords_right.push_back({ mouse_position.x,mouse_position.y }); }
	if (flag_left) { gesture_coords_left.push_back({ mouse_position.x,mouse_position.y }); }
}

vec2 WorldSystem::map_coords_to_position(vec2 map_coords) {
	return {map_scale * map_coords.x, map_scale * map_coords.y};
}

float WorldSystem::map_coords_to_position(float map_coords) {
	return map_scale * map_coords;
}

vec2 WorldSystem::position_to_map_coords(vec2 position) {
	return {floor(position.x / map_scale), floor(position.y / map_scale)};
}

int WorldSystem::position_to_map_coords(float position) {
	return (int) floor(position / map_scale);
}

bool WorldSystem::is_within_bounds(vec2 map_coords) {
	return map_coords.y >= 0
		&& map_coords.y < game_state.level.map_tiles.size()
		&& map_coords.x >= 0
		&& map_coords.x < game_state.level.map_tiles[(int)(map_coords.y)].size();
}

bool WorldSystem::tile_is_walkable(MapTile tile) {
	return tile != MapTile::UNBREAKABLE_WALL && tile != MapTile::BREAKABLE_WALL;
}

MapTile WorldSystem::get_map_tile(vec2 map_coords) {
	if (WorldSystem::is_within_bounds(map_coords)) return game_state.level.map_tiles[(int)(map_coords.y)][(int)(map_coords.x)];

	return MapTile::FREE_SPACE; // out of bounds
}
