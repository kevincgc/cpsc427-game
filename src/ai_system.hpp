#pragma once

#include <vector>

#include "common.hpp"
#include "components.hpp"

#include <entt.hpp>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class AISystem
{
private:
	static std::vector<vec2> AISystem::trace(std::vector<std::vector<vec2>> parent, vec2 starting_map_pos, vec2 ending_map_pos);
	static std::vector<vec2> get_adj_nodes(vec2 root_node);

public:
	void step(float elapsed_ms);

	static void AISystem::generate_path(vec2 ending_map_pos, vec2 starting_map_pos);
};