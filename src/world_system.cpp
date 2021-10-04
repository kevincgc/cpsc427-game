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
#include <map>
#include <iterator>
#include <string>

// Game configuration
const size_t MAX_TURTLES = 15;
const size_t MAX_FISH = 5;
const size_t TURTLE_DELAY_MS = 2000 * 3;
const size_t FISH_DELAY_MS = 5000 * 3;

// My Settings
bool flag_right = false;
bool flag_left = false;
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
//bool gesture_left_north = false;
//bool gesture_left_south = false;
//bool gesture_left_east = false;
//bool gesture_left_west = false;
//bool gesture_right_north = false;
//bool gesture_right_south = false;
//bool gesture_right_east = false;
//bool gesture_right_west = false;
// The number of swipes allowed before resetting cast
int max_swipes = 2;
int swipes = 0;

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

	// Remove debug info from the last step
	//while (registry.debugComponents.entities.size() > 0)
	//    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto motions = registry.view<Motion>();

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	//for (int i = (int)motions_registry.size()-1; i>=0; --i) {
	//    auto& motion = motions_registry.begin()[i];
	//	if (motion.position.x + abs(motion.scale.x) < 0.f) {
	//	    registry.remove_all_components_of(motions_registry.entities[i]);
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
		motion.position =
			vec2(screen_width -200.f,
				 50.f + uniform_dist(rng) * (screen_height - 100.f));
		motion.velocity = vec2(-100.f, 0.f);
	}

	// Spawning new fish
	next_fish_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.view<SoftShell>().size() <= MAX_FISH && next_fish_spawn < 0.f) {
		// !!!  TODO A1: Create new fish with createFish({0,0}), as for the Turtles above
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE PEBBLE SPAWN HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Processing the salmon state
	// assert(registry.screenStates.components.size() <= 1);
    // ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (entt::entity entity: registry.view<DeathTimer>()) {
		// progress timer
		DeathTimer& counter = registry.get<DeathTimer>(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.remove<DeathTimer>(entity);
			//screen.darken_screen_factor = 0;
            restart_game();
			return true;
		}
	}
	
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

	// !! TODO A3: Enable static pebbles on the ground
	// Create pebbles on the floor for reference
	/*
	for (uint i = 0; i < 20; i++) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		float radius = 30 * (uniform_dist(rng) + 0.3f); // range 0.3 .. 1.3
		Entity pebble = createPebble({ uniform_dist(rng) * w, h - uniform_dist(rng) * 20 },
			         { radius, radius });
		float brightness = uniform_dist(rng) * 0.5 + 0.5;
		registry.colors.insert(pebble, { brightness, brightness, brightness});
	}
	*/
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
				if (!registry.view<DeathTimer>().contains(entity)) {
					// Scream, reset timer, and make the salmon sink
					registry.emplace<DeathTimer>(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);
					registry.get<Motion>(entity).angle = 3.1415f;
					registry.get<Motion>(entity).velocity = { 0, 80 };

					// !!! TODO A1: change the salmon color on death
				}
			}
			// Checking Player - SoftShell collisions
			else if (registry.view<SoftShell>().contains(entity_other)) {
				if (!registry.view<DeathTimer>().contains(entity)) {
					// chew, count points, and set the LightUp timer
					registry.destroy(entity_other);
					Mix_PlayChannel(-1, salmon_eat_sound, 0);
					++points;

					// !!! TODO A1: create a new struct called LightUp in components.hpp and add an instance to the salmon entity by modifying the ECS registry
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
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	entt::entity salmon = registry.view<Player>().begin()[0];
	Motion& motion = registry.get<Motion>(salmon);
	const float salmon_vel = 500;
	if (!registry.view<DeathTimer>().contains(salmon)) {
		if (action == GLFW_PRESS && key == GLFW_KEY_LEFT) {
			motion.velocity[0] = -1 * salmon_vel;
		}

		if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT) {
			motion.velocity[0] = 0;
		}

		if (action == GLFW_PRESS && key == GLFW_KEY_RIGHT) {
			motion.velocity[0] = salmon_vel;
		}

		if (action == GLFW_RELEASE && key == GLFW_KEY_RIGHT) {
			motion.velocity[0] = 0;
		}

		if (action == GLFW_PRESS && key == GLFW_KEY_UP) {
			motion.velocity[1] = -1 * salmon_vel;
		}

		if (action == GLFW_RELEASE && key == GLFW_KEY_UP) {
			motion.velocity[1] = 0;
		}

		if (action == GLFW_PRESS && key == GLFW_KEY_DOWN) {
			motion.velocity[1] = salmon_vel;
		}

		if (action == GLFW_RELEASE && key == GLFW_KEY_DOWN) {
			motion.velocity[1] = 0;
		}
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);

}

void WorldSystem::on_mouse_button(int button, int action, int mods) {
	// Mouse actions are one of: GLFW_PRESS or GLFW_RELEASE
	// Get mouse button state (GFLW_PRESS or GLFW_RELEASE) with ex: 
	//   int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)

	// Get cursor position
	POINT p;
	GetCursorPos(&p);

	// Access mouse_spell helper functions
	Mouse_spell mouse_spell;

	// Forgiveness range
	// The leniency refers to whether the cursor x or y value strays too far from the origin
	// Example: start: (100,100). Intent: Up motion. End: (110,40). Interpretation: 10 right, 60 up.
	// Is 10 right okay? If leniency is 20, then yes, 10 is acceptable, and we register this as swiping up.
	float forgiveness_range = 100;
	// The distance the cursor must travel to register as a swipe
	float min_distance = 100;

	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			flag_right = true;
		}
		else if (action == GLFW_RELEASE) {
			flag_right = false;

			vec2 first = { gesture_coords_right.front().x , gesture_coords_right.front().y };
			vec2 last = { gesture_coords_right.back().x, gesture_coords_right.back().y };
			float dif_x = last.x - first.x;
			float dif_y = last.y - first.y;
			
			// Debug
			//std::cout << "First element: " << first.x << ", " << first.y << std::endl;
			//std::cout << "Last element: " << last.x << ", " << last.y << std::endl;
			//std::cout << "Dif_x: " << dif_x << std::endl;
			//std::cout << "Dif_y: " << dif_y << std::endl;

			if (dif_x > min_distance && abs(dif_y) < forgiveness_range) {
				std::cout << "RMB_swipe_right" << std::endl;
				gesture_statuses["gesture_RMB_right"] = true;
				mouse_spell.reset_swipe_status(gesture_statuses, "LMB", "right");
			}
			else if (dif_x < -1 * min_distance && abs(dif_y) < forgiveness_range) {
				std::cout << "RMB_swipe_left" << std::endl;
				gesture_statuses["gesture_RMB_left"] = true;
				mouse_spell.reset_swipe_status(gesture_statuses, "LMB", "left");
			}
			else if (abs(dif_x) < forgiveness_range && dif_y > min_distance) {
				std::cout << "RMB_swipe_down" << std::endl;
				gesture_statuses["gesture_RMB_down"] = true;
				mouse_spell.reset_swipe_status(gesture_statuses, "LMB", "down");
			}
			else if (abs(dif_x) < forgiveness_range && dif_y < -1 * min_distance) {
				std::cout << "RMB_swipe_up" << std::endl;
				gesture_statuses["gesture_RMB_up"] = true;
				mouse_spell.reset_swipe_status(gesture_statuses, "LMB", "up");
			}
			gesture_coords_right.clear();

			// Check Spell Cast
			mouse_spell.check_spell(gesture_statuses);

			// Debug: Print the gesture_statuses map
			//for (auto it = gesture_statuses.cbegin(); it != gesture_statuses.cend(); ++it) {
			//	std::cout << it->first << " " << it->second << std::endl;
			//}

		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			//std::cout << "clicked_left: " << p.x << ", " << p.y << std::endl;
			flag_left = true;
		}
		else if (action == GLFW_RELEASE) {
			flag_left = false;

			vec2 first = { gesture_coords_left.front().x , gesture_coords_left.front().y };
			vec2 last = { gesture_coords_left.back().x, gesture_coords_left.back().y };
			float dif_x = last.x - first.x;
			float dif_y = last.y - first.y;

			// Debug
			//std::cout << "First element: " << first.x << ", " << first.y << std::endl;
			//std::cout << "Last element: " << last.x << ", " << last.y << std::endl;
			//std::cout << "Dif_x: " << dif_x << std::endl;
			//std::cout << "Dif_y: " << dif_y << std::endl;

			if (dif_x > min_distance && abs(dif_y) < forgiveness_range) {
				std::cout << "LMB_swipe_right" << std::endl;
				gesture_statuses["gesture_LMB_right"] = true;
				mouse_spell.reset_swipe_status(gesture_statuses, "RMB", "right");
			}
			else if (dif_x < -1 * min_distance && abs(dif_y) < forgiveness_range) {
				std::cout << "LMB_swipe_left" << std::endl;
				gesture_statuses["gesture_LMB_left"] = true;
				mouse_spell.reset_swipe_status(gesture_statuses, "RMB", "left");
			}
			else if (abs(dif_x) < forgiveness_range && dif_y > min_distance) {
				std::cout << "LMB_swipe_down" << std::endl;
				gesture_statuses["gesture_LMB_down"] = true;
				mouse_spell.reset_swipe_status(gesture_statuses, "RMB", "down");
			}
			else if (abs(dif_x) < forgiveness_range && dif_y < -1 * min_distance) {
				std::cout << "LMB_swipe_up" << std::endl;
				gesture_statuses["gesture_LMB_up"] = true;
				mouse_spell.reset_swipe_status(gesture_statuses, "RMB", "up");
			}
			gesture_coords_left.clear();

			// Check Spell Cast
			mouse_spell.check_spell(gesture_statuses);

			// Debug: Print the gesture_statuses map
			//for (auto it = gesture_statuses.cbegin(); it != gesture_statuses.cend(); ++it) {
			//	std::cout << it->first << " " << it->second << std::endl;
			//}
		}
	}

}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// If RMB down
	if (flag_right) {
		//std::cout << "Tracking right... " << p.x << ", " << p.y << std::endl;
		gesture_coords_right.push_back({ mouse_position.x,mouse_position.y });
	}

	// If LMB down
	if (flag_left) {
		//std::cout << "Tracking left..." << p.x << ", " << p.y << std::endl;
		gesture_coords_left.push_back({ mouse_position.x,mouse_position.y });
	}
}
