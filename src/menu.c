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

char **levels;
int levels_size;
static char *selected_level = NULL;

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

void initMainMenu(static GLFWwindow* win, int window_width_px, int window_height_px, float scale_x_in, float scale_y_in) {
    height = window_height_px;
    width = window_width_px;
    scale_x = scale_x_in;
    scale_y = scale_y_in;
    ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_DEFAULT);
    {
        struct nk_font_atlas* atlas;
        nk_glfw3_font_stash_begin(&glfw, &atlas);
        char *relPath = "/data/fonts/kenvector_future_thin.ttf";
        size_t size = sizeof(char) * (strlen(PROJECT_SOURCE_DIR) + strlen(relPath) + 1);
        char *path = (char *) malloc(size);
        strcpy_s(path, size, PROJECT_SOURCE_DIR);
        strcat_s(path, size, relPath);
        struct nk_font *future = nk_font_atlas_add_from_file(atlas, path, 26 * scale_x_in, 0);
        free(path);
        nk_glfw3_font_stash_end(&glfw);
        nk_style_set_font(ctx, &future->handle);
        //nk_style_load_all_cursors(ctx, atlas->cursors);
    }
    bg.r = 0.45f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
}


void drawMainMenu(static GLFWwindow* win, int *out)
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

void drawOptionsMenu(static GLFWwindow* win, int* out)
{
    nk_glfw3_new_frame(&glfw);

    if (nk_begin(ctx, "Options", nk_rect(475 * scale_x, 200 * scale_y, 250 * scale_x, 400 * scale_y),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
    {
        int changed = 0;
        static int property = 20;
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

void drawPauseMenu(static GLFWwindow* win, int* out)
{
    nk_glfw3_new_frame(&glfw);
    if (nk_begin(ctx, "Pause Menu", nk_rect(475 * scale_x, 200 * scale_y, 250 * scale_x, 400 * scale_y),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        enum { EASY, HARD };
        static int op = EASY;
        static int property = 20;
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

void drawGameOverMenu(static GLFWwindow* win, int* out)
{
    nk_glfw3_new_frame(&glfw);
    if (nk_begin(ctx, "You Died", nk_rect(475 * scale_x, 200 * scale_y, 250 * scale_x, 400 * scale_y),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        enum { EASY, HARD };
        static int op = EASY;
        static int property = 20;
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
