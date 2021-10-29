
#define GL3W_IMPLEMENTATION
#include <GL/glew.h>

// stlib
#include <chrono>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "common.hpp"
#include "menu.h"

using Clock = std::chrono::high_resolution_clock;

const int window_width_px = 1200;
const int window_height_px = 800;

extern entt::registry registry;

typedef enum GameStates {
	MENU,
	INIT,
	RUNNING,
	PAUSED,
	OVER
};

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem ai;
	GameState state = MENU;

	// Initializing window
	GLFWwindow* window = world.create_window(window_width_px, window_height_px);
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}
	
	if (state == MENU) {
		draw(window, window_width_px, window_height_px);
	}
	else {
		// initialize the main systems
		renderer.init(window_width_px, window_height_px, window);
		world.init(&renderer);

		// variable timestep loop
		auto t = Clock::now();
		while (!world.is_over()) {
			// Processes system messages, if this wasn't present the window would become
			// unresponsive
			glfwPollEvents();

			// Calculating elapsed times in milliseconds from the previous iteration
			auto now = Clock::now();
			float elapsed_ms =
				(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
			t = now;

			world.step(elapsed_ms);
			ai.step(elapsed_ms);
			physics.step(elapsed_ms, window_width_px, window_height_px);
			world.handle_collisions();

			renderer.draw();

			// TODO A2: you can implement the debug freeze here but other places are possible too.
		}

	}

	return EXIT_SUCCESS;
}
