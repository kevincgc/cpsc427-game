#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
//#include "tiny_ecs.hpp"
#include <entt.hpp>

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
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("wall.png"),
		textures_path("enemy.png"), // <div>Icons made by <a href="https://www.freepik.com" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
		textures_path("drone.png"), // <div>Icons made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
		textures_path("Minotaur_sprite_sheet.png"), // https://elthen.itch.io/2d-pixel-art-minotaur-sprites
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = { // correspond to EFFECT_ASSET_ID
		shader_path("coloured"),
		shader_path("pebble"),
		shader_path("salmon"),
		shader_path("textured"),
		shader_path("water"),
		shader_path("minotaur"), // need to change to minotaur
		};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(int width, int height, GLFWwindow* window);

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

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(entt::entity entity, const mat3& projection);
	void drawTile(const vec2 map_coords, const MapTile map_tile, const mat3 &projection);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;
	float screen_scale;  // Screen to pixel coordinates scale factor (for apple
						 // retina display?)

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	entt::entity screen_state_entity = registry.create();
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
