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

// Game configuration
const size_t MAX_TURTLES = 0; // Setting to 0 for pathfinding debugging
const size_t MAX_FISH = 0; // Setting to 0 for pathfinding debugging
const size_t TURTLE_DELAY_MS = 2000 * 3;
const size_t FISH_DELAY_MS = 5000 * 3;
const size_t ITEM_DELAY_MS = 3000 * 3;
vec2 WorldSystem::camera = {0, 0};
float player_vel = 300.f;

// My Settings
auto t = Clock::now();
bool flag_right = false;
bool flag_left = false;
bool flag_fast = false;
bool active_spell = false;
float spell_timer = 6000.f;
vec2 path_target_map_pos; // For pathfinding feature
extern float softshell_scale = 75.f; // !!! hardcoded to 75.f, to be optimized, need to be the same with sprite scale
vec2 starting_map_pos;
vec2 ending_map_pos;
bool do_generate_path = false;


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
	{2, {
			{"name", "slowdown"},
			{"speed", "slow"},
			{"combo_1", "gesture_LMB_down"},
			{"combo_2", "gesture_RMB_down"},
			{"active", "false"}
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

// Below is the acceleration/flag-based movement implementation
////Movement
//bool move_right = false;
//bool move_left = false;
//bool move_up = false;
//bool move_down = false;

//Debugging
vec2 debug_pos = { 0,0 };

// Create the fish world
WorldSystem::WorldSystem()
	: points(0)
	, next_turtle_spawn(0.f)
	, next_fish_spawn(0.f) {
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
	if (registry.view<HardShell>().size() <= MAX_TURTLES && next_turtle_spawn < 0.f) {
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

	// Adjust turtle speed
	if (gesture_statuses["gesture_LMB_down"] && gesture_statuses["gesture_RMB_down"]) {
		for (entt::entity turtle : registry.view<HardShell>()) {
			Motion& motion = registry.get<Motion>(turtle);
			motion.velocity = vec2(-25.f, 0.f);
		}
	}
	//else {
	//	for (entt::entity turtle : registry.view<HardShell>()) {
	//		Motion& motion = registry.get<Motion>(turtle);
	//		motion.velocity = vec2(-100.f, 0.f);
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
				spell_timer = 6000;
				active_spell = false;
			}
		}
	}

	// Temporary implementation: Handle speed-up spell: Player moves faster
	player_vel = spellbook[1]["active"] == "true" ? 800.f : 300.f;

	// Temporary implementation: Player movement being handled in step - continue later if needed
	//entt::entity player = registry.view<Player>().begin()[0];

	//if (!registry.view<DeathTimer>().contains(player)) {
	//	Motion& motion = registry.get<Motion>(player);
	//	float y_pos = motion.position[1];
	//	float x_pos = motion.position[0];

	//	if (move_up)	{ y_pos = -1 * player_vel;  }
	//	if (move_left)  { x_pos = -1 * player_vel;  }
	//	if (move_down)	{ y_pos = player_vel;		}
	//	if (move_right) { x_pos = player_vel;		}
	//}

	// process player flash timer
	flash_timer -= elapsed_ms_since_last_update;
	if (flash_timer <= 0) {
		registry.remove<Flash>(player_minotaur);
	}


	// Pathfinding debugging
	// Print player position in map coords
	entt::entity player = registry.view<Player>().begin()[0];
	Motion& player_motion = registry.get<Motion>(player);
	vec2 debug_pos = position_to_map_coords(player_motion.position);
	//std::cout << debug_pos.x << ", " << debug_pos.y << std::endl; // starting pos: 0,1 means column 0, row 1,

	// Clicking creates map position



	return true;



}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// delete old map, if one exists
	game_state.map_tiles.clear();

	// load map
	fprintf(stderr, "Started loading map 1\n");
	std::ifstream file(maps_path("map1.txt"));
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) { // read one line from file
			std::istringstream str_stream(line);

			int tile;
			std::vector<MapTile> row;
			while (str_stream >> tile) { // read all tiles from this line
				row.push_back((MapTile) tile);
			}

			// push this map row to the final vector
			game_state.map_tiles.push_back(row);
		}
	}
	fprintf(stderr, "Finished loading map\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	registry.clear();

	// Create a new Minotaur
	player_minotaur = createMinotaur(renderer, { map_scale * 0.5, map_scale * 1.5 });
	registry.emplace<Colour>(player_minotaur, vec3(1, 0.8f, 0.8f));

	// reset player flash timer
	flash_timer = 1000.f;
	registry.emplace<Flash>(player_minotaur);
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

			// Checking Player - HardShell collisions
			if (registry.view<HardShell>().contains(entity_other) || registry.view<SoftShell>().contains(entity_other)) {
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
			// Checking Player - SoftShell collisions
			else if (registry.view<SoftShell>().contains(entity_other)) {
				// TODO: Implement other character actions
				// if (!registry.view<DeathTimer>().contains(entity)) {
				// 	// chew, count points, and set the LightUp timer
				// 	registry.destroy(entity_other);
				// 	Mix_PlayChannel(-1, salmon_eat_sound, 0);
				// 	++points;
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
	Motion& motion		= registry.get<Motion>(player);

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
			motion.velocity = {0, 0};

			if (pressed_keys.find(GLFW_KEY_UP) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_W) != pressed_keys.end()) {
				motion.velocity.y = -1 * player_vel;
			}

			if (pressed_keys.find(GLFW_KEY_LEFT) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_A) != pressed_keys.end()) {
				motion.velocity.x = -1 * player_vel;
			}

			if (pressed_keys.find(GLFW_KEY_RIGHT) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_D) != pressed_keys.end()) {
				motion.velocity.x = player_vel;
			}

			if (pressed_keys.find(GLFW_KEY_DOWN) != pressed_keys.end() || pressed_keys.find(GLFW_KEY_S) != pressed_keys.end()) {
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

			entt::entity player = registry.view<Player>().begin()[0];
			Motion& player_motion = registry.get<Motion>(player);

			// Implementation: There's a difference between camera coords and world coords.
			// glfwGetCursorPos gets the position of the cursor in the window. So clicking
			// the center of the window returns, for example, 1200/2=600 and 800/2=400.
			// But the player's coords are different. Even though the player is in the
			// center of the window, it's **world** coords are actually (initially) (75,225).
			// So we need to get the coords relative to the player

			// Get cursor screen coords
			vec2 cursor_screen_pos = { float(x_pos_release - window_width_px/2), float(y_pos_release - window_height_px/2) };

			// Get cursor world coords
			vec2 target_world_pos = { player_motion.position.x + cursor_screen_pos.x, player_motion.position.y + cursor_screen_pos.y };
			// std::cout << "cursor_world_pos " << target_world_pos.x << ", " << target_world_pos.y << std::endl;

			// Get cursor map coords (returns something like (0,1)) representing column 0, row 1.
			vec2 target_map_pos = position_to_map_coords(target_world_pos);

			// Only continue if clicked a traversable node (i.e. not a wall)
			if (get_map_tile(target_map_pos) == FREE_SPACE) {
				// Generate travel path (list of nodes)
				vec2 player_map_pos = position_to_map_coords(player_motion.position);
				starting_map_pos = player_map_pos;
				ending_map_pos = target_map_pos;
				do_generate_path = true;
				//AISystem::generate_path(player_map_pos, target_map_pos);

				// Debugging: Print coords
				std::cout << "player map pos: " << player_map_pos.x << ", " << player_map_pos.y << std::endl; // starting pos: 0,1 means column 0, row 1,
				std::cout << "released cursor map pos: " << target_map_pos.x << ", " << target_map_pos.y << std::endl; // 3,2
				//std::cout << "released cursor screen pos: " << path_target_screen_pos.x << ", " << path_target_screen_pos.y << std::endl; // ~588,405
				//std::cout << "player screen pos: "			<< player_motion.position.x << ", " << player_motion.position.y << std::endl; // starting pos: 75,225

			}
			else {
				std::cout << "Clicked on a wall!" << std::endl;
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
		&& map_coords.y < game_state.map_tiles.size()
		&& map_coords.x >= 0
		&& map_coords.x < game_state.map_tiles[(int)(map_coords.y)].size();
}

MapTile WorldSystem::get_map_tile(vec2 map_coords) {
	if (WorldSystem::is_within_bounds(map_coords)) return game_state.map_tiles[(int)(map_coords.y)][(int)(map_coords.x)];

	return MapTile::FREE_SPACE; // out of bounds
}


