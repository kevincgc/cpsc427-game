#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <entt.hpp>

// Player component
struct Player
{

};

struct Enemy
{

};

struct Friendly
{

};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	float angle = 0.0f;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 10, 10 };
	float mass = 50.0f;
	float coeff_rest = 0.8f;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	entt::entity other; // the second object involved in the collision
	Collision(entt::entity& other) { this->other = other; };
};

// map tiles
enum MapTile {
	FREE_SPACE = 0,
	BREAKABLE_WALL,
	UNBREAKABLE_WALL,
	ENTRANCE,
	EXIT
};

// Level State
struct LoadedLevel
{
	std::vector<std::vector<MapTile>> map_tiles;
	vec2 start_position;
};

// Global Game State
struct GameState
{
	std::string level_id = "procedural1";
	LoadedLevel level;
};
extern GameState game_state;

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// Data structure for togglin help mode
struct Help {
	bool in_help_mode = 0;
};
extern Help tips;

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float counter_ms = 1000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

struct Colour
{
	vec3 colour;
};

struct Flash
{
	// flash the sprite
};

struct Attack
{
	// if the entity is in attack mode
};
struct EndGame {
	float counter_ms = 3000;
};
// New Components for project
struct Item
{
	int id = 0;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	// wall types. for now only one type
	// but if we wanted to use different sprites for junctions
	// we could do something like:
	// WALL_VERTICAL,
	// WALL_HORIZONTAL,
	// WALL_TOP_RIGHT,
	// WALL_TOP_LEFT,
	// WALL_BTM_LEFT,
	// WALL_BTM_RIGHT,
	// WALL_T_TOP,
	// WALL_T_LEFT,
	// WALL_T_BTM,
	// WALL_T_RIGHT,
	// WALL_CROSS,
	WALL = 0,
	FREESPACE,
	SPIKE,
	DRONE,
	MINOTAUR,
	TEXTURE_COUNT
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PEBBLE = COLOURED + 1,
	// SALMON = PEBBLE + 1, // remove salmon
	TEXTURED = PEBBLE + 1,
	WATER = TEXTURED + 1,
	MINOTAUR = WATER + 1,
	TEXT = MINOTAUR + 1,
	ENEMY = MINOTAUR + 1,
	ITEM = ENEMY + 1,
	TRAP = ITEM + 1,
	EFFECT_COUNT = TEXT + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SALMON = 0,
	SPRITE = SALMON + 1,
	PEBBLE = SPRITE + 1,
	DEBUG_LINE = PEBBLE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	MINOTAUR = SCREEN_TRIANGLE + 1,
	ENEMY = MINOTAUR + 1,
	ITEM = ENEMY + 1,
	TRAP = ITEM + 1,
	GEOMETRY_COUNT = MINOTAUR + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	bool is_reflected = false;
};

