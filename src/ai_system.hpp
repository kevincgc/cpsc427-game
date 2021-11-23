#pragma once

#include <vector>

#include "common.hpp"
#include "components.hpp"
#include "world_system.hpp"
#include "b_tree.hpp"

#include <entt.hpp>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class AISystem
{
private:
	static std::vector<vec2> trace(std::vector<std::vector<vec2>> parent, vec2 starting_map_pos, vec2 ending_map_pos);
	static std::vector<vec2> get_adj_nodes(vec2 root_node);
	static void generate_path(vec2 ending_map_pos, vec2 starting_map_pos);
	bool safe_zone_check(Motion &entity_motion, vec2 tile_world_coord);

	bool within_threshold(entt::entity entity, entt::entity other, float threshold);


public:
	void step();

};

extern std::vector<ChickAI> chick_ai;
