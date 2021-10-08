// Header
#include "world_system.hpp"
#include "world_init.hpp"

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
const size_t MAX_TURTLES = 15;
const size_t MAX_FISH = 5;
const size_t TURTLE_DELAY_MS = 2000 * 3;
const size_t FISH_DELAY_MS = 5000 * 3;
const size_t ITEM_DELAY_MS = 3000 * 3;
SDL_Rect WorldSystem::camera = {0,0,1200,800};
float player_vel = 300;

// My Settings
auto t = Clock::now();
bool flag_right = false;
bool flag_left = false;
bool flag_fast = false;
bool active_spell = false;
float spell_timer = 6000.f;
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
	camera.x = registry.get<Motion>(player_salmon).position.x - screen_width / 2;
	camera.y = registry.get<Motion>(player_salmon).position.y - screen_height / 2;

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
		//if (entity != player_salmon) {
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
		// Create turtle
		entt::entity entity = createTurtle(renderer, {0,0});
		// Setting random initial position and constant velocity
		Motion& motion = registry.get<Motion>(entity);
		motion.mass = 200;
		motion.coeff_rest = 0.9;
		motion.position =
			vec2(screen_width -200.f,
				50.f + uniform_dist(rng) * (screen_height - 100.f));
		motion.velocity = vec2(-100.f, 0.f);
	}

	// Adjust turtle speed
	if (gesture_statuses["gesture_LMB_down"] && gesture_statuses["gesture_RMB_down"]) {
		for (entt::entity turtle : registry.view<HardShell>()) {
			Motion& motion = registry.get<Motion>(turtle);
			motion.velocity = vec2(-25.f, 0.f);
		}
	}
	else {
		for (entt::entity turtle : registry.view<HardShell>()) {
			Motion& motion = registry.get<Motion>(turtle);
			motion.velocity = vec2(-100.f, 0.f);
		}
	}
	
	// Spawning new fish
	next_fish_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.view<SoftShell>().size() <= MAX_FISH && next_fish_spawn < 0.f) {
		// !!!  TODO A1: Create new fish with createFish({0,0}), as for the Turtles above
		next_fish_spawn = (FISH_DELAY_MS / 2) + uniform_dist(rng) * (next_fish_spawn / 2);
		entt::entity fish = createFish(renderer, {0,0});
		// Setting random initial position and constant velocity
		Motion& motion = registry.get<Motion>(fish);
		motion.position =
			vec2(50.f + uniform_dist(rng) * (screen_width - 100.f), 
				 50.f + uniform_dist(rng) * (screen_height - 100.f));
		// motion.velocity = vec2(-200.f, 0.f);
		motion.velocity = vec2( (uniform_dist(rng) - 0.5f) * 200, 
				  (uniform_dist(rng) - 0.5f) * 200);
	}
	
	if (camera.x <= 0) {
		camera.x = 0;
	}
	if (camera.y <= 0) {
		camera.y = 0;
	}

	if (camera.x >= camera.w) {
		camera.x = camera.w;
	}

	if (camera.y >= camera.h) {
		camera.y = camera.h;
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
	player_vel = spellbook[1]["active"] == "true" ? 800 : 300;
	
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

	// Debugging for memory/component leaks
	//registry.list_all_components();
	//printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	registry.clear();

	// Debugging for memory/component leaks
	//registry.list_all_components();

	// Create a new salmon
	player_salmon = createSalmon(renderer, { 100, 200 });
	registry.emplace<Colour>(player_salmon, vec3(1, 0.8f, 0.8f));
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
			//Player& player = registry.players.get(entity);

			// Checking Player - HardShell collisions
			if (registry.view<HardShell>().contains(entity_other)) {
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

					// Below is the acceleration/flag-based movement implementation
					//move_right = false;
					//move_left = false;
					//move_up = false;
					//move_down = false;
				}
			}
			// Checking Player - SoftShell collisions
			else if (registry.view<SoftShell>().contains(entity_other)) {
				// if (!registry.view<DeathTimer>().contains(entity)) {
				// 	// chew, count points, and set the LightUp timer
				// 	registry.destroy(entity_other);
				// 	Mix_PlayChannel(-1, salmon_eat_sound, 0);
				// 	++points;

				// 	// !!! TODO A1: create a new struct called LightUp in components.hpp and add an instance to the salmon entity by modifying the ECS registry
				// }
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

	// Below is the acceleration/flag-based movement implementation

	entt::entity player = registry.view<Player>().begin()[0];
	Motion& motion = registry.get<Motion>(player);

	//if (!registry.view<DeathTimer>().contains(player)) {
	//	if (action == GLFW_PRESS) {
	//		if		(key == GLFW_KEY_D	|| key == GLFW_KEY_RIGHT) { move_right = true;	}
	//		else if (key == GLFW_KEY_A  || key == GLFW_KEY_LEFT ) { move_left  = true;	}
	//		if		(key == GLFW_KEY_W	|| key == GLFW_KEY_UP	) { move_up    = true;	}
	//		else if (key == GLFW_KEY_S  || key == GLFW_KEY_DOWN ) { move_down  = true;	}
	//	}

	//	if (action == GLFW_RELEASE) {
	//		if		(key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) { move_right = false;	}
	//		else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT)  { move_left  = false;	}
	//		if		(key == GLFW_KEY_W || key == GLFW_KEY_UP)	 { move_up    = false;	}
	//		else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)  { move_down  = false;	}
	//	}
	//}

	if (!registry.view<DeathTimer>().contains(player)) {
		if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_W || key == GLFW_KEY_UP) { motion.velocity[1] = -1 * player_vel; }
			else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) { motion.velocity[0] = -1 * player_vel; }
			if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) { motion.velocity[0] = player_vel; }
			else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) { motion.velocity[1] = player_vel; }
		}

		if (action == GLFW_RELEASE) {
			if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) { motion.velocity[0] = 0; }
			else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) { motion.velocity[0] = 0; }
			if (key == GLFW_KEY_W || key == GLFW_KEY_UP) { motion.velocity[1] = 0; }
			else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) { motion.velocity[1] = 0; }
		}
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		restart_game();
	}
}

void WorldSystem::on_mouse_button(int button, int action, int mods) {

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


