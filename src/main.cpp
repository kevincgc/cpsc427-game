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

int window_width_px = 1200;
int window_height_px = 800;
// global scaler according to resolution of screen - modified in WorldSystem::createWindow
vec2 global_scaling_vector = { 1.f, 1.f };
// map scale used to transform map coordinates to pixels
vec2 map_scale = { 150.f, 150.f };

extern entt::registry registry;

extern "C" {
	void initMainMenu( GLFWwindow* win, int window_width_px, int window_height_px, float scale_x_in, float scale_y_in);
	void drawMainMenu(GLFWwindow* window, int* is_start_game);
	void drawOptionsMenu( GLFWwindow* win, int* out);
	void drawPauseMenu( GLFWwindow* win, int* out);
	void drawGameOverMenu( GLFWwindow* win, int* out, int player_win);

	void initOptionsMenu();
	void closeOptionsMenu();
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
	bool has_completed_init = false;
	auto t = Clock::now();

	while (!world.is_over()) {
		glfwPollEvents();
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		switch (state) {
		case ProgramState::INIT:
			window = world.create_window();
			if (!window) {
				printf("Press any key to exit");
				getchar();
				return EXIT_FAILURE;
			}
			initMainMenu(window, window_width_px, window_height_px, global_scaling_vector.x, global_scaling_vector.y);
			state = ProgramState::MAIN_MENU;
			break;
		case ProgramState::MAIN_MENU:
		{
			int selection = 0;
			drawMainMenu(window, &selection);
			if (selection == 1 && !has_completed_init) {
				renderer.init(window_width_px, window_height_px, window);
				world.init(&renderer);
				has_completed_init = true;
				state = ProgramState::RUNNING;
			}
			else if (selection == 1 && has_completed_init) {
				state = ProgramState::RESET_GAME;
			}
			else if (selection == 2) {
				initOptionsMenu();
				state = ProgramState::OPTIONS;
			}
			else if (selection == -1) {
				state = ProgramState::EXIT;
			}
			break;
		}
		case ProgramState::OPTIONS:
		{
			int selection = 0;
			drawOptionsMenu(window, &selection);
			if (selection == -1) {
				closeOptionsMenu();
				state = ProgramState::MAIN_MENU;
			}
			break;
		}
		case ProgramState::RESET_GAME:
			renderer.reinit(window_width_px, window_height_px, window);
			world.restart_game();
			state = ProgramState::RUNNING;
			break;
		case ProgramState::RUNNING:
		{
			world.step(elapsed_ms);
			ai.step();
			physics.step(elapsed_ms, window_width_px, window_height_px);
			world.handle_collisions();
			renderer.draw();
			break;
		}
		case ProgramState::PAUSED:
		{
			int selection = 0;
			drawPauseMenu(window, &selection);
			switch (selection) {
			case 1:
				renderer.reinit(window_width_px, window_height_px, window);
				state = ProgramState::RUNNING;
				break;
			case 2:
				state = ProgramState::RESET_GAME;
				break;
			case 3:
				state = ProgramState::MAIN_MENU;
				break;
			case -1:
				state = ProgramState::EXIT;
				break;
			}
			break;
		}
		case ProgramState::GAME_OVER_DEAD:
		case ProgramState::GAME_OVER_WIN:
		{
			int selection = 0;
			drawGameOverMenu(window, &selection, state == ProgramState::GAME_OVER_WIN);
			switch (selection) {
			case 1:
				state = ProgramState::RESET_GAME;
				break;
			case 2:
				state = ProgramState::MAIN_MENU;
				break;
			case -1:
				state = ProgramState::EXIT;
				break;
			}
			break;
		}
		default:
			return 0;
		}
	}

	return EXIT_SUCCESS;
}
