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

struct nk_glfw glfw = { 0 };
struct nk_context* ctx;
struct nk_colorf bg;
float x = 0;

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

void initMainMenu(static GLFWwindow* win, int window_width_px, int window_height_px) {
    ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_DEFAULT);
    {
        struct nk_font_atlas* atlas;
        nk_glfw3_font_stash_begin(&glfw, &atlas);
        struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../data/fonts/kenvector_future_thin.ttf", 26, 0);
        nk_glfw3_font_stash_end(&glfw);
        nk_style_set_font(ctx, &future->handle);
        //nk_style_load_all_cursors(ctx, atlas->cursors);
    }
    bg.r = 0.45f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
}


void drawMainMenu(static GLFWwindow* win, int *out)
{
    glfwPollEvents();
    nk_glfw3_new_frame(&glfw);

    if (nk_begin(ctx, "Main Menu", nk_rect(475, 200, 250, 400),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
    {
        enum { EASY, HARD };
        static int op = EASY;
        static int property = 20;
        nk_layout_row_dynamic(ctx, 50, 1);
        if (nk_button_label(ctx, "START GAME")) {
            fprintf(stdout, "Starting Game\n");
            *out = 1;
        }
            
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
        nk_layout_row_dynamic(ctx, 50, 1);
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

