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

// Andrew's Variables
float cutscene_alpha = 255;
char* cutscene_chosen_text;

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

void drawGameOverMenu( GLFWwindow* win, int* out)
{
    nk_glfw3_new_frame(&glfw);
    if (nk_begin(ctx, "You Died", nk_rect(475 * scale_x, 200 * scale_y, 250 * scale_x, 400 * scale_y),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        enum { EASY, HARD };
         int op = EASY;
         int property = 20;
        nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
        if (nk_button_label(ctx, "TRY AGAIN")) {
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
// out 10-19 - cutscene_2 (reaached the exit)
// out 100-199 = cutscene_death

void drawCutscene(GLFWwindow* win, int* out)
{
	nk_glfw3_new_frame(&glfw);

	// Set max panels so you can only click the next button a limited number of times
	int max_cutscene_selection;
	int min_cutscene_selection;

	// Cutscene Window Dimensions
	float cut_x = width / 3;
	float cut_y = height * 3 / 4 * scale_y - 50;
	float cut_w = width  * 2 / 3 * scale_x - 50;
	float cut_h = height * 1 / 4 * scale_y + 10;

	// Make an outer window that's just the background so the dialogue window won't be right at the edge
	if (nk_begin(ctx, "Cutscene_outer", nk_rect(cut_x, cut_y, cut_w, cut_h), NK_WINDOW_NO_SCROLLBAR)) {

	}
	nk_end(ctx);

	// Dialogue
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
			nk_label(ctx, "endless wandering. My father is due what he is owed.",		   NK_TEXT_ALIGN_LEFT);
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
			nk_label(ctx, "Daedalus: Oh Son of Minos,",							NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "Did you think Poseidon would appoint any artificer", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "To be your warden?",									NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 11) {
			min_cutscene_selection = 10;
			max_cutscene_selection = 12;
			nk_label(ctx, "Your revenge upon your father will have to wait, for", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "during your feable attempt to escape my labyrinth", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "I have constructed yet another.",					   NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 12) {
			min_cutscene_selection = 10;
			max_cutscene_selection = 12;
			nk_label(ctx, "And this one will not be so simple.",			   NK_TEXT_ALIGN_LEFT);
		}
		// Variation Two: Second exit
		// Variation Three: Final End
		else if (*out == 15) {
			// Reached exit - speaker: drone
			min_cutscene_selection = 15;
			max_cutscene_selection = 17;
			nk_label(ctx, "Daedalus: NoOoOooOOo!", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 16) {
			min_cutscene_selection = 15;
			max_cutscene_selection = 17;
			nk_label(ctx, "I haven't finished my next labryinth!", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 17) {
			min_cutscene_selection = 15;
			max_cutscene_selection = 17;
			nk_label(ctx, "Enjoy your victory, Son of Minos, for it will be short lived.", NK_TEXT_ALIGN_LEFT);
		}

		// ********** Death Dialogue **********
		else if (*out == 101) { 
			// First death - speaker: minotaur
			min_cutscene_selection = *out, max_cutscene_selection = *out;
			nk_label(ctx, "Minotaur: Yet I live?",	  NK_TEXT_ALIGN_LEFT); 
			nk_label(ctx, "How can this be?", NK_TEXT_ALIGN_LEFT); 
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
			nk_label(ctx, "Minotaur:  I wonder what your son Icarus would think,", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "were he alive to see you driven mad, Daedalus", NK_TEXT_ALIGN_LEFT);
		}
		else if (*out == 104) {
			// Fourth death - speaker: drone
			min_cutscene_selection = *out, max_cutscene_selection = *out;
			nk_label(ctx, "Daedalus: Oh my son Icarus, the sun should never have", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "melted the wax from your wings. My inventions were", NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, "inadequate, but you, Son of Minos, will not find them so.", NK_TEXT_ALIGN_LEFT);
		}


		else if (105 <= *out) {
			// For now we will set anything above ____ as one of five randomly selected 'voicelines' of the minotaur
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
		}
	}
	nk_end(ctx);

	// PREV
	float prev_x = width  * 8 / 14  * scale_x;
	float prev_y = height * 13 / 14 * scale_y - 40;
	float prev_w = width  * 2 / 14  * scale_x - 50;
	float prev_h = height * 1 / 14  * scale_y;
	if (nk_begin(ctx, "Prev Button/Window", nk_rect(prev_x, prev_y, prev_w, prev_h), NK_WINDOW_NO_SCROLLBAR))
	{
		
		nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
		if (nk_button_label(ctx, "Prev")) {
			if (*out > min_cutscene_selection) { *out -= 1; }
		}
	}
	nk_end(ctx);

	// NEXT
	float next_x = width  * 12 / 14 * scale_x;
	float next_y = height * 13 / 14 * scale_y - 40;
	float next_w = width  * 2 / 14  * scale_x - 50;
	float next_h = height * 1 / 14  * scale_y;
	if (nk_begin(ctx, "Next Button/Window", nk_rect(next_x, next_y, next_w, next_h), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
		if (nk_button_label(ctx, "Next")) {
			if (*out + 1 > max_cutscene_selection) { *out =  0; }
			else								   { *out += 1; }
			cutscene_chosen_text = NULL;
		}
	}
	nk_end(ctx);

	// SKIP
	float skip_x = width  * 10 / 14 * scale_x;
	float skip_y = height * 13 / 14 * scale_y - 40;
	float skip_w = width  * 2 / 14  * scale_x - 50;
	float skip_h = height * 1 / 14  * scale_y;
	if (nk_begin(ctx, "Skip Button/Window", nk_rect(skip_x, skip_y, skip_w, skip_h), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 50 * scale_x, 1);
		if (nk_button_label(ctx, "Skip")) {
			*out = 0;
			cutscene_chosen_text = NULL;
		}
	}
	nk_end(ctx);

	// Because NK_WINDOW_NO_INPUT does not work, this is a hack
	// to make sure the buttons and dialogue stays on top
	if (nk_window_is_active(ctx, "Cutscene_outer")) { 
		nk_window_set_focus(ctx, "Cutscene");
		nk_window_set_focus(ctx, "Prev Button/Window");
		nk_window_set_focus(ctx, "Skip Button/Window");
		nk_window_set_focus(ctx, "Next Button/Window");
	}
	if (nk_window_is_active(ctx, "Cutscene")) {
		nk_window_set_focus(ctx, "Prev Button/Window");
		nk_window_set_focus(ctx, "Skip Button/Window");
		nk_window_set_focus(ctx, "Next Button/Window");
	}

	nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
	glfwSwapBuffers(win);
}