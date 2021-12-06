#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <nuklear.h>
#include <nuklear_glfw_gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ext/project_path.hpp"


struct nk_glfw glfw = { 0 };
struct nk_context* ctx;
struct nk_colorf bg;
float x = 0;
int width;
int height;
float scale_x;
float scale_y;

// Cutscene Variables
char* cutscene_chosen_text; // Helps determine which of the random dialogues to choose from

char **levels;
int levels_size;
 char *selected_level = NULL;

char **get_menu_options(int *num_options); // from CPP
void clear_menu_options(char **levels, int num_options); // from CPP
void set_level(char *level); // from cpp

void setBackground(float H) {
    int h = ((int)H) % 360;
    if (h >= 0.0 && h < 60.0)
    {
        bg.r = 0.5;
        bg.g = 0.25 + h / 240.;
        bg.b = 0.25;
    }
    else if (h >= 60.0 && h < 120.0)
    {
        bg.r = 0.5 - (h - 60.) / 240;
        bg.g = 0.5;
        bg.b = 0.25;
    }
    else if (h >= 120.0 && h < 180.0)
    {
        bg.r = 0.25;
        bg.g = 0.5;
        bg.b = 0.25 + (h - 120.) / 240.;
    }
    else if (h >= 180.0 && h < 240.0)
    {
        bg.r = 0.25;
        bg.g = 0.5 - (h - 180.) / 240;
        bg.b = 0.5;
    }
    else if (h >= 240.0 && h < 300.0)
    {
        bg.r = 0.25 + (h - 240.) / 240.;
        bg.g = 0.25;
        bg.b = 0.5;
    }
    else if (h >= 300.0 && h < 360.0)
    {
        bg.r = 0.5;
        bg.g = 0.25;
        bg.b = 0.5 - (h - 300.) / 240;
    }
}

void initMainMenu( GLFWwindow* win, int window_width_px, int window_height_px, float scale_x_in, float scale_y_in) {
    height = window_height_px;
    width = window_width_px;
    scale_x = scale_x_in;
    scale_y = scale_y_in;
    ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_DEFAULT);
    {
        struct nk_font_atlas* atlas;
        nk_glfw3_font_stash_begin(&glfw, &atlas);
        //char *relPath = "/data/fonts/kenvector_future_thin.ttf";
        char *relPath = "/data/fonts/Roboto-Black.ttf";
        size_t size = sizeof(char) * (strlen(PROJECT_SOURCE_DIR) + strlen(relPath) + 1);
        char *path = (char *) malloc(size);
        #if defined(WIN32) || defined(_WIN32) || defined(WIN32) || defined(NT)
        strcpy_s(path, size, PROJECT_SOURCE_DIR);
        strcat_s(path, size, relPath);
        #elif __APPLE__
        strcpy(path, PROJECT_SOURCE_DIR);
        strcat(path, relPath);
        #endif

        struct nk_font *future = nk_font_atlas_add_from_file(atlas, path, 26 * scale_x_in, 0);
        free(path);
        nk_glfw3_font_stash_end(&glfw);
        nk_style_set_font(ctx, &future->handle);
        //nk_style_load_all_cursors(ctx, atlas->cursors);
    }
    bg.r = 0.45f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
}


void drawMainMenu( GLFWwindow* win, int *out)
{
    nk_glfw3_new_frame(&glfw);

    if (nk_begin(ctx, "Main Menu", nk_rect(475 * scale_x, 200 * scale_y, 250 * scale_x, 400 * scale_y),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "START GAME")) {
            fprintf(stdout, "Starting Game\n");
            *out = 1;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "LOAD GAME")) {
            fprintf(stdout, "Load game selected\n");
            *out = 3;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "OPTIONS")) {
            fprintf(stdout, "Options Menu\n");
            *out = 2;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "EXIT GAME")) {
            fprintf(stdout, "Exiting Game\n");
            *out = -1;
        }
    }
    setBackground(x);
    x += 0.05;
    nk_end(ctx);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(bg.r, bg.g, bg.b, bg.a);
    /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
        * with blending, scissor, face culling, depth test and viewport and
        * defaults everything back into a default state.
        * Make sure to either a.) save and restore or b.) reset your own state after
        * rendering the UI. */
    nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    glfwSwapBuffers(win);
    //nk_glfw3_shutdown(&glfw);
    //glfwTerminate();
}

void initOptionsMenu() {
    levels = get_menu_options(&levels_size);
    selected_level = NULL;
}

void closeOptionsMenu() {
    clear_menu_options(levels, levels_size);
}

void drawOptionsMenu( GLFWwindow* win, int* out)
{
    nk_glfw3_new_frame(&glfw);

    if (nk_begin(ctx, "Options", nk_rect(475 * scale_x, 200 * scale_y, 250 * scale_x, 400 * scale_y),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
    {
        int changed = 0;
         int property = 20;
        nk_layout_row_dynamic(ctx, 30 * scale_x, 1);

        for (int i = 0; i < levels_size; i++) {
            if (nk_option_label(ctx, levels[i], selected_level == levels[i])) {
                if (selected_level != levels[i]) changed = 1;
                selected_level = levels[i];
            }
        }

        if (changed) {
            set_level(selected_level);
        }

        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "BACK")) {
            fprintf(stdout, "Main Menu\n");
            *out = -1;
        }
    }
    setBackground(x);
    x += 0.05;
    nk_end(ctx);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(bg.r, bg.g, bg.b, bg.a);
    nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    glfwSwapBuffers(win);
}

void drawPauseMenu( GLFWwindow* win, int* out)
{
    nk_glfw3_new_frame(&glfw);
    if (nk_begin(ctx, "Pause Menu", nk_rect(475 * scale_x, 200 * scale_y, 250 * scale_x, 400 * scale_y),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        enum { EASY, HARD };
         int op = EASY;
         int property = 20;
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "RESUME GAME")) {
            fprintf(stdout, "Starting Game\n");
            *out = 1;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "SAVE GAME")) {
            fprintf(stdout, "Save Game Selected\n");
            *out = 4;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "RESTART GAME")) {
            fprintf(stdout, "Restarting Game\n");
            *out = 2;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "MAIN MENU")) {
            fprintf(stdout, "Return To Main\n");
            *out = 3;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "EXIT GAME")) {
            fprintf(stdout, "Exiting Game\n");
            *out = -1;
        }
    }
    nk_end(ctx);
    nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    glfwSwapBuffers(win);
}

void drawGameOverMenu( GLFWwindow* win, int* out, int player_win, int leaderboard_size, char **leaderboard, const char *player_time, const char *level_info, int progression)
{
    nk_glfw3_new_frame(&glfw);
    int res;

    if (player_win) {
        res = nk_begin(ctx, "You escaped!", nk_rect(400 * scale_x, 75 * scale_y, 425 * scale_x, 600 * scale_y), NK_WINDOW_BORDER | NK_WINDOW_TITLE);
        nk_layout_row_dynamic(ctx, 30 * scale_x, 1);
        nk_label(ctx, level_info, NK_TEXT_ALIGN_LEFT);
        nk_label(ctx, player_time, NK_TEXT_ALIGN_LEFT);

        nk_label(ctx, "LEADERBOARD", NK_TEXT_ALIGN_CENTERED);
        for (int i = 0; i < leaderboard_size; i++) {
            nk_label(ctx, leaderboard[i], NK_TEXT_ALIGN_LEFT);
        }
    } else {
        res = nk_begin(ctx, "You died.", nk_rect(475 * scale_x, 200 * scale_y, 250 * scale_x, 400 * scale_y), NK_WINDOW_BORDER | NK_WINDOW_TITLE);
    }

    if (res)
    {
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        int res = 0;
        switch (progression) {
            case 0:
                res = nk_button_label(ctx, "TRY AGAIN");
                break;

            case 1:
                res = nk_button_label(ctx, "RESTART LEVEL");
                break;

            case 2:
                res = nk_button_label(ctx, "NEXT LEVEL");
                break;

            case 3:
                res = nk_button_label(ctx, "NEXT PHASE");
                break;
        }

        if (res) {
            fprintf(stdout, "Restarting Game\n");
            *out = 1;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "MAIN MENU")) {
            fprintf(stdout, "Return To Main\n");
            *out = 2;
        }
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "EXIT GAME")) {
            fprintf(stdout, "Exiting Game\n");
            *out = -1;
        }
    }
    glClear(GL_COLOR_BUFFER_BIT);
    nk_end(ctx);
    nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    glfwSwapBuffers(win);
}


// ************* CUTSCENES *************
// out 0 = resume game
// out 1-9 = cutscene_1 (game start)
// out 10-19 = cutscene_2 (reached the exit)
// out 100-199 = cutscene_death
// out 200-299 = cutscene_tutorial

void drawCutscene(GLFWwindow* win, int* out)
{
	nk_glfw3_new_frame(&glfw);

	// Set max panels so you can only click the next button a limited number of times
	int max_cutscene_selection;
	int min_cutscene_selection;

	// Cutscene Window Dimensions
	float cut_x = width  * 33 / 100;
	float cut_y = height * 69 / 100;
	float cut_w = width  * 65 / 100;
	float cut_h = height * 28 / 100;

	// Make an outer window that's just the background so the dialogue window won't be right at the edge
	if (nk_begin(ctx, "Cutscene_outer", nk_rect(cut_x, cut_y, cut_w, cut_h), NK_WINDOW_NO_SCROLLBAR)) {}
	nk_end(ctx);

	// Dialogue

	// IMPORTANT: Every dialogue card needs to have three nk_labels for spacing purposes.

	if (nk_begin(ctx, "Cutscene", nk_rect(cut_x+10, cut_y+10, cut_w-10, cut_h-10), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 50 * scale_x, 1);

		// First Spawn
		if (*out == 1) {
			min_cutscene_selection = 1;
			max_cutscene_selection = 3;
			nk_label(ctx, "Minotaur: I have suffered enough for the crimes of my father. If only he  ", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "had sacrificed the bull as Poseidon wished, I would not have been ", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "transformed into this beast, only to be locked away.", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 2) {
			min_cutscene_selection = 1;
			max_cutscene_selection = 3;
			nk_label(ctx, "This is an impressive maze, Daedalus, but my hooves grow weary of the", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "endless wandering...",		   NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 3) {
			min_cutscene_selection = 1;
			max_cutscene_selection = 3;
			nk_label(ctx, "I will show you just how quickly I can make it out.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "(by pressing [awsd/arrow keys] or [clicking] an empty tile to move", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "and pressing [space] to swing the axe)", NK_TEXT_ALIGN_LEFT);
		}

		// ********** Reached exit **********

		// Variation One: First exit
		if (*out == 10) {
			// Reached exit - speaker: drone
			min_cutscene_selection = 10;
			max_cutscene_selection = 12;
			nk_label(ctx, "Daedalus: Oh Son of Minos, suely you held me to greater esteem.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Your revenge upon your father will have to wait...", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 11) {
			min_cutscene_selection = 10;
			max_cutscene_selection = 12;
			nk_label(ctx, "For I am the greatest artificer there was! During your feeble attempt", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "to escape my labyrinth, I have constructed yet another.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 12) {
			min_cutscene_selection = 10;
			max_cutscene_selection = 12;
			nk_label(ctx, "And this one will not be so simple.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		// Variation Two: Second exit
		// Variation Three: Final End
		else if (*out == 15) {
			// Reached exit - speaker: drone
			min_cutscene_selection = 15;
			max_cutscene_selection = 16;
			nk_label(ctx, "Daedalus: What?! I barely finished constructing this phase!", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "I better get to work on the next one...", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 16) {
			min_cutscene_selection = 15;
			max_cutscene_selection = 16;
			nk_label(ctx, "But I don't think I can finish the background decorations in time!", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Why don't you take it a little slower, Son of Minos,", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "there's so much to see.", NK_TEXT_ALIGN_LEFT);
		}

		// ********** Death Dialogue **********
		else if (*out == 101) {
			// First death - speaker: minotaur
			min_cutscene_selection = *out, max_cutscene_selection = *out;
			nk_label(ctx, "Minotaur: Yet I live?",	  NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "How can this be?", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 102) {
			// Second death - speaker: drone
			min_cutscene_selection = *out, max_cutscene_selection = *out;
			nk_label(ctx, "Daedalus: Oh Son of Minos, were it only so easy...",  NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Poseidon damned you to eternal suffering,", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "and so damned you shall be.",			   NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 103) {
			// Third death - speaker: minotaur
			min_cutscene_selection = *out, max_cutscene_selection = *out;
			nk_label(ctx, "Minotaur: I wonder what your son Icarus would think,", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "were he alive to see you in such state, Daedalus.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 104) {
			// Fourth death - speaker: drone
			min_cutscene_selection = *out, max_cutscene_selection = *out;
			nk_label(ctx, "Daedalus: Oh my son Icarus, the sun should never have", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "melted the wax from your wings. My inventions were", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "inadequate, but you, Son of Minos, will not find them so.", NK_TEXT_ALIGN_LEFT);
		}


		else if (105 <= *out && *out <= 199) {
			// For now we will set anything between 105 to 199 as one of five randomly selected 'voicelines' of the minotaur
			min_cutscene_selection = *out, max_cutscene_selection = *out;

			if (!cutscene_chosen_text) {
				int r = 105 + rand() % 5; // returns 105 + [0..4]
				switch (r) {
				case 105: cutscene_chosen_text = "A minor flesh wound";		  break;
				case 106: cutscene_chosen_text = "A careless mistake";        break;
				case 107: cutscene_chosen_text = "A painful death, that one"; break;
				case 108: cutscene_chosen_text = "Only a matter of time";     break;
				case 109: cutscene_chosen_text = "A rare taste of failure";   break;
				}
			}
			nk_label(ctx, cutscene_chosen_text, NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}

		// ********** Tutorial Dialogue **********
		// Tutorial start
		else if (*out == 200) {
			min_cutscene_selection = 200, max_cutscene_selection = 202;
			nk_label(ctx, "I will suffer imprisonment no longer. Time to stretch these legs", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "(by pressing [w/a/s/d] or clicking on an empty tile).", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 201) {
			min_cutscene_selection = 200, max_cutscene_selection = 202;
			nk_label(ctx, "What's this, a gifted hammer?", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "If I pick it up, I can use it (by pressing [1]) on a wall (by clicking it)...", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 202) {
			min_cutscene_selection = 200, max_cutscene_selection = 202;
			nk_label(ctx, "Looks fragile, I think it only has 20 seconds before I can use it.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "And it looks like I can only use it once unless if I pick up another.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "I don't think it's strong enough to break the outer walls.", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 203) {
			min_cutscene_selection = 200, max_cutscene_selection = 203;
			nk_label(ctx, "This hammer is fragile", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "It looks like it'll break after one use.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		// Note enemy movement and speed boost
		else if (*out == 205) {
			min_cutscene_selection = 205, max_cutscene_selection = 206;
			nk_label(ctx, "Interesting. The enemies only move when I do.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Wait, did I just see a chick? I suppose eating them may grant", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "me a short burst of speed. If they tire, they might stop moving...", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 206) {
			min_cutscene_selection = 205, max_cutscene_selection = 206;
			nk_label(ctx, "Another item over there. Looks like it'll speed me up.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "I wonder how long it'll last for after activating it", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "(by pressing [3]). Maybe 5 seconds?", NK_TEXT_ALIGN_LEFT);
		}
		// Note floor disappearing
		else if (*out == 210) {
			min_cutscene_selection = 210, max_cutscene_selection = 210;
			nk_label(ctx, "The void! It disappeared!", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "I'm standing on bare floor.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Is this all just an illusion?", NK_TEXT_ALIGN_LEFT);
		}
		// Hold it right there: Speaker: Daedalus
		else if (*out == 215) {
			min_cutscene_selection = 215, max_cutscene_selection = 215;
			nk_label(ctx, "Hold it right there.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "You're now in stasis, beast.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Let me get a closer look at you.", NK_TEXT_ALIGN_LEFT);
		}
		// Daedalus speech: Speaker: Daedalus
		else if (*out == 216) {
			min_cutscene_selection = 216, max_cutscene_selection = 217;
			nk_label(ctx, "Oh Son of Minos, I've been careless to leave my toys lying around", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "But never have I crafted one that could break walls.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "I'll find out who's helping you, but first...", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 217) {
			min_cutscene_selection = 216, max_cutscene_selection = 217;
			nk_label(ctx, "I'll test my drones. If you get too close, they'll chase you", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "If you destroy them, they'll deform as the souls are set free", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "They can still hit you, but you won't lose any life.", NK_TEXT_ALIGN_LEFT);
		}
		// Attack cutscene: Speaker: Minotaur
		else if (*out == 220) {
			min_cutscene_selection = 220, max_cutscene_selection = 221;
			nk_label(ctx, "I can defend myself (by pressing [spacebar])", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Looks like these drones aren't very smart.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "If they hit the wall, they'll probably stop moving.", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 221) {
			min_cutscene_selection = 220, max_cutscene_selection = 221;
			nk_label(ctx, "But if I know Daedalus, he's already made smarter drones.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Regardless, these will be easy to take care of.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		// Note teleporter: Speaker: Minotaur
		else if (*out == 225) {
			min_cutscene_selection = 225, max_cutscene_selection = 227;
			nk_label(ctx, "The exit! Finally I can escape this prison.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "And over there in the corner, a teleporter and a key?", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Daedalus probably puts invisible doors at the exits... ", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 226) {
			min_cutscene_selection = 225, max_cutscene_selection = 227;
			nk_label(ctx, "I'll need the key to escape. I can use the teleporter (by pressing [2]).", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "It looks unstable though - no wonder he threw them away.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "I don't think I can control where it teleports me...", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 227) {
			min_cutscene_selection = 225, max_cutscene_selection = 227;
			nk_label(ctx, "But I don't have a choice.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Let's hope I end up on the other side of this wall.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		// Note teleporter arrival: Speaker: Minotaur
		else if (*out == 230) {
			min_cutscene_selection = 230, max_cutscene_selection = 230;
			nk_label(ctx, "That was lucky. Good think there's a key here.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Certain mazes will need a certain number of kesy to escape.", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Now, time to get out of this labyrinth.", NK_TEXT_ALIGN_LEFT);
		}
		// Note teleporter arrival: Speaker: Minotaur
		else if (*out == 300) {
		min_cutscene_selection = 300, max_cutscene_selection = 300;
		nk_label(ctx, "I still need to find all the keys before I can leave.", NK_TEXT_ALIGN_LEFT);
		nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}

		// Row for Buttons
		// Make sure dialogue has three nk_labels so row doesn't float upwards.
		nk_layout_row_dynamic(ctx, 45 * scale_x, 3);
		if (nk_button_label(ctx, "Prev")) {
			if (*out > min_cutscene_selection) { *out -= 1; }
		}
		if (nk_button_label(ctx, "Skip")) {
			*out = 0;
			cutscene_chosen_text = NULL;
		}
		if (nk_button_label(ctx, "Next")) {
			if (*out + 1 > max_cutscene_selection) { *out = 0; }
			else { *out += 1; }
			cutscene_chosen_text = NULL;
		}
	}
	nk_end(ctx);

	// Because NK_WINDOW_NO_INPUT does not work, this is a hack
	// to make sure the buttons and dialogue stays on top if
	// someone clicks on the dialogue window
	if (nk_window_is_active(ctx, "Cutscene_outer")) {
		nk_window_set_focus(ctx, "Cutscene");
		nk_window_set_focus(ctx, "Prev Button/Window");
		nk_window_set_focus(ctx, "Skip Button/Window");
		nk_window_set_focus(ctx, "Next Button/Window");
	}

	nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
	glfwSwapBuffers(win);
}
