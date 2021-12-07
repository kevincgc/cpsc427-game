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
#include <iomanip>


// myLibs
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <iterator>
#include <string>
#include <chrono>
#include <math.h>
#include <random>
using Clock = std::chrono::high_resolution_clock;

// Game configuration
int MAX_DRONES;
int MAX_SPIKES;
vec2 WorldSystem::camera = { 0, 0 };
vec2 player_vel = { 300.f, 300.f };
vec2 enemy_vel  = { 100.f, 100.f };
vec2 default_player_vel = { 300.f, 300.f };
int death_count = 0;
int player_health = 3;
auto t = Clock::now();
bool  flag_right   = false;
bool  flag_left    = false;
bool  flag_fast    = false;
bool  active_spell = false;
float spell_timer  = 6000.f;
std::vector<vec2> spawnable_tiles; // moved out for respawn functionality
int required_num_of_keys;

// Item-related
std::map<ItemType, int> inventory = {
	{ItemType::WALL_BREAKER, 0},
	{ItemType::KEY, 0},
	{ItemType::TELEPORT, 0},
	{ItemType::SPEED_BOOST, 0},
};
Item most_recent_collected_item;
std::map<std::string, ItemType> item_to_enum = {
	{"wall breaker", ItemType::WALL_BREAKER},
	{"key", ItemType::KEY},
	{"teleporter", ItemType::TELEPORT},
	{"speed boost", ItemType::SPEED_BOOST},
};
bool wall_breaker_active = false;
ItemType most_recent_used_item;

// ******** For pathfinding feature *******
bool do_generate_path = false;
vec2 path_target_map_pos;
vec2 starting_map_pos;
vec2 ending_map_pos;
// ********* For cutscene feature *********
enum cutscene_speaker {
	SPEAKER_MINOTAUR		 = 1,
	SPEAKER_DRONE			 = 2,
	SPEAKER_DRONE_SAD		 = 3,
	SPEAKER_DRONE_LAUGHING   = 4,
	SPEAKER_MINOTAUR_RTX_OFF = 5,
	SPEAKER_DRONE_RTX_OFF	 = 6
};
bool cutscene_reached_exit   = false;
bool cutscene_1_frame_0      = true;
bool cutscene_1_frame_1      = false;
bool cutscene_1_frame_2      = false;
bool do_cutscene_1			 = true;
bool in_a_cutscene			 = false;
bool rtx_on					 = true;
bool play_need_key_cutscene  = true;
int  num_times_exit_reached  = 0;
int  cutscene_selection      = 1; // 1 = game start (see menu.c for more info)
int  cutscene_speaker        = SPEAKER_MINOTAUR;
entt::entity cutscene_minotaur_entity;
entt::entity cutscene_drone_entity;
entt::entity cutscene_drone_sad_entity;
entt::entity cutscene_drone_laughing_entity;
entt::entity cutscene_minotaur_rtx_off_entity;
entt::entity cutscene_drone_rtx_off_entity;
// ********* For parallax feature *********
std::vector<entt::entity> background_entities;
entt::entity background_space2_entity;
entt::entity background_space3_entity;
// ********* For HUD feature **************
entt::entity hud_heart_1_entity;
entt::entity hud_heart_2_entity;
entt::entity hud_heart_3_entity;
entt::entity hud_bg_entity;
entt::entity hud_hammer_entity;
entt::entity hud_teleport_entity;
entt::entity hud_speedboost_entity;
entt::entity hud_key_entity;
entt::entity hud_no_hammer_entity;
entt::entity hud_no_teleport_entity;
entt::entity hud_no_speedboost_entity;
entt::entity hud_no_key_entity;
// ********* For Tutorial feature **************
std::vector<entt::entity> tutorial_enemy_entities;
entt::entity daedalus_entity;
std::map<std::string, bool> tutorial_flags = {
	{"initial_spawned",			 false },
	{"played_in_cell_cutscene",  false },
	{"noted_enemy_movement",	 false },
	{"noted_floor",				 false },
	{"interrupted",				 false },
	{"spawned_daedalus",		 false },
	{"daedalus_reached_player",  false },
	{"do_interrupt_cutscene",    false },
	{"finished_interruption",    false },
	{"phase_2",					 false },
	{"p2_spawned_enemies",		 false },
	{"removed_daedalus",		 false },
	{"did_attack_cutscene",		 false },
	{"noted_teleporter",		 false },
	{"noted_arrival",		 false }
}; 

int speed_counter = 0;
int wallbreaker_counter = 0;

// Player flags
bool player_swing			   = false;
bool player_can_lose_health    = true;
bool player_marked_for_death   = false;
bool player_is_manually_moving = false;

std::map<int, bool> pressed_keys = std::map<int, bool>();

// For gestures
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
	{3, {
			{"name", "invincibility"},
			{"speed", "none"},
			{"combo_1", "gesture_LMB_down"},
			{"combo_2", "gesture_RMB_left"},
			{"active", "false"}
		}
	}
};
Mouse_spell mouse_spell;

class WorldSystem;
WorldSystem* wrld_sys;
class KeyCallback {
public:
	void on_key(int key, int, int action, int mod) {
		wrld_sys->on_key(key, 0, action, mod);
	}
	void on_mouse_move(vec2 mouse_position) {
		wrld_sys->on_mouse_move(mouse_position);
	}
	void on_mouse_button(int button, int action, int mods) {
		wrld_sys->on_mouse_button(button, action, mods);
	}
};

//Debugging
vec2 debug_pos = { 0,0 };

std::string formatTime(float game_time_ms) {
	float seconds = game_time_ms / 1000.f;
	int minutes = floor(seconds) / 60;
	seconds = seconds - minutes * 60;
	char buf[10] = {0};
	// sprintf_s(buf, 10, "%02d:%06.3f", minutes, seconds);
	sprintf(buf, "%02d:%06.3f", minutes, seconds);
	return std::string(buf);
}

// Create the world
WorldSystem::WorldSystem()
	: points(0) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
	wrld_sys = this;
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);

	for (int i = 0; i < sound_effect_count; i++) {
		if (sound_effects[i] != nullptr) Mix_FreeChunk(sound_effects[i]);
	}
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
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((KeyCallback*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((KeyCallback*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((KeyCallback*)glfwGetWindowUserPointer(wnd))->on_mouse_button(_0, _1, _2); };
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
	std::array<std::string, sound_effect_count> sound_effect_paths = {
		"player_death.wav",
		"player_item.wav",
		"tada.wav",
		"horse_snort.wav",
		"drone_were_it_only_so_easy.wav",
		"drone_stupid_boy.wav",
		"item_break_wall.wav",
		"item_teleport.wav",
		"item_speed_boost.wav",
		"chick_die.wav",
	};

	for (int i = 0; i < sound_effect_count; i++) {
		sound_effects[i] = Mix_LoadWAV(audio_path(sound_effect_paths[i]).c_str());
		if (sound_effects[i] == nullptr) {
			fprintf(stderr, "Not found: ");
			fprintf(stderr, sound_effect_paths[i].c_str());
			assert(false);
		}
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	Mix_VolumeMusic(Mix_VolumeMusic(-1) / 20);
	fprintf(stderr, "Loaded music\n");

	// scale global variables according to user's screen resolution (map, meshes, motion, etc)
	map_scale = 150.f * global_scaling_vector;
	player_vel *= global_scaling_vector;
	default_player_vel *= global_scaling_vector;
	enemy_vel *= global_scaling_vector;
}

std::vector<std::string> WorldSystem::get_leaderboard() {
	return current_leaderboard;
}

std::string WorldSystem::get_level_info() {
	std::string str = "LEVEL: " + game_state.level_id;

	if (game_state.level.phase > 0) {
		str += " PHASE: " + std::to_string(game_state.level.phase);
	}

	return str;
}

std::string WorldSystem::get_player_time() {
	return "Your time: " + formatTime(current_finish_time);
}

// ************************************************************************************* step ***********************************************************
// Update our game world
void WorldSystem::play_sounds() {
	for (auto sound_request : game_state.sound_requests) {
		Mix_PlayChannel(-1, sound_effects[(int) sound_request.sound], 0);
	}
	game_state.sound_requests.clear();
}

bool WorldSystem::step(float elapsed_ms_since_last_update) {
	game_time_ms += elapsed_ms_since_last_update;

	// Get the screen dimensions
	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Elapsed time: " << formatTime(game_time_ms);
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	for (auto debug_ent : registry.view<DebugComponent>()) {
		registry.destroy(debug_ent);
	}

	// setting coordinates of camera
	camera.x = registry.get<Motion>(player_minotaur).position.x - screen_width / 2;
	camera.y = registry.get<Motion>(player_minotaur).position.y - screen_height / 2;

	if (do_death_and_endgame(elapsed_ms_since_last_update)) { return true; }

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

	// Change player speed
	if (registry.view<SpeedBoostTimer>().contains(player_minotaur)) { player_vel = default_player_vel * 2.f; }
	// Temporary implementation: Handle speed-up spell: Player moves faster
	player_vel = spellbook[1]["active"] == "true" ? default_player_vel * 2.5f : default_player_vel;
	if (game_state.level_id == "tutorial") {
		do_tutorial(elapsed_ms_since_last_update);
	}
	do_timers(elapsed_ms_since_last_update);
	do_exit(); // check if player has won
	do_cutscene();
	do_HUD();
	do_cleanup();


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
	for (int i = 3; i < height; i++) {
		if (tile_is_walkable(maze[i][width - 2])) possible_end_positions.push_back(i);
	}

	// start at 3, because 0, 1 and 2 are too easy for some algorithms
	ind = std::uniform_int_distribution<int>(0, possible_end_positions.size() - 1)(rng);
	const int end_position = possible_end_positions[ind];
	maze[end_position][width - 1] = MapTile::EXIT;

	return maze;
}

void WorldSystem::process_entity_node(YAML::Node node, std::function<void(std::string, vec2)> spawn_callback, float multiplier) {
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

		entity_count *= multiplier;

		while (entity_count--) { // callback for entity_count entities
			int pos_ind = std::uniform_int_distribution<int>(0, spawnable_tiles.size() - 1)(rng);
			vec2 position = map_coords_to_position(spawnable_tiles[pos_ind]);
			position += vec2(map_scale.x / 2, map_scale.y / 2); // to spawn in the middle of the tile
			spawnable_tiles.erase(spawnable_tiles.begin() + pos_ind);

			spawn_callback(entity_type, position);
		}
	}
}

void WorldSystem::save_game() {
	std::string path = data_path() + "/savegame.yaml";
	std::ifstream file(path);
	if (!file.good()) {
		std::ofstream outfile(path);
		outfile.close();
	}
	file.close();

	YAML::Node save;

	YAML::Node node_game_state;
	save["game_state"] = node_game_state;

	save["game_state"]["level_id"] = game_state.level_id;
	save["game_state"]["win_condition"] = game_state.win_condition;
	save["game_state"]["level_phase"] = game_state.level.phase;
	save["game_state"]["level_has_next"] = game_state.level.has_next;

	save["game_state"]["level_map"] = game_state.level.map_tiles;

	YAML::Node node_world;
	save["world"] = node_world;

	Motion& player_motion = registry.get<Motion>(player_minotaur);
	save["world"]["player_position"] = position_to_map_coords(player_motion.position);

	// TODO save/restore other entities with motion
	for (entt::entity entity : registry.view<Motion>()) {
		if (entity == player_minotaur) continue;

		Motion& motion = registry.get<Motion>(entity);

		YAML::Node node_entity;
		node_entity["motion"]["position"] = motion.position;
		node_entity["motion"]["angle"] = motion.angle;
		node_entity["motion"]["velocity"] = motion.velocity;
		node_entity["motion"]["scale"] = motion.scale;
		node_entity["motion"]["mass"] = motion.mass;
		node_entity["motion"]["coeff_rest"] = motion.coeff_rest;
		node_entity["motion"]["can_collide"] = motion.can_collide;

		RenderRequest& rr = registry.get<RenderRequest>(entity);
		node_entity["texture_type"] = rr.used_texture;
		node_entity["geometry_type"] = rr.used_geometry;

		if (registry.view<Item>().contains(entity)) {
			Item &item = registry.get<Item>(entity);
			node_entity["item_type"] = item.name;
		}

		save["world"]["entities"].push_back(node_entity);
	}

	save["world"]["death_count"] = death_count;
	save["world"]["num_times_exit_reached"] = num_times_exit_reached;
	save["world"]["rtx_on"] = rtx_on;

	save["world"]["inventory"] = inventory;
	save["world"]["most_recent_collected_item"]["name"] = most_recent_collected_item.name;
	save["world"]["most_recent_collected_item"]["duration_ms"] = most_recent_collected_item.duration_ms;

	save["world"]["game_time_ms"] = game_time_ms;
	save["world"]["player_health"] = player_health;

	save["world"]["required_num_of_keys"] = required_num_of_keys;

	std::ofstream outfile(path);
	YAML::Emitter out;
	out << save;
	outfile << out.c_str();
	outfile.close();
}

void WorldSystem::load_game() {
	std::string path = data_path() + "/savegame.yaml";
	std::ifstream file(path);
	if (!file.good()) {
		// no save file, start from scratch
		restart_game();
		return;
	}
	file.close();

	// Delete old map, if one exists
	game_state.level.map_tiles.clear();

	// Remove all entities that we created
	registry.clear();
	chick_ai.clear();

	YAML::Node save = YAML::LoadFile(path);

	game_state.level_id = save["game_state"]["level_id"].as<std::string>();
	game_state.prev_level = game_state.level_id;
	game_state.win_condition = save["game_state"]["win_condition"].as<bool>();
	game_state.level.phase = save["game_state"]["level_phase"].as<int>();
	game_state.level.has_next = save["game_state"]["level_has_next"].as<bool>();

	game_state.level.map_tiles = save["game_state"]["level_map"].as<std::vector<std::vector<MapTile>>>();

	// Create a new Minotaur
	vec2 minotaur_position = WorldSystem::map_coords_to_position(save["world"]["player_position"].as<vec2>());
	minotaur_position += vec2(map_scale.x / 2, map_scale.y / 2); // this is to make it spawn on the center of the tile
	player_minotaur = createMinotaur(renderer, minotaur_position);
	registry.emplace<Colour>(player_minotaur, vec3(1, 0.8f, 0.8f));

	YAML::Node entities = save["world"]["entities"];
	for (int i = 0; i < entities.size(); i++) {
		YAML::Node entity_node = entities[i];

		vec2 position = entity_node["motion"]["position"].as<vec2>();
		if (entity_node["item_type"]) { // type item
			createItem(renderer, position, entity_node["item_type"].as<std::string>());
		} else {
			TEXTURE_ASSET_ID texture_type = entity_node["texture_type"].as<TEXTURE_ASSET_ID>();

			if (texture_type == TEXTURE_ASSET_ID::CHICK) {
				createChick(renderer, position);
			} else if (texture_type == TEXTURE_ASSET_ID::SPIKE) {
				createSpike(renderer, position);
			} else if (texture_type == TEXTURE_ASSET_ID::DRONE) {
				createDrone(renderer, position);
			}
		}
	}

	death_count = save["world"]["death_count"].as<int>();
	num_times_exit_reached = save["world"]["num_times_exit_reached"].as<int>();
	rtx_on = save["world"]["rtx_on"].as<bool>();

	inventory = save["world"]["inventory"].as<std::map<ItemType, int>>();
	player_health = save["world"]["player_health"].as<int>();
	required_num_of_keys = save["world"]["required_num_of_keys"].as<int>();

	most_recent_collected_item.name = save["world"]["most_recent_collected_item"]["name"].as<std::string>();
	most_recent_collected_item.duration_ms = save["world"]["most_recent_collected_item"]["duration_ms"].as<float>();

	game_time_ms = save["world"]["game_time_ms"].as<double>();

	start_game();
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	static std::string prev_level = "";

	// Delete old map, if one exists
	game_state.level.map_tiles.clear();

	YAML::Node level_config = YAML::LoadFile(levels_path(game_state.level_id + "/level.yaml"));

	if (game_state.win_condition && game_state.prev_level == game_state.level_id && level_config["progression"]["next_level"]) {
		fprintf(stderr, "Progression: changing level\n");
		game_state.level_id = level_config["progression"]["next_level"].as<std::string>();
		level_config = YAML::LoadFile(levels_path(game_state.level_id + "/level.yaml"));
	}

	const std::string level_name = level_config["name"].as<std::string>();
	const std::string level_type = level_config["type"].as<std::string>();

	fprintf(stderr, "Started loading level: %s - %s (%s)\n", game_state.level_id.c_str(), level_name.c_str(), level_type.c_str());

	if (game_state.prev_level != game_state.level_id) {
		fprintf(stderr, "Level changed\n");
		game_state.prev_level = game_state.level_id;

		if (level_config["progression"]["phase_multiply"]) {
			fprintf(stderr, "Enabling phase progression\n");
			game_state.level.phase = 1;
		} else {
			game_state.level.phase = 0;
		}
	} else if (game_state.win_condition && game_state.level.phase > 0) {
		// if phase progression enabled
		game_state.level.phase++;
	}

	if (level_config["progression"]["next_level"]) {
		fprintf(stderr, "Level has next_level\n");
		game_state.level.has_next = true;
	} else {
		game_state.level.has_next = false;
	}

	if (game_state.level.phase > 0) {
		fprintf(stderr, "Loading phase %d\n", game_state.level.phase);
	}

	bool multiplier_enabled = game_state.level.phase > 0 && level_config["progression"] && level_config["progression"]["phase_multiply"];

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
		std::vector<int> size = procedural_options["size"].as<std::vector<int>>();

		if (multiplier_enabled) {
			if (level_config["progression"]["phase_multiply"]["procedural_size"]) {
				float multiplier = level_config["progression"]["phase_multiply"]["procedural_size"].as<float>() * (game_state.level.phase - 1);
				multiplier = max(multiplier, 1.f);
				size[0] *= multiplier;
				if (size[0] % 2 == 0) size[0] += 1; // ensure odd

				size[1] *= multiplier;
				if (size[1] % 2 == 0) size[1] += 1; // ensure odd
			}
		}

		assert(size[0] >= 5 && size[1] >= 5); // needs to be larger than 5
		assert(size[0] % 2 != 0 && size[1] % 2 != 0); // needs to be odd number
		game_state.level.map_tiles = generateProceduralMaze(method, size[0], size[1], game_state.level.start_position);

		fprintf(stderr, "Loaded procedural map\n");
	}
	else assert(false); // unknown level type
	// ***********************************

	// Remove all entities that we created
	registry.clear();
	chick_ai.clear();

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
		float multiplier = 1.f;
		if (multiplier_enabled && level_config["progression"]["phase_multiply"]["enemies"]) {
			multiplier = level_config["progression"]["phase_multiply"]["enemies"].as<float>() * (game_state.level.phase - 1);
			multiplier = max(multiplier, 1.f);
			fprintf(stderr, "Phase %d enemies multiplier %d\n", game_state.level.phase, multiplier);
		}

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
			}, multiplier);
	}

	// create prey for this level
	const YAML::Node prey = level_config["prey"];
	if (prey) {
		float multiplier = 1.f;
		if (multiplier_enabled && level_config["progression"]["phase_multiply"]["prey"]) {
			multiplier = level_config["progression"]["phase_multiply"]["prey"].as<float>() * (game_state.level.phase - 1);
			multiplier = max(multiplier, 1.f);
			fprintf(stderr, "Phase %d prey multiplier %d\n", game_state.level.phase, multiplier);
		}

		process_entity_node(prey, [this](std::string prey_type, vec2 position) {
			if (prey_type == "chick") {
				createChick(renderer, position);
			}
			else {
				assert(false);
				return;
			}
			}, multiplier);
	}

	// create items for this level
	const YAML::Node items = level_config["items"];
	if (items) {
		float multiplier = 1.f;
		if (multiplier_enabled && level_config["progression"]["phase_multiply"]["items"]) {
			multiplier = level_config["progression"]["phase_multiply"]["items"].as<float>() * (game_state.level.phase - 1);
			multiplier = max(multiplier, 1.f);
			fprintf(stderr, "Phase %d items multiplier %d\n", game_state.level.phase, multiplier);
		}

		process_entity_node(items, [this](std::string item_type, vec2 position) {
			if (item_to_enum[item_type]) {
				createItem(renderer, position, item_type);
			}
			else {
				assert(false); // unsupported item
				return;
			}
			}, multiplier);
	}

	// Create a new Minotaur
	vec2 minotaur_position = WorldSystem::map_coords_to_position(game_state.level.start_position);
	minotaur_position += vec2(map_scale.x / 2, map_scale.y / 2); // this is to make it spawn on the center of the tile
	player_minotaur = createMinotaur(renderer, minotaur_position);
	registry.emplace<Colour>(player_minotaur, vec3(1, 0.8f, 0.8f));

	// reset inventory
	for (auto& item : inventory) {
		item.second = 0;
	}
	most_recent_collected_item = Item();

	// set number of keys needed
	if (game_state.level.phase == 0) { required_num_of_keys = 1; }
	else if (game_state.level.phase < 3) { required_num_of_keys = game_state.level.phase; }
	else { required_num_of_keys = game_state.level.phase + 1; }

	fprintf(stderr, "Loaded level: %s - %s (%s)\n", game_state.level_id.c_str(), level_name.c_str(), level_type.c_str());

	game_time_ms = 0.f;

	start_game();
}

void WorldSystem::start_game() {
	tips = Help();

	// reset player flash timer
	flash_timer = 1000.f;
	registry.emplace<Flash>(player_minotaur);

	// Create cutscene entities
	cutscene_drone_entity			 = createCutscene(renderer, Cutscene_enum::DRONE		   );
	cutscene_drone_sad_entity		 = createCutscene(renderer, Cutscene_enum::DRONE_SAD	   );
	cutscene_drone_laughing_entity   = createCutscene(renderer, Cutscene_enum::DRONE_LAUGHING  );
	cutscene_minotaur_entity		 = createCutscene(renderer, Cutscene_enum::MINOTAUR		   );
	cutscene_minotaur_rtx_off_entity = createCutscene(renderer, Cutscene_enum::MINOTAUR_RTX_OFF);
	cutscene_drone_rtx_off_entity	 = createCutscene(renderer, Cutscene_enum::DRONE_RTX_OFF   );

	// ************* Order is important for correct layering ***************

	// Create HUD entities
	hud_heart_1_entity	     = createHUD(renderer, 1);
	hud_heart_2_entity	     = createHUD(renderer, 1);
	hud_heart_3_entity	     = createHUD(renderer, 1);
	hud_no_hammer_entity     = createHUD(renderer, 6);
	hud_no_teleport_entity   = createHUD(renderer, 7);
	hud_no_speedboost_entity = createHUD(renderer, 8);
	hud_no_key_entity		 = createHUD(renderer, 10);
	hud_hammer_entity	     = createHUD(renderer, 3);
	hud_teleport_entity	     = createHUD(renderer, 4);
	hud_speedboost_entity    = createHUD(renderer, 5);
	hud_key_entity			 = createHUD(renderer, 9);
	hud_bg_entity			 = createHUD(renderer, 2);

	// Reset background
	background_entities.clear();
	// Create background entites
	vec2 pos;
	int asset;
	for (int i = 0; i < 8; i++) {
		switch (i) {
		case 0: pos = { 500,500 }; asset = 2; break;
		case 1: pos = { 500,1000 }; asset = 2; break;
		case 2: pos = { 1500,500 }; asset = 2; break;
		case 3: pos = { 1500,1000 }; asset = 2; break;
		case 4: pos = { 3000,500 }; asset = 2; break;
		case 5: pos = { 3000,1000 }; asset = 2; break;
		case 6: pos = { 500, 500 }; asset = 4; break;
		case 7: pos = { 200, 500 }; asset = 5; break;
		}
		entt::entity ent = createBackground(renderer, pos, asset);
		background_entities.push_back(ent);
	}
	createBackground(renderer, { 1000,1000  }, 1);
	createBackground(renderer, { 1000,3000  }, 1);
	createBackground(renderer, { 3000,1000  }, 1);
	createBackground(renderer, { 3000,3000  }, 1);
	createBackground(renderer, { 5000,1000  }, 1);
	createBackground(renderer, { 5000,3000  }, 1);


	// ***********************************************************

	// Reset player life to 3
	player_health = 3;

	// To prevent enemies from moving before player moves
	do_pathfinding_movement   = false;
	player_is_manually_moving = false;
	pressed_keys.clear();

	game_state.win_condition = false;

	// Reset tutorial
	if (game_state.level_id == "tutorial") {
		// Set all flags to false
		for (auto it = tutorial_flags.begin(); it != tutorial_flags.end(); it++) {
			it->second = false;
		}
		// Clear enemy entites
		tutorial_enemy_entities.clear();
	}



}

void WorldSystem::onNotify(const entt::entity& entity, const entt::entity& other, Event event) {
	entt::entity player = registry.view<Player>().begin()[0];
	switch (event)
	{
	case Event::PLAYER_ENEMY_COLLISION:
		if (player_can_lose_health && !registry.view<DeathTimer>().contains(player))
		{
			game_state.sound_requests.push_back({ SoundEffects::PLAYER_DEAD }); // Scream
			registry.emplace<DeathTimer>(entity);							    // Start a death timer (an invulnerability cooldown)
			player_can_lose_health = false;										// Set invulnerability
			player_health--;													// Reduce player health by one

			if (player_health < 1) {

				player_marked_for_death = true;									// Mark player for death

				// Render colour
				Colour& c = registry.get<Colour>(entity);
				c.colour = vec3(0.27, 0.27, 0.27);

				// Increment death_count
				death_count++;
				std::cout << "Death count is: " << death_count << std::endl;

				// Set movement flags to false so enemies won't move upon reset
				do_pathfinding_movement = false;
				player_is_manually_moving = false;
			}
		}
		break;
	case Event::PLAYER_PREY_COLLISION:
		game_state.sound_requests.push_back({ SoundEffects::CHICK_DIE });
		std::cout << "Calories make you go brrrrr" << std::endl;
		registry.get<Motion>(player).velocity *= 2.5f;
		for (auto it = registry.view<Prey>().begin(); it != registry.view<Prey>().end(); it++) {
			Prey& p = registry.get<Prey>(*it);
		}
		const entt::entity& prey_entity = registry.view<Prey>().contains(entity) ? entity : other;
		Prey& prey = registry.view<Prey>().contains(entity) ? registry.get<Prey>(entity) : registry.get<Prey>(other);
		auto ai_it = chick_ai.begin();
		for (auto it = chick_ai.begin(); it != chick_ai.end(); it++) {
			if ((*it).get_id() == prey.id) {
				ai_it = it;
			}
		}
		(*ai_it).clear();
		chick_ai.erase(ai_it);
		std::cout << "Destroyed entity " << int(prey_entity) << std::endl;
		registry.destroy(prey_entity);
		break;
	}
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window) || state == ProgramState::EXIT);
}

// Item functions
void WorldSystem::use_wall_breaker(){
	entt::entity player = registry.view<Player>().begin()[0];
	std::cout << "Used wall breaker item! The player now has 20 seconds to click a breakable wall to break it!" << std::endl;
	registry.emplace_or_replace<WallBreakerTimer>(player);
	most_recent_used_item = ItemType::WALL_BREAKER;

}

//void WorldSystem::add_extra_life(){
//	// TODO: pending addition of life system
//	entt::entity player = registry.view<Player>().begin()[0];
//	std::cout << "Used extra life item!" << std::endl;
//	most_recent_used_item = ItemType::EXTRA_LIFE;
//
//}

void WorldSystem::use_teleport(){
	entt::entity player = registry.view<Player>().begin()[0];
	Motion& player_motion = registry.get<Motion>(player);
	vec2 position;
	if (game_state.level_id == "tutorial") {
		position = map_coords_to_position({ 30,4 });
		position += vec2(map_scale.x / 2, map_scale.y / 2);
	}
	else {
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

		std::random_device rd; // obtain a random number from hardware
		std::mt19937 gen(rd()); // seed the generator
		std::uniform_int_distribution<unsigned long int> distr(1, teleportable_tiles.size() - 1); // define the range

		position = map_coords_to_position(teleportable_tiles[distr(gen)]);
		position += vec2(map_scale.x / 2, map_scale.y / 2);
	}
	std::cout << "Used teleport item to teleport to a random location!" << std::endl;
	player_motion.position = position;
	most_recent_used_item = ItemType::TELEPORT;
	game_state.sound_requests.push_back({SoundEffects::ITEM_TELEPORT});
}

void WorldSystem::use_speed_boost(){
	entt::entity player = registry.view<Player>().begin()[0];
	std::cout << "Used speed boost item!" << std::endl;
	registry.emplace_or_replace<SpeedBoostTimer>(player);
	most_recent_used_item = ItemType::SPEED_BOOST;
	game_state.sound_requests.push_back({SoundEffects::ITEM_SPEED_BOOST});
}

void WorldSystem::postItemUse(entt::entity& player) {
	registry.emplace_or_replace<TextTimer>(player);
	registry.emplace_or_replace<AnimationTimer>(player);
	tips = Help();
	tips.used_item = 1;
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// Menu Interaction
	if (action == GLFW_PRESS && state == ProgramState::RUNNING) {
		pressed_keys.insert({ key, true });
	}
	else if (action == GLFW_RELEASE && pressed_keys.find(key) != pressed_keys.end() && state == ProgramState::RUNNING) {
		pressed_keys.erase(key);
	} // not GLFW_REPEAT

	if (state == ProgramState::RUNNING) {
	entt::entity player		   = registry.view<Player>().begin()[0];
	Player& p_player		   = registry.get<Player>(player);
	Motion& motion			   = registry.get<Motion>(player);
	Motion& hud_heart_1_motion = registry.get<Motion>(hud_heart_1_entity);

		if (!registry.view<DeathTimer>().contains(player) && !in_a_cutscene) {

			// Swing Attack
			if (key == GLFW_KEY_SPACE) {
				if (action == GLFW_PRESS && !registry.view<Attack>().contains(player)) { registry.emplace<Attack>(player); }
				if (action == GLFW_RELEASE)											   { registry.remove<Attack>(player);  }
			}

			// Movement
			if (action != GLFW_REPEAT) {
				// Parallax Settings
				for (entt::entity bg_entity : background_entities) {
					Motion& bg_motion = registry.get<Motion>(bg_entity);
					bg_motion.velocity = { 0,0 };
				}
				//bg_2_motion.velocity = { 0, 0 }; // This prevents constant movement by resetting to 0
				//bg_3_motion.velocity = { 0, 0 };
				float bg_2_vel = 40.f;
				float bg_3_vel = 100.f;
				float bg_moon_vel = 20.f;
				float bg_satellite_vel = 60.f;

				hud_heart_1_motion.velocity = { 0,0 };
				motion.velocity				= { 0, 0 };


				if (pressed_keys.find(GLFW_KEY_UP) != pressed_keys.end()    || pressed_keys.find(GLFW_KEY_W) != pressed_keys.end()) {
					do_pathfinding_movement       = false;
					player_is_manually_moving     = true;
					motion.velocity.y		      = -1 * player_vel.y;
					for (int bg_entity = 0; bg_entity < background_entities.size(); bg_entity++) {
						Motion& bg_motion = registry.get<Motion>(background_entities[bg_entity]);
						if (int(bg_entity) % 2  == 0 && int(bg_entity) < 6)  { bg_motion.velocity.y = bg_2_vel; }
						else if (int(bg_entity) == 6) { bg_motion.velocity.y = bg_moon_vel; }
						else if (int(bg_entity) == 7) { bg_motion.velocity.y = bg_satellite_vel; }
						else						  { bg_motion.velocity.y = bg_3_vel; }
					}
				}

				if (pressed_keys.find(GLFW_KEY_LEFT) != pressed_keys.end()  || pressed_keys.find(GLFW_KEY_A) != pressed_keys.end()) {
					do_pathfinding_movement       = false;
					player_is_manually_moving     = true;
					motion.velocity.x		      = -1 * player_vel.x;
					//hud_heart_1_motion.velocity.x = -1 * player_vel.x;
					for (int bg_entity = 0; bg_entity < background_entities.size(); bg_entity++) {
						Motion& bg_motion = registry.get<Motion>(background_entities[bg_entity]);
						if (int(bg_entity) % 2 == 0 && int(bg_entity) < 6) { bg_motion.velocity.x = bg_2_vel; }
						else if (int(bg_entity) == 6) { bg_motion.velocity.x = bg_moon_vel; }
						else if (int(bg_entity) == 7) { bg_motion.velocity.x = bg_satellite_vel; }
						else { bg_motion.velocity.x = bg_3_vel; }
					}
				}

				if (pressed_keys.find(GLFW_KEY_RIGHT) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_D) != pressed_keys.end()) {
					do_pathfinding_movement = false;
					player_is_manually_moving = true;
					motion.velocity.x = player_vel.x;
					for (int bg_entity = 0; bg_entity < background_entities.size(); bg_entity++) {
						Motion& bg_motion = registry.get<Motion>(background_entities[bg_entity]);
						if (int(bg_entity) % 2 == 0 && int(bg_entity) < 6) { bg_motion.velocity.x = -1 * bg_2_vel; }
						else if (int(bg_entity) == 6) { bg_motion.velocity.x = -1 * bg_moon_vel; }
						else if (int(bg_entity) == 7) { bg_motion.velocity.x = -1 * bg_satellite_vel; }
						else { bg_motion.velocity.x = -1 * bg_3_vel; }
					}
				}

				if (pressed_keys.find(GLFW_KEY_DOWN) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_S) != pressed_keys.end()) {
					do_pathfinding_movement = false;
					player_is_manually_moving = true;
					motion.velocity.y = player_vel.y;
					for (int bg_entity = 0; bg_entity < background_entities.size(); bg_entity++) {
						Motion& bg_motion = registry.get<Motion>(background_entities[bg_entity]);
						if (int(bg_entity) % 2 == 0 && int(bg_entity) < 6) { bg_motion.velocity.y = -1 * bg_2_vel; }
						else if (int(bg_entity) == 6) { bg_motion.velocity.y = -1 * bg_moon_vel; }
						else if (int(bg_entity) == 7) { bg_motion.velocity.y = -1 * bg_satellite_vel; }
						else { bg_motion.velocity.y = -1 * bg_3_vel; }
					}
				}

				if (pressed_keys.size() == 0) { player_is_manually_moving = false; }
			}

			// Use Items
			if (action == GLFW_PRESS && key == GLFW_KEY_1 && inventory[ItemType::WALL_BREAKER] > 0) {
				use_wall_breaker();
				inventory[ItemType::WALL_BREAKER]--;
				 postItemUse(player);
			}

			if (action == GLFW_PRESS && key == GLFW_KEY_2 && inventory[ItemType::TELEPORT] > 0) {
				use_teleport();
				inventory[ItemType::TELEPORT]--;
				 postItemUse(player);
			}

			if (action == GLFW_PRESS && key == GLFW_KEY_3 && inventory[ItemType::SPEED_BOOST] > 0) {
				use_speed_boost();
				inventory[ItemType::SPEED_BOOST]--;
				 postItemUse(player);
			}

			// cheats!
			if (pressed_keys.find(GLFW_KEY_LEFT_CONTROL) != pressed_keys.end() && pressed_keys.find(GLFW_KEY_F) != pressed_keys.end()) {
				game_state.cheat_finish = true;
			}

			// Tell user about the item they just picked up (toggle with T if they are holding an item)
			if (!most_recent_collected_item.name.empty() && action == GLFW_PRESS && key == GLFW_KEY_T) {
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

			// Toggle Debugging mode
			if (key == GLFW_KEY_B && action == GLFW_PRESS) {
				debugging.in_debug_mode = !debugging.in_debug_mode;
			}

			// Toggle cutscene rtx
			if (action == GLFW_RELEASE) {
				if (key == GLFW_KEY_R) {
					rtx_on = !rtx_on;
					char* status;

					if (rtx_on) { status = "on"; }
					else { status = "off"; }

					std::cout << "RTX has been turned " << status << std::endl;
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
	if (state == ProgramState::RUNNING && !in_a_cutscene) {
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

					// ==== Feature: Items ====
					if (registry.view<WallBreakerTimer>().contains(player) && get_map_tile(target_map_pos) == MapTile::BREAKABLE_WALL) {
						game_state.level.map_tiles[(int)(target_map_pos.y)][(int)(target_map_pos.x)] = MapTile::FREE_SPACE;
						game_state.sound_requests.push_back({SoundEffects::ITEM_BREAK_WALL});
						wallbreaker_counter = 0;
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

// Cutscenes
void WorldSystem::do_cutscene() {
	// This is pretty much a hack to get around some issues with drawing images
	// Because of bufferswap in nuklear, we have to make sure two successive
	// frames have the speaker drawn, hence the flags. (Three successive frames
	// are required on load to give the maze walls time to draw/be colored in,
	// hence three flags frame_0, frame_1, and frame_2)

	// Gate 1
	if (cutscene_1_frame_0) {
		// Set the switches
		cutscene_1_frame_0 = false;
		cutscene_1_frame_1 = true;

		// To prevent jittering
		entt::entity player = registry.view<Player>().begin()[0];
		Motion& motion = registry.get<Motion>(player);
		pressed_keys.clear();
		motion.velocity = { 0,0 };
	}

	// Gate 2
	else if (cutscene_1_frame_1) {
		// Set the switches
		cutscene_1_frame_2 = true;
		cutscene_1_frame_1 = false;

		// ***** Set up the variables *****
		entt::entity player = registry.view<Player>().begin()[0];
		Motion& motion		= registry.get<Motion>(player);

		// These variables are used in main as well, to set the scales to 0 after the cutscene ends

		cutscene_minotaur_entity		 = registry.view<Cutscene>().begin()[2];
		cutscene_drone_entity			 = registry.view<Cutscene>().begin()[5];
		cutscene_drone_sad_entity		 = registry.view<Cutscene>().begin()[4];
		cutscene_drone_laughing_entity	 = registry.view<Cutscene>().begin()[3];
		cutscene_minotaur_rtx_off_entity = registry.view<Cutscene>().begin()[1];
		cutscene_drone_rtx_off_entity	 = registry.view<Cutscene>().begin()[0];

		Motion& cutscene_drone_motion			 = registry.get<Motion>(cutscene_drone_entity);
		Motion& cutscene_drone_sad_motion		 = registry.get<Motion>(cutscene_drone_sad_entity);
		Motion& cutscene_drone_laughing_motion	 = registry.get<Motion>(cutscene_drone_laughing_entity);
		Motion& cutscene_minotaur_motion		 = registry.get<Motion>(cutscene_minotaur_entity);
		Motion& cutscene_minotaur_rtx_off_motion = registry.get<Motion>(cutscene_minotaur_rtx_off_entity);
		Motion& cutscene_drone_rtx_off_motion	 = registry.get<Motion>(cutscene_drone_rtx_off_entity);

		// Determine which image to show and scale it up
		float scale_x = 900.f * global_scaling_vector.x;
		float scale_y = 800.f * global_scaling_vector.y;
		if (rtx_on) {
			if (cutscene_speaker == cutscene_speaker::SPEAKER_MINOTAUR) {
				cutscene_minotaur_motion.position		  = { motion.position.x - window_width_px / 4, motion.position.y + window_height_px / 7 };
				cutscene_minotaur_motion.scale			  = { scale_x, scale_y }; }
			else if (cutscene_speaker == cutscene_speaker::SPEAKER_DRONE) {
				cutscene_drone_motion.position            = { motion.position.x - window_width_px / 4, motion.position.y + window_height_px / 10 };
				cutscene_drone_motion.scale	              = { scale_x,scale_y }; }
			else if (cutscene_speaker == cutscene_speaker::SPEAKER_DRONE_SAD) {
				cutscene_drone_sad_motion.position		  = { motion.position.x - window_width_px / 4, motion.position.y + window_height_px / 10 };
				cutscene_drone_sad_motion.scale			  = { scale_x,scale_y }; }
			else if (cutscene_speaker == cutscene_speaker::SPEAKER_DRONE_LAUGHING) {
				cutscene_drone_laughing_motion.position	  = { motion.position.x - window_width_px / 4, motion.position.y + window_height_px / 10 };
				cutscene_drone_laughing_motion.scale	  = { scale_x,scale_y }; } }
		else {
			if (cutscene_speaker == cutscene_speaker::SPEAKER_MINOTAUR) {
				cutscene_minotaur_rtx_off_motion.position = { motion.position.x - window_width_px / 4, motion.position.y + window_height_px / 7 };
				cutscene_minotaur_rtx_off_motion.scale    = { scale_x,scale_y }; }
			else {
				cutscene_drone_rtx_off_motion.position    = { motion.position.x - window_width_px / 4, motion.position.y + window_height_px / 10 };
				cutscene_drone_rtx_off_motion.scale       = { scale_x,scale_y }; }
		}
	}

	// Gate 3
	else if (cutscene_1_frame_2) {
		// Reset switch
		cutscene_1_frame_2 = false;

		// Play audio files
		if      (cutscene_selection == 102) { game_state.sound_requests.push_back({ SoundEffects::DRONE_WERE_IT_ONLY_SO_EASY }); }
		else if (cutscene_selection == 10)  { game_state.sound_requests.push_back({ SoundEffects::DRONE_STUPID_BOY }); }
		else if (cutscene_speaker   == cutscene_speaker::SPEAKER_MINOTAUR
				 && cutscene_selection != 15
				 && cutscene_selection != 205
				 && cutscene_selection != 220
				 && cutscene_selection != 230) { game_state.sound_requests.push_back({ SoundEffects::HORSE_SNORT }); }

		// Set state to cutscene
		state = ProgramState::CUTSCENE1;
	}
}

// HUD
void WorldSystem::do_HUD() {
	entt::entity hud_player			  = registry.view<Player>().begin()[0];
	Motion& hud_player_motion		  = registry.get<Motion>(hud_player);
	Motion& hud_heart_1_motion		  = registry.get<Motion>(hud_heart_1_entity);
	Motion& hud_heart_2_motion		  = registry.get<Motion>(hud_heart_2_entity);
	Motion& hud_heart_3_motion		  = registry.get<Motion>(hud_heart_3_entity);
	Motion& hud_bg_motion			  = registry.get<Motion>(hud_bg_entity);
	Motion& hud_hammer_motion		  = registry.get<Motion>(hud_hammer_entity);
	Motion& hud_teleport_motion		  = registry.get<Motion>(hud_teleport_entity);
	Motion& hud_speedboost_motion	  = registry.get<Motion>(hud_speedboost_entity);
	Motion& hud_key_motion		      = registry.get<Motion>(hud_key_entity);
	Motion& hud_no_hammer_motion	  = registry.get<Motion>(hud_no_hammer_entity);
	Motion& hud_no_teleport_motion	  = registry.get<Motion>(hud_no_teleport_entity);
	Motion& hud_no_speedboost_motion  = registry.get<Motion>(hud_no_speedboost_entity);
	Motion& hud_no_key_motion		  = registry.get<Motion>(hud_no_key_entity);

	// **** Hearts ****
	// Update how many hearts there should be
	vec2 heart_scale = { 50.f * global_scaling_vector.x, 50.f * global_scaling_vector.y };
	vec2 item_scale  = { 50.f * global_scaling_vector.x, 50.f * global_scaling_vector.y };
	if (player_health == 3) {
		hud_heart_1_motion.scale = heart_scale;
		hud_heart_2_motion.scale = heart_scale;
		hud_heart_3_motion.scale = heart_scale; }
	else if (player_health == 2) {
		hud_heart_1_motion.scale = heart_scale;
		hud_heart_2_motion.scale = heart_scale;
		hud_heart_3_motion.scale = { 0,0 }; }
	else if (player_health == 1) {
		hud_heart_1_motion.scale = heart_scale;
		hud_heart_2_motion.scale = { 0,0 };
		hud_heart_3_motion.scale = { 0,0 }; }
	else if (player_health < 1) {
		hud_heart_1_motion.scale = { 0,0 };
		hud_heart_2_motion.scale = { 0,0 };
		hud_heart_3_motion.scale = { 0,0 };
	}
	// Update heart positions on screen
	vec2 heart_1_adj = { -window_width_px / 3 - 100 * global_scaling_vector.x, -window_height_px / 2.2  };
	vec2 heart_2_adj = { -window_width_px / 3								 , -window_height_px / 2.2  };
	vec2 heart_3_adj = { -window_width_px / 3 + 100 * global_scaling_vector.x, -window_height_px / 2.2  };
	hud_heart_1_motion.position =   { hud_player_motion.position.x + heart_1_adj.x, hud_player_motion.position.y + heart_1_adj.y };
	hud_heart_2_motion.position =   { hud_player_motion.position.x + heart_2_adj.x, hud_player_motion.position.y + heart_2_adj.y };
	hud_heart_3_motion.position =   { hud_player_motion.position.x + heart_3_adj.x, hud_player_motion.position.y + heart_3_adj.y };

	// **** HUD Background ****
	// Update HUD background position
	hud_bg_motion.position = { hud_player_motion.position.x - window_width_px/3, hud_player_motion.position.y - window_height_px/2.5 };

	// **** Items ****
	// Update what items should be displayed and their positions
	vec2 hammer_adj			= { -window_width_px / 3 - 150 * global_scaling_vector.x, -window_height_px / 2.9 };
	vec2 teleport_adj		= { -window_width_px / 3 - 50  * global_scaling_vector.x, -window_height_px / 2.9 };
	vec2 speedboost_adj		= { -window_width_px / 3 + 50  * global_scaling_vector.x, -window_height_px / 2.9 };
	vec2 hud_key_adj		= { -window_width_px / 3 + 150 * global_scaling_vector.x, -window_height_px / 2.9 };
	vec2 hud_hammer_pos		= { hud_player_motion.position.x + hammer_adj.x         , hud_player_motion.position.y + hammer_adj.y       };
	vec2 hud_teleport_pos	= { hud_player_motion.position.x + teleport_adj.x       , hud_player_motion.position.y + teleport_adj.y     };
	vec2 hud_speedboost_pos = { hud_player_motion.position.x + speedboost_adj.x     , hud_player_motion.position.y + speedboost_adj.y   };
	vec2 hud_key_pos        = { hud_player_motion.position.x + hud_key_adj.x      , hud_player_motion.position.y + hud_key_adj.y    };

	// Hammer
	if (inventory[ItemType::WALL_BREAKER] == 0) {
		hud_hammer_motion.scale		      = { 0,0 };
		hud_no_hammer_motion.scale        = item_scale;
		hud_no_hammer_motion.position     = hud_hammer_pos;
	}
	else if (inventory[ItemType::WALL_BREAKER] > 0) {
		hud_hammer_motion.scale		      = item_scale;
		hud_hammer_motion.position	      = hud_hammer_pos;
		hud_no_hammer_motion.scale	      = { 0,0 };
	}

	// Teleport
	if (inventory[ItemType::TELEPORT] == 0) {
		hud_teleport_motion.scale	      = { 0,0 };
		hud_no_teleport_motion.scale	  = item_scale;
		hud_no_teleport_motion.position   = hud_teleport_pos;
	}
	else if (inventory[ItemType::TELEPORT] > 0) {
		hud_teleport_motion.scale	      = item_scale;
		hud_teleport_motion.position      = hud_teleport_pos;
		hud_no_teleport_motion.scale      = { 0,0 };
	}

	// Speed boost
	if (inventory[ItemType::SPEED_BOOST] == 0) {
		hud_speedboost_motion.scale		  = { 0,0 };
		hud_no_speedboost_motion.scale    = item_scale;
		hud_no_speedboost_motion.position = hud_speedboost_pos;
	}
	else if (inventory[ItemType::SPEED_BOOST] > 0) {
		hud_speedboost_motion.scale		  = item_scale;
		hud_speedboost_motion.position	  = hud_speedboost_pos;
		hud_no_speedboost_motion.scale    = { 0,0 };
	}

	// Key
	if (inventory[ItemType::KEY] == 0) {
		hud_key_motion.scale			  = { 0,0 };
		hud_no_key_motion.scale		  = item_scale;
		hud_no_key_motion.position	  = hud_key_pos;
	}
	else if (inventory[ItemType::KEY] > 0) {
		hud_key_motion.scale			  = item_scale;
		hud_key_motion.position		  = hud_key_pos;
		hud_no_key_motion.scale		  = { 0,0 };
	}

	// Rest of the text handling is done in render_system.cpp's draw() function
}

// Item and Flashing Timers
void WorldSystem::do_timers(float elapsed_ms_since_last_update) {

	// Minotaur Flash
	flash_timer -= elapsed_ms_since_last_update;
	if (flash_timer <= 0) {
		registry.remove<Flash>(player_minotaur);
	}

	// Text
	if (!registry.view<TextTimer>().empty()) {
		TextTimer &counter = registry.get<TextTimer>(player_minotaur);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			registry.remove<TextTimer>(player_minotaur);
			most_recent_used_item = ItemType::NONE;
		}
	}

	// Speed Boost
	if (!registry.view<SpeedBoostTimer>().empty()) {
		SpeedBoostTimer& counter = registry.get<SpeedBoostTimer>(player_minotaur);
		counter.counter_ms -= elapsed_ms_since_last_update;
		speed_counter = counter.counter_ms; // capture speed_counter for HUD countdown
		if (counter.counter_ms < 0) {
			registry.remove<SpeedBoostTimer>(player_minotaur);
		}
	}

	// Wall Breaker
	if (!registry.view<WallBreakerTimer>().empty()) {
		WallBreakerTimer& counter = registry.get<WallBreakerTimer>(player_minotaur);
		counter.counter_ms -= elapsed_ms_since_last_update;
		wallbreaker_counter = counter.counter_ms; // capture wallbreaker_counter for HUD countdown
		if (counter.counter_ms < 0) {
			registry.remove<WallBreakerTimer>(player_minotaur);
		}
	}

	// Animation
	if (!registry.view<AnimationTimer>().empty()) {
		AnimationTimer& counter = registry.get<AnimationTimer>(player_minotaur);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			registry.remove<AnimationTimer>(player_minotaur);
			tips.used_item = 0;
		}
	}
}

// Handle Reaching Exit
void WorldSystem::do_exit() {
	Motion& player_motion = registry.get<Motion>(player_minotaur);
	MapTile tile = get_map_tile(position_to_map_coords(player_motion.position));

	// If player has the key and reaches the exit tile || cheat used...
	if ((tile == MapTile::EXIT && inventory[ItemType::KEY] >= required_num_of_keys) || game_state.cheat_finish) {
		game_state.cheat_finish = false;
		game_state.win_condition = true;

		// player has found the exit!
		if (!registry.view<EndGame>().contains(player_minotaur)) {
			registry.emplace<EndGame>(player_minotaur);
			game_state.sound_requests.push_back({SoundEffects::TADA});
			initial_game = false;
			do_pathfinding_movement = false;

			current_finish_time = game_time_ms;
			std::cout << "Finished game in " << formatTime(current_finish_time) << "!" << std::endl;

			// For cutscenes
			num_times_exit_reached++;

			if (num_times_exit_reached == 1) {
				cutscene_selection = 10;
				cutscene_speaker = cutscene_speaker::SPEAKER_DRONE;
			}
			else if (num_times_exit_reached > 1) {
				cutscene_selection = 15;
				cutscene_speaker = cutscene_speaker::SPEAKER_DRONE_SAD;
			}

			// For leaderboard
			std::string path = data_path() + "/leaderboard.yaml";
			std::ifstream file(path);
			if (!file.good()) {
				std::ofstream outfile(path);
				outfile.close();
			}
			file.close();

			YAML::Node leaderboard = YAML::LoadFile(path);

			std::vector<double> leaderboard_map = {};
			std::string leaderboard_id = game_state.level_id + "_" + std::to_string(game_state.level.phase);
			if (leaderboard[leaderboard_id]) leaderboard_map = leaderboard[leaderboard_id].as<std::vector<double>>();

			leaderboard_map.push_back(current_finish_time);
			std::sort(leaderboard_map.begin(), leaderboard_map.end());

			current_leaderboard.clear();
			std::cout << "LEVEL " << game_state.level_id;
			if (game_state.level.phase > 0) { // only if phase progression enabled
				std::cout << " PHASE " << game_state.level.phase;
			}
			std::cout << " LEADERBOARD:" << std::endl;

			for (int i = 0; i < leaderboard_map.size(); i++) {
				std::ostringstream str;
				str << i + 1 << ". " << formatTime(leaderboard_map[i]);
				if (leaderboard_map[i] == current_finish_time) str << " <-- YOUR POSITION!";
				current_leaderboard.push_back(str.str());

				std::cout << str.str() << std::endl;
			}

			leaderboard[leaderboard_id] = leaderboard_map;

			std::ofstream outfile(path);
			YAML::Emitter out;
			out << leaderboard;
			outfile << out.c_str();
			outfile.close();
		}
	}

	else if (tile == MapTile::EXIT && inventory[ItemType::KEY] < required_num_of_keys && play_need_key_cutscene){
		play_need_key_cutscene = false;
		// Play a cutscene explaining that they need to get the key
		cutscene_speaker = cutscene_speaker::SPEAKER_MINOTAUR;
		cutscene_selection = 300;
		cutscene_1_frame_0 = true;
	}
	// Once player steps out of exit tile, reset the trigger
	// So that when they reach the exit tile without a key again,
	// The cutscene will play
	if (tile == MapTile::FREE_SPACE) { play_need_key_cutscene = true; }
}

// Handle Death and Endgame
bool WorldSystem::do_death_and_endgame(float elapsed_ms_since_last_update) {
	float min_counter_ms = 3000.f;

	// For each entity in DeathTimer (can be enemy or player)
	for (entt::entity entity : registry.view<DeathTimer>()) {
		// Progress Death Timer
		DeathTimer& death_counter = registry.get<DeathTimer>(entity);
		death_counter.counter_ms -= elapsed_ms_since_last_update;

		// Keep enemies still
		player_is_manually_moving = false;
		do_pathfinding_movement = false;

		if (death_counter.counter_ms < min_counter_ms) { min_counter_ms = death_counter.counter_ms; }

		// If death timer expires...
		if (death_counter.counter_ms < 0) {

			// Remove entity from DeathTimer
			registry.remove<DeathTimer>(entity);
			
			// If the entity is the player...
			if (registry.view<Player>().contains(entity)) {
				// End invulnerability
				player_can_lose_health = true;
				// Stop player movement
				Motion& player_motion  = registry.get<Motion>(entity);
				player_motion.velocity = { 0,0 };
			}
			// If the entity is not the player, destroy it (i.e. destroy the enemy)
			else {
				registry.destroy(entity);
			}
			
			// Restart the game if the player is marked for death
			if (player_marked_for_death) {
				cutscene_1_frame_0 = true;
				if (death_count == 2) { cutscene_speaker = cutscene_speaker::SPEAKER_DRONE_LAUGHING; }
				else if (death_count == 4) { cutscene_speaker = cutscene_speaker::SPEAKER_DRONE_SAD; }
				else { cutscene_speaker = cutscene_speaker::SPEAKER_MINOTAUR; }
				cutscene_selection = 100 + death_count;

				state = ProgramState::GAME_OVER_DEAD;
				player_marked_for_death = false;

				return true;
			}
		}
	}

	for (entt::entity entity : registry.view<EndGame>()) {
		// progress timer
		EndGame& counter = registry.get<EndGame>(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.remove<EndGame>(entity);
			cutscene_reached_exit = true;
			cutscene_1_frame_0 = true;
			state = ProgramState::GAME_OVER_WIN;
			return true;
		}
	}

	return false;
}

// Remove Off-screen Entities
void WorldSystem::do_cleanup() {
	// Removing out of screen motion entities (excluding: cutscene, background, and hud entities)
	auto motions = registry.view<Motion>();
	for (auto entity : motions) {
		if (!registry.view<Cutscene>().contains(entity) && !registry.view<Background>().contains(entity) && !registry.view<HUD>().contains(entity)) {
			Motion& motion = motions.get<Motion>(entity);
			if (motion.position.x + abs(motion.scale.x) < 0.f) {
				std::cout << "Destroyed entity " << int(entity) << " via removing off screen" << std::endl;
				registry.destroy(entity);
			}
		}
	}
}

// Handle Tutorial
void WorldSystem::do_tutorial(float elapsed_ms_since_last_update) {

	// Get player position
	entt::entity player  = registry.view<Player>().begin()[0];
	Motion&		 motion	 = registry.get<Motion>(player);
	vec2		 map_pos = position_to_map_coords(motion.position);

	// Spawn initial entities
	if (!tutorial_flags["initial_spawned"]) {
		tutorial_flags["initial_spawned"] = true;

		// Spawn Items and Prisoners
		vec2 hammer_position = map_coords_to_position({ 1,5 });
		vec2 speed_position  = map_coords_to_position({ 7,3 });
		vec2 tele_position   = map_coords_to_position({ 28,2 });
		vec2 key_position	 = map_coords_to_position({ 30,2 });
		vec2 chick_position  = map_coords_to_position({ 1,1 });
		vec2 drone_position  = map_coords_to_position({ 4,1 });
		vec2 spike_position  = map_coords_to_position({ 4,5 });
		hammer_position += vec2(map_scale.x / 2, map_scale.y / 2);
		speed_position  += vec2(map_scale.x / 2, map_scale.y / 2);
		tele_position   += vec2(map_scale.x / 2, map_scale.y / 2);
		key_position    += vec2(map_scale.x / 2, map_scale.y / 2);
		spike_position  += vec2(map_scale.x / 2, map_scale.y / 2);
		drone_position  += vec2(map_scale.x / 2, map_scale.y / 2);
		chick_position  += vec2(map_scale.x / 2, map_scale.y / 2);
		createItem( renderer, hammer_position, "wall breaker");
		createItem( renderer, speed_position,  "speed boost");
		createItem( renderer, tele_position,   "teleporter");
		createItem( renderer, key_position,    "key");
		createSpike(renderer, spike_position);
		createDrone(renderer, drone_position);
		createChick(renderer, chick_position);
	}

	// Cutscene: In Cell
	if (!tutorial_flags["played_in_cell_cutscene"]) {
		tutorial_flags["played_in_cell_cutscene"] = true;
		cutscene_speaker = cutscene_speaker::SPEAKER_MINOTAUR;
		cutscene_selection = 200;
		cutscene_1_frame_0 = true;
	}

	// Cutscene: Note enemy movement
	vec2 movement_and_speed_trigger_pos = { 5,3 };
	if (map_pos == movement_and_speed_trigger_pos) {
		if (!tutorial_flags["noted_enemy_movement"]) {
			tutorial_flags["noted_enemy_movement"] = true;
			player_is_manually_moving = false;
			do_pathfinding_movement = false;
			motion.velocity = { 0,0 };
			cutscene_speaker = cutscene_speaker::SPEAKER_MINOTAUR;
			cutscene_selection = 205;
			cutscene_1_frame_0 = true;
			pressed_keys.clear();
		}
	}

	// Cutscene: Note Floor
	//vec2 floor_trigger_pos = { 19,3 };
	//if (map_pos == floor_trigger_pos) {
	//	if (!tutorial_flags["noted_floor"]) {
	//		tutorial_flags["noted_floor"] = true;
	//		player_is_manually_moving = false;
	//		do_pathfinding_movement = false;
	//		motion.velocity = { 0,0 };
	//		cutscene_speaker = cutscene_speaker::SPEAKER_MINOTAUR;
	//		cutscene_selection = 210;
	//		cutscene_1_frame_0 = true;
	//		pressed_keys.clear();
	//	}
	//}

	// Cutscene: Interrupt
	vec2 interrupt_trigger_pos = { 24, 3 };
	if (map_pos == interrupt_trigger_pos && !tutorial_flags["finished_interruption"]) {
		if (!tutorial_flags["interrupted"]) {
			tutorial_flags["interrupted"] = true;
			in_a_cutscene = true;
			motion.velocity = { 0,0 };
			cutscene_speaker = cutscene_speaker::SPEAKER_DRONE;
			cutscene_selection = 215;
			cutscene_1_frame_0 = true;
		}

	}

	// Spawn Daedalus
	if (tutorial_flags["interrupted"] && !tutorial_flags["spawned_daedalus"]) {
		tutorial_flags["spawned_daedalus"] = true;
		vec2 daedalus_position = map_coords_to_position({ 19,3 });
		daedalus_position += vec2(map_scale.x / 2, map_scale.y / 2);
		daedalus_entity = createDrone(renderer, daedalus_position);
	}

	// Move Daedalus to player
	if (tutorial_flags["interrupted"] && !tutorial_flags["do_interrupt_cutscene"]) {
		Motion& daedalus_motion = registry.get<Motion>(daedalus_entity);
		vec2 daedalus_map_pos = position_to_map_coords(daedalus_motion.position);
		// Turn player
		if (motion.velocity.x < 0) { motion.velocity.x = 0; }
		vec2 turn_player_trigger_pos = { 20,3 };
		if (daedalus_map_pos == turn_player_trigger_pos) { motion.velocity.x = -0.1; }
		// Stop Daedalus
		vec2 daedalus_stop_moving_trigger_pos = { 22,3 };
		if (daedalus_map_pos == daedalus_stop_moving_trigger_pos) {
			daedalus_motion.velocity.x = 0;
			tutorial_flags["do_interrupt_cutscene"] = true;
		}
		else {
			daedalus_motion.velocity.x = 100;
		}
	}

	// Daedalus speech
	if (tutorial_flags["interrupted"] && tutorial_flags["do_interrupt_cutscene"]) {
		tutorial_flags["do_interrupt_cutscene"] = false;
		tutorial_flags["interrupted"] = false;
		tutorial_flags["finished_interruption"] = true;
		in_a_cutscene = false;
		// Start cutscene
		cutscene_speaker = cutscene_speaker::SPEAKER_DRONE;
		cutscene_selection = 216;
		cutscene_1_frame_0 = true;

		pressed_keys.clear();

		Motion& daedalus_motion = registry.get<Motion>(daedalus_entity);
		daedalus_motion.velocity.x = -300;

		// Set flag for next stage
		tutorial_flags["phase_2"] = true;
	}

	// Remove Daedalus
	if (tutorial_flags["phase_2"] && !tutorial_flags["removed_daedalus"]) {
		Motion& daedalus_motion = registry.get<Motion>(daedalus_entity);
		float remove_trigger_x_coord = 2850.f;
		if (daedalus_motion.position.x <= remove_trigger_x_coord) {
			std::cout << "Destroyed entity " << int(daedalus_entity) << " for Daedalus crossing the line" << std::endl;
			registry.destroy(daedalus_entity);
			tutorial_flags["removed_daedalus"] = true;
			player_is_manually_moving = false;
			do_pathfinding_movement = false;
		}
	}

	// Spawn the goons
	if (tutorial_flags["phase_2"] && !tutorial_flags["p2_spawned_enemies"]) {
		tutorial_flags["p2_spawned_enemies"] = true;
			vec2		 pos;
		entt::entity ent;
		for (int i = 0; i < 4; i++) {
			switch (i) {
			case 0: pos = { 20,1 }; break;
			case 1: pos = { 19,2 }; break;
			case 2: pos = { 19,4 }; break;
			case 3: pos = { 20,5 }; break;
			}
			pos = map_coords_to_position(pos);
			pos += vec2(map_scale.x / 2, map_scale.y / 2);
			ent = createDrone(renderer, pos);
			tutorial_enemy_entities.push_back(ent);
		}
	}

	// Set drones on player
	if (tutorial_flags["phase_2"] && tutorial_flags["p2_spawned_enemies"]) {
		for (entt::entity entity : tutorial_enemy_entities) {
			if (registry.view<Motion>().contains(entity)) {
				Motion& entity_motion = registry.get<Motion>(entity);
					if (!tutorial_flags["removed_daedalus"]) { entity_motion.velocity = { 0,0   }; }
					else { entity_motion.velocity = { 200,0 }; }
			}
		}
	}

	// Spacebar to attack
	if (tutorial_flags["phase_2"] && !tutorial_flags["did_attack_cutscene"] && tutorial_flags["p2_spawned_enemies"] && tutorial_flags["removed_daedalus"]) {
		tutorial_flags["did_attack_cutscene"] = true;
		cutscene_speaker = cutscene_speaker::SPEAKER_MINOTAUR;
		cutscene_selection = 220;
		cutscene_1_frame_0 = true;
		pressed_keys.clear();
	}

	// Note teleporter and key
	vec2 teleporter_trigger_pos = { 28,3 };
	if (map_pos == teleporter_trigger_pos) {
		if (!tutorial_flags["noted_teleporter"]) {
			tutorial_flags["noted_teleporter"] = true;
			cutscene_speaker = cutscene_speaker::SPEAKER_MINOTAUR;
			cutscene_selection = 225;
			cutscene_1_frame_0 = true;
			pressed_keys.clear();
		}
	}

	// Note teleporter arrival
	vec2 arrival_trigger_pos = { 30,4 };
	if (map_pos == arrival_trigger_pos) {
		if (!tutorial_flags["noted_arrival"]) {
			tutorial_flags["noted_arrival"] = true;
			cutscene_speaker = cutscene_speaker::SPEAKER_MINOTAUR;
			cutscene_selection = 230;
			cutscene_1_frame_0 = true;
			pressed_keys.clear();
		}
	}


}
