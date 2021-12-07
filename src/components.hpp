#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <entt.hpp>

enum class SoundEffects {
	PLAYER_DEAD = 0,
	PLAYER_ITEM,
	TADA,
	HORSE_SNORT,
	DRONE_WERE_IT_ONLY_SO_EASY,
	DRONE_STUPID_BOY,
	ITEM_BREAK_WALL,
	ITEM_TELEPORT,
	ITEM_SPEED_BOOST,
	CHICK_DIE,
	COUNT
};
const int sound_effect_count = (int)SoundEffects::COUNT;

struct SoundEffectRequest
{
	SoundEffects sound;
};

struct Player {
	// Used by hud to determine whether player has actually travelled
	vec2 prev_pos; 
};
struct Enemy {};
struct Friendly {};
struct Prey {int id;};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	vec2 prev_pos = { 0, 0 };
	float angle = 0.0f;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 10, 10 };
	float mass = 50.0f;
	float coeff_rest = 0.8f;
	bool can_collide = true;
	bool can_reflect = true;
	bool can_be_attacked = true;
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

enum ItemType {
	NONE = 0,
	WALL_BREAKER,
	TELEPORT,
	SPEED_BOOST,
	KEY,
};

// Level State
struct LoadedLevel
{
	std::vector<std::vector<MapTile>> map_tiles;
	vec2 start_position;
	int phase = 0;
	bool has_next = false;
};

// Global Game State
struct GameState
{
	std::string level_id = "main";
	std::string prev_level = "";
	LoadedLevel level;
	bool win_condition = false;

	bool cheat_finish = false;

	std::vector<SoundEffectRequest> sound_requests;
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
	bool basic_help = 0;
	bool picked_up_item = 0;
	bool item_info = 0;
	bool used_item = 0;
};
extern Help tips;

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};
struct AnimationTimer
{
	float counter_ms = 1000.f;
};

// A timer that will be associated to dying minotaur
struct DeathTimer {
	float counter_ms = 1000.f;
};

struct WallBreakerTimer
{
	float counter_ms = 20000.f;
};

struct TextTimer {
	float counter_ms = 5000.f;
};

struct SpeedBoostTimer {
	float counter_ms = 5000.f;
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


struct NormalMappingVertices
{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	vec3 tangent;

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
	std::string name;
	float duration_ms;
};

// Cutscene Elements
enum Cutscene_enum {
	BACKGROUND = 1,
	MINOTAUR = 2,
	DRONE = 3,
	DRONE_SAD = 4,
	DRONE_LAUGHING = 5,
	MINOTAUR_RTX_OFF = 6,
	DRONE_RTX_OFF = 7
};
struct Cutscene {};

// Background - so we can draw background elements first
struct Background {};

// HUD - so we can draw HUD elements in between world and cutscene
struct HUD {};

// Mesh data structure for storing vertex and index buffers
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
	WALL_NORMAL_MAP,
	FREESPACE,
	FREESPACE_NORMAL_MAP,
	SPIKE,
	DRONE,
	MINOTAUR,
	CHICK,
	WALL_BREAKER,
	EXTRA_LIFE,
	TELEPORT,
	SPEED_BOOST,
	CUTSCENE_MINOTAUR,
	CUTSCENE_DRONE,
	CUTSCENE_DRONE_SAD,
	CUTSCENE_DRONE_LAUGHING,
	CUTSCENE_MINOTAUR_RTX_OFF,
	CUTSCENE_DRONE_RTX_OFF,
	BACKGROUND_SPACE1,
	BACKGROUND_SPACE2,
	HUD_HEART,
	HUD_BACKGROUND,
	KEY,
	NO_HAMMER,
	NO_TELEPORT,
	NO_SPEEDBOOST,
	NO_KEY,
	BACKGROUND_MOON,
	BACKGROUND_SATELLITE,
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
	ENEMY = TEXT + 1,
	NORMAL_MAP = ENEMY + 1,
	ITEM = ENEMY + 1,
	TRAP = ITEM + 1,
	EFFECT_COUNT = NORMAL_MAP + 1
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
	DRONE = ENEMY + 1,
	TILE = DRONE + 1,
	ITEM = TILE + 1,
	TRAP = ITEM + 1,
	GEOMETRY_COUNT = TILE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	bool is_reflected = false;
};

