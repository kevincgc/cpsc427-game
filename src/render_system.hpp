#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include <entt.hpp>
#include <ft2build.h>

extern entt::registry registry;

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {

	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::MINOTAUR, mesh_path("minotaur.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::ENEMY, mesh_path("enemy.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::DRONE, mesh_path("drone.obj")),
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count > texture_paths = {
		textures_path("wall.png"), //<div>Icons made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
		textures_path("wall_normal_map.png"),
		textures_path("freespace.png"),
		textures_path("freespace_normal_map.png"),
		textures_path("enemy.png"), // <div>Icons made by <a href="https://www.freepik.com" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
		textures_path("drone.png"), // <div>Icons made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
		textures_path("Minotaur_sprite_sheet.png"), // https://elthen.itch.io/2d-pixel-art-minotaur-sprites
		textures_path("prey.png"),
		textures_path("hammer.png"), // <div>Icons made by <a href="https://www.freepik.com" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
		textures_path("key.png"),
		textures_path("teleport.png"),  // <div>Icons made by <a href="https://www.flaticon.com/authors/berkahicon" title="berkahicon">berkahicon</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
		textures_path("speedboost.png"),
		textures_path("cutscene_minotaur.png"),
		textures_path("cutscene_drone.png"),
		textures_path("cutscene_drone_sad.png"),
		textures_path("cutscene_drone_laughing.png"),
		textures_path("cutscene_minotaur_rtx_off.png"),
		textures_path("cutscene_drone_rtx_off.png"),
		textures_path("background_space1.png"), // https://wallpaperaccess.com/4k-space#google_vignette
		textures_path("background_space2.png"), // https://pngtree.com/free-png-vectors/white-stars
		textures_path("heart.png"), //
		textures_path("hud_background.png"),
		textures_path("key.png"), // https://pngtree.com/
		textures_path("no_hammer.png"),
		textures_path("no_teleport.png"),
		textures_path("no_speedboost.png"),
		textures_path("no_key.png"),
		textures_path("background_moon.png"), // https://toppng.com/moon-PNG-free-PNG-Images_25446
		textures_path("background_satellite.png") //https://www.pngaaa.com/detail/1865660
	};



	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
// enum class EFFECT_ASSET_ID {
// 	COLOURED = 0,
// 	PEBBLE = COLOURED + 1,
// 	// SALMON = PEBBLE + 1, // remove salmon
// 	TEXTURED = PEBBLE + 1,
// 	WATER = TEXTURED + 1,
// 	MINOTAUR = WATER + 1,
// 	TEXT = MINOTAUR + 1,
// 	ENEMY = TEXT + 1,
// 	ITEM = ENEMY + 1,
// 	TRAP = ITEM + 1,
// 	EFFECT_COUNT = ENEMY + 1
// };
	const std::array<std::string, effect_count> effect_paths = { // correspond to EFFECT_ASSET_ID
		shader_path("coloured"),
		shader_path("pebble"),
		shader_path("textured"),
		shader_path("water"),
		shader_path("minotaur"),
		shader_path("text"),
		shader_path("enemy"),
		shader_path("normal_map")
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(int width, int height, GLFWwindow* window);

	bool reinit(int width, int height, GLFWwindow* window_arg, bool is_init = 0);

	void reinitSetBuffer(int width, int height, GLFWwindow* window_arg);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader
	bool initScreenTexture();
	bool reinitScreenTexture();

	// generate help text(automatic explaination)
	void initText();


	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	FT_Library ft;

	FT_Face face;

	mat3 createProjectionMatrix();

	mat3 createProjectionMatrixforText();


private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(entt::entity entity, const mat3& projection);
	void drawTile(const vec2 map_coords, const MapTile map_tile, const mat3& projection, vec2 screen);
	void drawWall(const vec2 map_coords, const mat3& projection, vec2 screen);
	void drawFreeSpace(const vec2 map_coords, const mat3& projection, vec2 screen);
	void drawText(const std::string text, vec2 position, vec2 scale, const mat3& projection, vec3 text_colour);
	void drawToScreen();


	// Window handle
	GLFWwindow* window;
	float screen_scale;  // Screen to pixel coordinates scale factor (for apple
						 // retina display?)

	float pixel_size;
	std::string renderedText_1;
	std::string renderedText_2;

	// Screen texture handles
	GLuint frame_buffer;

	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;
	entt::entity screen_state_entity = registry.create();



};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);



