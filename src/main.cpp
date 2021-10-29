
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

using Clock = std::chrono::high_resolution_clock;

const int window_width_px = 1200;
const int window_height_px = 800;

extern entt::registry registry;

extern "C" {
	void initMainMenu(static GLFWwindow* win, int window_width_px, int window_height_px);
	void drawMainMenu(GLFWwindow* window, int *is_start_game);
}

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem ai;
	GLFWwindow* window;
	auto t = Clock::now();

	while (!world.is_over()) {
		glfwPollEvents();
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		switch (state) {
		case ProgramState::INIT:
			window = world.create_window(window_width_px, window_height_px);
			if (!window) {
				printf("Press any key to exit");
				getchar();
				return EXIT_FAILURE;
			}
			initMainMenu(window, window_width_px, window_height_px);
			state = ProgramState::MENU;
			break;
		case ProgramState::MENU:
		{
			int out = 0;
			drawMainMenu(window, &out);
			if (out == 1) {
				out = 0;
				state = ProgramState::START_GAME;
			}
			else if (out == -1) {
				state = ProgramState::EXIT;
			}
			break;
		}
		case ProgramState::START_GAME:
			renderer.init(window_width_px, window_height_px, window);
			world.init(&renderer);
			t = Clock::now();
			state = ProgramState::RUNNING;
			break;
		case ProgramState::RESET_GAME:
			world.restart_game();
			state = ProgramState::RUNNING;
			break;
		case ProgramState::RUNNING:
		{
			printf("Running");
			world.step(elapsed_ms);
			ai.step(elapsed_ms);
			physics.step(elapsed_ms, window_width_px, window_height_px);
			world.handle_collisions();
			renderer.draw();
			break;
		}
		case ProgramState::PAUSED:
		{
			printf("Paused");
			renderer.draw();
			break;
		}
		default:
			return 0;
		}
	}

	return EXIT_SUCCESS;
}
