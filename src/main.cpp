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
float game_start_time;
bool initial_game;

extern "C" {
	void initMainMenu( GLFWwindow* win, int window_width_px, int window_height_px, float scale_x_in, float scale_y_in);
	void drawMainMenu(GLFWwindow* window, int* is_start_game);
	void drawOptionsMenu( GLFWwindow* win, int* out);
	void drawPauseMenu( GLFWwindow* win, int* out);
	void drawGameOverMenu( GLFWwindow* win, int* out, int player_win, int leaderboard_size, char **leaderboard, const char *player_time, const char *level_info, int progression);

	void drawCutscene(GLFWwindow* win, int* out);

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
	physics.addObserver(&world);

	while (!world.is_over()) {
		glfwPollEvents();
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		world.play_sounds();

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
				world.restart_game();
				has_completed_init = true;
				initial_game = true;
				game_start_time = (float)(glfwGetTime());
				state = ProgramState::RUNNING;
			}
			else if (selection == 1 && has_completed_init) {
				state = ProgramState::RESET_GAME;
			}
			else if (selection == 2) {
				initOptionsMenu();
				state = ProgramState::OPTIONS;
			}
			else if (selection == 3) {
				if (has_completed_init) {
					renderer.reinit(window_width_px, window_height_px, window);
					world.load_game();
					initial_game = true;
					game_start_time = (float)(glfwGetTime());
					state = ProgramState::RUNNING;
				} else {
					renderer.init(window_width_px, window_height_px, window);
					world.init(&renderer);
					world.load_game();
					has_completed_init = true;
					initial_game = true;
					game_start_time = (float)(glfwGetTime());
					state = ProgramState::RUNNING;
				}
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
			initial_game = true;
			game_start_time = (float)(glfwGetTime());
			state = ProgramState::RUNNING;
			break;

		case ProgramState::RUNNING:
		{

			world.step(elapsed_ms);
			ai.step();
			physics.step(elapsed_ms, window_width_px, window_height_px);
			renderer.draw();

			break;
		}
		case ProgramState::PAUSED:
		{
			int selection = 0;
			drawPauseMenu(window, &selection);
			switch (selection) {
			case 2:
				state = ProgramState::RESET_GAME;
				break;
			case 3:
				state = ProgramState::MAIN_MENU;
				break;
			case 4:
				world.save_game();
				// onto case 1
			case 1:
				renderer.reinit(window_width_px, window_height_px, window);
				state = ProgramState::RUNNING;
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

			if (state == ProgramState::GAME_OVER_WIN) {
				std::vector<std::string> leaderboard = world.get_leaderboard();
				auto size_send = min((int) leaderboard.size(), 10);

				char **leaderboard_array = new char *[size_send];
				for (uint i = 0; i < size_send; i++) {
					auto len = strlen(leaderboard[i].c_str());
					leaderboard_array[i] = new char[len + 1];
					strncpy(leaderboard_array[i], leaderboard[i].c_str(), len + 1);
				}

				int progression = 1;
				if (game_state.level.phase > 0) {
					progression = 3;
				} else if (game_state.level.has_next) {
					progression = 2;
				}

				drawGameOverMenu(window, &selection, 1, size_send, leaderboard_array, world.get_player_time().c_str(), world.get_level_info().c_str(), progression);

				for (int i = 0; i < size_send; i++) {
					delete[] leaderboard_array[i];
				}
				delete[] leaderboard_array;
			} else {
				drawGameOverMenu(window, &selection, 0, 0, nullptr, nullptr, nullptr, 0);
			}

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


		case ProgramState::CUTSCENE1:
		{
			entt::entity player						 = registry.view<Player>().begin()[0];
			Motion& motion							 = registry.get<Motion>(player);
			Motion& cutscene_drone_motion			 = registry.get<Motion>(cutscene_drone_entity);
			Motion& cutscene_drone_sad_motion		 = registry.get<Motion>(cutscene_drone_sad_entity);
			Motion& cutscene_drone_laughing_motion	 = registry.get<Motion>(cutscene_drone_laughing_entity);
			Motion& cutscene_minotaur_motion		 = registry.get<Motion>(cutscene_minotaur_entity);
			Motion& cutscene_minotaur_rtx_off_motion = registry.get<Motion>(cutscene_minotaur_rtx_off_entity);
			Motion& cutscene_drone_rtx_off_motion	 = registry.get<Motion>(cutscene_drone_rtx_off_entity);

			drawCutscene(window, &cutscene_selection);

			switch (cutscene_selection) {
			case 0:
				cutscene_minotaur_motion.scale = { 0,0 };
				cutscene_drone_motion.scale = { 0,0 };
				renderer.reinit(window_width_px, window_height_px, window);
				
				// Keep player from persistently moving post-cutscene
				motion.velocity = { 0,0 };
				pressed_keys.clear();

				state = ProgramState::RUNNING;

				cutscene_minotaur_motion.scale		   = { 0,0 };
				cutscene_drone_motion.scale			   = { 0,0 };
				cutscene_drone_laughing_motion.scale   = { 0,0 };
				cutscene_drone_sad_motion.scale		   = { 0,0 };
				cutscene_minotaur_rtx_off_motion.scale = { 0,0 };
				cutscene_drone_rtx_off_motion.scale	   = { 0,0 };

				break;
			case 2:
				cutscene_minotaur_motion.scale = { 0,0 };
				cutscene_drone_motion.position = { motion.position.x + window_width_px / 4, motion.position.y - window_height_px / 8 };
				cutscene_drone_motion.scale	   = { 400,400 };
				//cutscene_drone_motion.scale = { 0,0 };
				drawCutscene(window, &cutscene_selection);
				break;
			}
		}
		break;


		default:
			return 0;
		}
	}

	return EXIT_SUCCESS;
}
