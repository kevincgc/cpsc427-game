// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "ai_system.hpp"
#include <GLFW/glfw3.h>


// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>


// myLibs
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <iterator>
#include <string>
#include <chrono>
using Clock = std::chrono::high_resolution_clock;

// Game configuration
int MAX_DRONES;
int MAX_SPIKES;
const size_t TURTLE_DELAY_MS = 2000 * 3;
const size_t FISH_DELAY_MS = 5000 * 3;
const size_t ITEM_DELAY_MS = 3000 * 3;
vec2 WorldSystem::camera = { 0, 0 };
vec2 player_vel = { 300.f, 300.f };
vec2 enemy_vel = { 100.f, 100.f };
vec2 default_player_vel = { 300.f, 300.f };

// My Settings
auto t = Clock::now();

bool flag_right = false;
bool flag_left = false;
bool flag_fast = false;
bool active_spell = false;
float spell_timer = 6000.f;
std::vector<vec2> spawnable_tiles; // moved out for respawn functionality

// Item-related
std::vector<Item> inventory;
Item current_item;
std::map<std::string, ItemType> item_to_enum = {
	{"wall breaker", ItemType::WALL_BREAKER},
	{"extra life", ItemType::EXTRA_LIFE},
	{"teleporter", ItemType::TELEPORT},
	{"speed boost", ItemType::SPEED_BOOST},
};
bool wall_breaker_active = false;
ItemType most_recent_used_item = ItemType::NONE;

// For pathfinding feature
bool do_generate_path = false;
vec2 path_target_map_pos;
vec2 starting_map_pos;
vec2 ending_map_pos;

// For attack
bool player_swing = false;

// For enemy moving when player moves
bool player_is_manually_moving = false;

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
	if (tada_sound != nullptr)
		Mix_FreeChunk(tada_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
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

	// obtain user's screen resolution
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	float user_aspect_ratio = (float)(mode->width) / (mode->height);

	// game initally designed with a 1200 x 800 window with 1920 x 1080 resolution, so scale the window size to maintain this ratio with the user's screen
	// keeps the aspect ratio of the game intact across different machines
	window_width_px = mode->width * (1200.f / 1920.f);
	window_height_px = mode->height * (800.f / 1080.f);
	float width_scaling = window_width_px / 1200.f;
	float height_scaling = window_height_px / 800.f;
	// Will be used to scale all rendered components and movement attributes in the game
	global_scaling_vector = { width_scaling, height_scaling };

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Maze Runners", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glViewport(0, 0, window_width_px, window_height_px);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to setup GLEW\n");
		exit(1);
	}
	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button(_0, _1, _2); };
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
	tada_sound = Mix_LoadWAV(audio_path("tada.wav").c_str());

	

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
	// Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// scale global variables according to user's screen resolution (map, meshes, motion, etc)
	map_scale = 150.f * global_scaling_vector;
	player_vel *= global_scaling_vector;
	default_player_vel *= global_scaling_vector;
	enemy_vel *= global_scaling_vector;

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
	// TODO: add timer
	title_ss << "Elapsed time: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// setting coordinates of camera
	camera.x = registry.get<Motion>(player_minotaur).position.x - screen_width / 2;
	camera.y = registry.get<Motion>(player_minotaur).position.y - screen_height / 2;

	// Removing out of screen entities
	auto motions = registry.view<Motion>();

	for (auto entity : motions) {
		Motion& motion = motions.get<Motion>(entity);
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			registry.destroy(entity);
		}
	}

	float min_counter_ms = 3000.f;
	for (entt::entity entity : registry.view<DeathTimer>()) {
		// progress timer
		DeathTimer& counter = registry.get<DeathTimer>(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.remove<DeathTimer>(entity);
			state = ProgramState::GAME_OVER;
			return true;
		}
	}

	// Temporary implementation: General timer for spell duration. Later implementation will have spell-specific timers
	// Deactivate spells based on time
	for (auto& spell : spellbook) {
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

	if (registry.view<SpeedBoostTimer>().contains(player_minotaur)) {
		player_vel = default_player_vel * 2.f;
	}

	// Temporary implementation: Handle speed-up spell: Player moves faster
	player_vel = spellbook[1]["active"] == "true" ? default_player_vel * 2.5f : default_player_vel;

	// progress timers
	flash_timer -= elapsed_ms_since_last_update;
	if (flash_timer <= 0) {
		registry.remove<Flash>(player_minotaur);
	}

	// TODO: abstract out helper given a component to advance timers
	if (!registry.view<TextTimer>().empty()) {
		TextTimer &counter = registry.get<TextTimer>(player_minotaur);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			registry.remove<TextTimer>(player_minotaur);
			most_recent_used_item = ItemType::NONE;
		}
	}

	if (!registry.view<SpeedBoostTimer>().empty()) {
		SpeedBoostTimer& counter = registry.get<SpeedBoostTimer>(player_minotaur);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			registry.remove<SpeedBoostTimer>(player_minotaur);
		}
	}

	if (!registry.view<WallBreakerTimer>().empty()) {
		WallBreakerTimer& counter = registry.get<WallBreakerTimer>(player_minotaur);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			registry.remove<WallBreakerTimer>(player_minotaur);
		}
	}

	if (!registry.view<AnimationTimer>().empty()) {
		AnimationTimer& counter = registry.get<AnimationTimer>(player_minotaur);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			registry.remove<AnimationTimer>(player_minotaur);
			tips.used_item = 0;
		}
	}

	// Temporary for crossplay playability: Handle enemy respawn
	// Problems: spawns in walls, spawns on player
	//if (registry.size<Enemy>() < MAX_DRONES + MAX_SPIKES) {

	//	entt::entity entity;
	//	int pos_ind = std::uniform_int_distribution<int>(0, spawnable_tiles.size() - 1)(rng);
	//	vec2 position = map_coords_to_position(spawnable_tiles[pos_ind]);
	//	position += vec2(map_scale / 2, map_scale / 2); // to spawn in the middle of the tile
	//	spawnable_tiles.erase(spawnable_tiles.begin() + pos_ind);

	//	entity = createSpike(renderer, position);

	//	// TODO this should be controlled by AI, not an initial velocity
	//	Motion& motion = registry.get<Motion>(entity);
	//	motion.mass = 200;
	//	motion.coeff_rest = 0.9f;
	//	motion.velocity = vec2(-100.f, 0.f);
	//}

	Motion& player_motion = registry.get<Motion>(player_minotaur);
	MapTile tile = get_map_tile(position_to_map_coords(player_motion.position));
	// check if player has won
	if (tile == MapTile::EXIT) {
		// player has found the exit!
		Mix_PlayChannel(-1, tada_sound, 0);
		restart_game();
		do_pathfinding_movement = false;
	}

	return true;
}

// based on the explanation from https://en.wikipedia.org/wiki/Maze_generation_algorithm (yes, really)
void WorldSystem::recursiveGenerateMaze(std::vector<std::vector<MapTile>>& maze, int begin_x, int begin_y, int end_x, int end_y) {
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
			}
			else {
				maze[split_y][i] = MapTile::BREAKABLE_WALL;
			}
		}

		recursiveGenerateMaze(maze, begin_x, begin_y, end_x, split_y);
		recursiveGenerateMaze(maze, begin_x, split_y + 1, end_x, end_y);
	}
	else {
		// split vertically
		std::uniform_int_distribution<int> rand_spot(begin_y, end_y - 1);
		const int passage = rand_spot(rng);
		for (int i = begin_y; i < end_y; i++) {
			if (passage == i) {
				maze[i][split_x] = MapTile::FREE_SPACE;
			}
			else {
				maze[i][split_x] = MapTile::BREAKABLE_WALL;
			}
		}

		recursiveGenerateMaze(maze, begin_x, begin_y, split_x, end_y);
		recursiveGenerateMaze(maze, split_x + 1, begin_y, end_x, end_y);
	}

}

std::vector<std::vector<MapTile>> WorldSystem::generateProceduralMaze(std::string method, int width, int height, vec2& start_tile) {
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
	std::fill(maze[height - 1].begin(), maze[height - 1].end(), MapTile::UNBREAKABLE_WALL);
	for (int i = 0; i < height; i++) {
		maze[i][0] = maze[i][width - 1] = MapTile::UNBREAKABLE_WALL;
	}

	if (method == "recursive") {
		recursiveGenerateMaze(maze, 1, 1, width - 1, height - 1);
	}
	else if (method == "binarytree") {
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
					}
					else {
						left = false;
					}
				}

				if (up) {
					maze[i - 1][j] = MapTile::FREE_SPACE;
				}
				else if (left) {
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
	start_tile = vec2(0.0, (float)start_position);

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

void WorldSystem::process_entity_node(YAML::Node node, std::function<void(std::string, vec2)> spawn_callback) {
	// all subnodes
	for (YAML::const_iterator it = node.begin(); it != node.end(); it++) {
		std::string entity_type = it->first.as<std::string>();
		int entity_count = 0;

		if (it->second.IsScalar()) {
			// spawn exactly this number of entities
			entity_count = it->second.as<int>();
		}
		else if (it->second.IsSequence()) {
			// spawn random number of entities
			std::vector<int> ct = it->second.as<std::vector<int>>();
			entity_count = std::uniform_int_distribution<int>(ct[0], ct[1])(rng);
		}
		else assert(false);

		while (entity_count--) { // callback for entity_count entities
			int pos_ind = std::uniform_int_distribution<int>(0, spawnable_tiles.size() - 1)(rng);
			vec2 position = map_coords_to_position(spawnable_tiles[pos_ind]);
			position += vec2(map_scale.x / 2, map_scale.y / 2); // to spawn in the middle of the tile
			spawnable_tiles.erase(spawnable_tiles.begin() + pos_ind);

			spawn_callback(entity_type, position);
		}
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// delete old map, if one exists
	game_state.level.map_tiles.clear();

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
					row.push_back((MapTile)tile);

					if (tile == MapTile::ENTRANCE) {
						// current is entrance
						game_state.level.start_position = vec2(row.size() - 1, game_state.level.map_tiles.size());
					}
				}

				// push this map row to the final vector
				game_state.level.map_tiles.push_back(row);
			}
		}
		else assert(false);
		file.close();

		fprintf(stderr, "Loaded premade map\n");
	}
	else if (level_type == "procedural") {
		fprintf(stderr, "Loading procedural map\n");

		const auto procedural_options = level_config["procedural_options"];
		const std::string method = procedural_options["method"].as<std::string>();
		const std::vector<int> size = procedural_options["size"].as<std::vector<int>>();

		assert(size[0] >= 5 && size[1] >= 5); // needs to be larger than 5
		assert(size[0] % 2 != 0 && size[1] % 2 != 0); // needs to be odd number
		game_state.level.map_tiles = generateProceduralMaze(method, size[0], size[1], game_state.level.start_position);

		fprintf(stderr, "Loaded procedural map\n");
	}
	else assert(false); // unknown level type

 // Remove all entities that we created
 // All that have a motion, we could also iterate over all spikes, drones, ... but that would be more cumbersome
	registry.clear();

	// find spawnable tiles for both items and enemies
	spawnable_tiles.clear();
	auto maze = game_state.level.map_tiles;
	for (uint i = 0; i < maze.size(); i++) {
		auto row = maze[i];
		for (uint j = 0; j < row.size(); j++) {
			if (row[j] == MapTile::FREE_SPACE) {
				// inverted coordinates
				spawnable_tiles.push_back({ j, i });
			}
		}
	}

	// create enemies for this level
	const YAML::Node enemies = level_config["enemies"];
	if (enemies) {
		process_entity_node(enemies, [this](std::string enemy_type, vec2 position) {
			if (enemy_type == "spikes") {
				createSpike(renderer, position);
			}
			else if (enemy_type == "drones") {
				createDrone(renderer, position);
			}
			else {
				assert(false); // unsupported enemy
				return;
			}
			});
	}

	// create items for this level
	const YAML::Node items = level_config["items"];
	if (items) {
		process_entity_node(items, [this](std::string item_type, vec2 position) {
			if (item_to_enum[item_type]) {
				createItem(renderer, position, item_type);
			}
			else {
				assert(false); // unsupported item
				return;
			}
			});
	}

	// Create a new Minotaur
	vec2 minotaur_position = WorldSystem::map_coords_to_position(game_state.level.start_position);
	minotaur_position += vec2(map_scale.x / 2, map_scale.y / 2); // this is to make it spawn on the center of the tile
	player_minotaur = createMinotaur(renderer, minotaur_position);
	registry.emplace<Colour>(player_minotaur, vec3(1, 0.8f, 0.8f));
	current_item = Item();
	tips = Help();

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

		// For now, we are only interested in collisions that involve the minotaur
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
					c.colour = vec3(0.27, 0.27, 0.27);

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
	return bool(glfwWindowShouldClose(window) || state == ProgramState::EXIT);
}

// Item functions
void WorldSystem::use_wall_breaker(Item& item){
	entt::entity player = registry.view<Player>().begin()[0];
	std::cout << "Used wall breaker item! The player now has 20 seconds to click a breakable wall to break it!" << std::endl;
	registry.emplace_or_replace<WallBreakerTimer>(player);
}

void WorldSystem::add_extra_life(Item& item){
	// TODO: pending addition of life system
	entt::entity player = registry.view<Player>().begin()[0];
	std::cout << "Used extra life item!" << std::endl;

}

void WorldSystem::use_teleport(Item& item){
	entt::entity player = registry.view<Player>().begin()[0];
	Motion& player_motion = registry.get<Motion>(player);

	std::vector<vec2> teleportable_tiles;
	auto maze = game_state.level.map_tiles;
	for (uint i = 0; i < maze.size(); i++) {
		auto row = maze[i];
		for (uint j = 0; j < row.size(); j++) {
			if (row[j] == MapTile::FREE_SPACE) {
				// inverted coordinates
				teleportable_tiles.push_back({ j, i });
			}
		}
	}

	int pos_ind = std::uniform_int_distribution<int>(0, teleportable_tiles.size() - 1)(rng);
	vec2 position = map_coords_to_position(teleportable_tiles[pos_ind]);
	position += vec2(map_scale.x / 2, map_scale.y / 2);
	std::cout << "Used teleport item to teleport to a random location!" << std::endl;
	player_motion.position = position;
}

void WorldSystem::use_speed_boost(Item& item){
	entt::entity player = registry.view<Player>().begin()[0];
	std::cout << "Used speed boost item!" << std::endl;
	registry.emplace_or_replace<SpeedBoostTimer>(player);
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	static std::map<int, bool> pressed_keys = std::map<int, bool>();

	// Menu Interaction
	if (action == GLFW_PRESS && state == ProgramState::RUNNING) {
		pressed_keys.insert({ key, true });
	}
	else if (action == GLFW_RELEASE && pressed_keys.find(key) != pressed_keys.end() && state == ProgramState::RUNNING) {
		pressed_keys.erase(key);
	} // not GLFW_REPEAT

	if (state == ProgramState::RUNNING) {
		entt::entity player = registry.view<Player>().begin()[0];
		Motion& motion = registry.get<Motion>(player);

		if (!registry.view<DeathTimer>().contains(player)) {

			// Swing Attack
			if (key == GLFW_KEY_SPACE) {
				if (action == GLFW_PRESS && !registry.view<Attack>().contains(player))
				{
					registry.emplace<Attack>(player);
					player_swing = true;
				}

				if (action == GLFW_RELEASE) {
					registry.remove<Attack>(player);
					player_swing = false;
				}
			}

			// Movement
			if (action != GLFW_REPEAT) {
				motion.velocity = { 0, 0 };

				if (pressed_keys.find(GLFW_KEY_UP) != pressed_keys.end()    || pressed_keys.find(GLFW_KEY_W) != pressed_keys.end()) {
					do_pathfinding_movement = false;
					player_is_manually_moving = true;
					motion.velocity.y = -1 * player_vel.y;
				}
        
				if (pressed_keys.find(GLFW_KEY_LEFT) != pressed_keys.end()  || pressed_keys.find(GLFW_KEY_A) != pressed_keys.end()) {
					do_pathfinding_movement = false;
					player_is_manually_moving = true;
					motion.velocity.x = -1 * player_vel.x;
				}

				if (pressed_keys.find(GLFW_KEY_RIGHT) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_D) != pressed_keys.end()) {
					do_pathfinding_movement = false;
					player_is_manually_moving = true;
					motion.velocity.x = player_vel.x;
				}

				if (pressed_keys.find(GLFW_KEY_DOWN) != pressed_keys.end()  || pressed_keys.find(GLFW_KEY_S) != pressed_keys.end()) {
					do_pathfinding_movement = false;
					player_is_manually_moving = true;
					motion.velocity.y = player_vel.y;
				}

				if (pressed_keys.size() == 0) { player_is_manually_moving = false; }
			}

			//if (action == GLFW_RELEASE && 
			//	(key == GLFW_KEY_UP    || key == GLFW_KEY_W ||
			//	 key == GLFW_KEY_LEFT  || key == GLFW_KEY_A || 
			//	 key == GLFW_KEY_RIGHT || key == GLFW_KEY_D ||
			//	 key == GLFW_KEY_DOWN  || key == GLFW_KEY_S)) {
			//	player_is_manually_moving = false;
			//}

			// Use Item
			if (key == GLFW_KEY_I) {
				if (!current_item.name.empty() && action == GLFW_PRESS) {
					most_recent_used_item = item_to_enum[current_item.name];
					switch (most_recent_used_item) {
					case ItemType::WALL_BREAKER:
						use_wall_breaker(current_item);
						break;
					case ItemType::EXTRA_LIFE:
						add_extra_life(current_item);
						break;
					case ItemType::TELEPORT:
						use_teleport(current_item);
						break;
					case ItemType::SPEED_BOOST:
						use_speed_boost(current_item);
						break;
					default:
						// unsupported item or NONE
						assert(false);
						break;
					}
					tips.basic_help = 0;
					tips.picked_up_item = 0;
					tips.item_info = 0;
					tips.used_item = 1;
					registry.emplace_or_replace<TextTimer>(player);
					registry.emplace_or_replace<AnimationTimer>(player);
					current_item = Item();
				}
			}

			// Tell user about the item they are holding (toggle with T if they are holding an item)
			if (!current_item.name.empty() && action == GLFW_PRESS && key == GLFW_KEY_T) {
				tips.basic_help = 0;
				tips.picked_up_item = 0;
				tips.item_info = !tips.item_info;
			} 

			// Pause Game
			if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
				state = ProgramState::PAUSED;
			}

			// Help Mode
			if (action == GLFW_PRESS) {
				// toggle H for basic help mode
				if (key == GLFW_KEY_H) {
					tips.basic_help = !tips.basic_help;
				}
			}
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
	if (state == ProgramState::RUNNING) {
		if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) { glfwGetCursorPos(window, &x_pos_press, &y_pos_press); }
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
				vec2 cursor_screen_pos = { float(x_pos_release - window_width_px / 2), float(y_pos_release - window_height_px / 2) };

				// Get cursor world coords
				vec2 target_world_pos = { player_motion.position.x + cursor_screen_pos.x, player_motion.position.y + cursor_screen_pos.y };

				// Get cursor map coords (returns something like (0,1)) representing column 0, row 1.
				vec2 target_map_pos = position_to_map_coords(target_world_pos);

				// If clicked a traversable node (i.e. not a wall)...
				if (tile_is_walkable(get_map_tile(target_map_pos))) {

					// Store starting and ending positions for ai position to look at
					vec2 player_map_pos = position_to_map_coords(player_motion.position);
					starting_map_pos = player_map_pos;
					ending_map_pos = target_map_pos;

					// Trigger a flag and let ai_system.cpp handle the rest
					do_generate_path = true;
				}

				// Clicked a wall
				else { 
					std::cout << "Clicked a wall!" << std::endl;
					if (registry.view<WallBreakerTimer>().contains(player) && get_map_tile(target_map_pos) == MapTile::BREAKABLE_WALL) {
						// do attack or stab animation, maybe turn red?
						game_state.level.map_tiles[(int)(target_map_pos.y)][(int)(target_map_pos.x)] = MapTile::FREE_SPACE;
						registry.erase<WallBreakerTimer>(player);
					}
				}

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
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// Capture mouse movement coords into vector if LMB or RMB is pressed
	if (flag_right) { gesture_coords_right.push_back({ mouse_position.x,mouse_position.y }); }
	if (flag_left) { gesture_coords_left.push_back({ mouse_position.x,mouse_position.y }); }
}

vec2 WorldSystem::map_coords_to_position(vec2 map_coords) {
	return { map_scale.x * map_coords.x, map_scale.y * map_coords.y };
}

vec2 WorldSystem::position_to_map_coords(vec2 position) {
	return { floor(position.x / map_scale.x), floor(position.y / map_scale.y) };
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
