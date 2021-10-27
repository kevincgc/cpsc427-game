// internal
#include "ai_system.hpp"
#include <vector>
#include <iostream>
#include <algorithm> // for reversing a vector

// Pathfinding Datastructure
// Hold a vector of coordinates. For example: If starting at [0,1]
// [[0,1], [1,1], [2,1], [3,1]]
// corresponds to traveling 3 tiles to the right.
std::vector<vec2> path;
extern bool do_pathfinding_movement = false;
float DIST_THRESHOLD = 200.f;

// Determine the distance threshold between the enemy and player
bool within_threshold(entt::entity entity, entt::entity other)
{
	Motion& entity_motion = registry.get<Motion>(entity);
	Motion& other_motion = registry.get<Motion>(other);
	
	vec2 direction_vector = entity_motion.position - other_motion.position;
	float distance = sqrt(dot(direction_vector, direction_vector));
	return distance <= DIST_THRESHOLD;
}



void AISystem::step(float elapsed_ms)
{
	entt::entity player = registry.view<Player>().begin()[0];
	Motion& motion = registry.get<Motion>(player);

	for (entt::entity entity : registry.view<Motion>())
	{
		Motion& entity_motion = registry.get<Motion>(entity);
		vec2 entity_velocity = entity_motion.velocity;
		float step_seconds = 1.0f * (elapsed_ms / 4000.f);
		if ( entity != player)
		{
			if (within_threshold(player, entity))
			{
				vec2 direction_vector = motion.position - entity_motion.position;
				entity_motion.velocity = direction_vector;
				//entity_motion.position += entity_motion.velocity * step_seconds;
			}
			else {
				entity_motion.velocity = entity_velocity;
				//entity_motion.position += entity_velocity * step_seconds * 4.f;
			}
		}
		
	}

	// ========= Feature: Pathfinding =========
	if (do_generate_path) {
		generate_path(starting_map_pos, ending_map_pos);
	}

	if (do_pathfinding_movement) {
		std::cout << "Doing pathfinding movement..." << std::endl;

		if (path.size() > 0) {
			// Get the player's current map position
			vec2 player_map_pos = { (int)(motion.position.x / map_scale), (int)(motion.position.y / map_scale) };

			// Get the first destination in the path
			vec2 target_node = path.front();
			std::cout << "Target node [" << target_node.x << ", " << target_node.y << "]" << std::endl;

			// Must convert node map coords (ex. [0,1]) to world coords (ex. [400,600])
			// Otherwise player will stop at node edge, not node center.
			// Adding extra map_scale / 2 to get coords for center of map tile.
			vec2 target_node_coord = { map_scale * target_node.x + map_scale/2, map_scale * target_node.y + map_scale / 2 };

			// If the player has not eached the target node...
			if (target_node_coord.x != (int)motion.position.x || target_node_coord.y != (int)motion.position.y) {
				std::cout << "Determining direction to move" << std::endl;
				std::cout << "Target coords: [" << target_node_coord.x << ", " << target_node_coord.y << "]" << std::endl;
				std::cout << "Player coods:  [" << (int)motion.position.x << ", " << (int)motion.position.y << "]" << std::endl;

				//Move
				float temp_vel = 100;
				if (target_node_coord.x > (int)motion.position.x) { motion.velocity.x = temp_vel; }
				else if (target_node_coord.x < (int)motion.position.x) { motion.velocity.x = -1*temp_vel; }
				if (target_node_coord.y > (int)motion.position.y) { motion.velocity.y = temp_vel; }
				else if (target_node_coord.y < (int)motion.position.y) { motion.velocity.y = -1*temp_vel; }

			}


			// If the player has not eached the target node...
			//if (target_node != player_map_pos) {
			//	// Determine the direction to move
			//	std::cout << "Determining direction to move" << std::endl;

			//	// Move right
			//	float temp_vel = 100;
			//	if (target_node.x > player_map_pos.x) {
			//		motion.velocity.x = temp_vel;
			//	}
			//	// Move left
			//	else if (target_node.x < player_map_pos.x) {
			//		motion.velocity.x = -1 * temp_vel;

			//	}
			//	// Move down
			//	else if (target_node.y > player_map_pos.y) {
			//		motion.velocity.y = temp_vel;
			//	}
			//	// Move up
			//	else if (target_node.y < player_map_pos.y) {
			//		motion.velocity.y = -1 * temp_vel;
			//	}

			//}

			// The player has reached the target node...
			else {
				std::cout << "Reached node" << std::endl;
				// We've only reached the node edge, so we need to continue travelling until we've reached the node center
				// Get node center coord


				// Get player coord

				// Stop movement
				motion.velocity.x = 0;
				motion.velocity.y = 0;

				// Remove the first node in path (we've reached it)
				path.erase(path.begin());
			}

		}

		else {
			std::cout << "Path size is 0" << std::endl;
			do_pathfinding_movement = false;
		}

		

	}

}


// ========= Feature: Pathfinding =========
void AISystem::generate_path(vec2 starting_map_pos, vec2 ending_map_pos) {
	std::vector<std::vector<vec2>> parent;
	std::vector<vec2> visited;
	std::vector<vec2> queue;
	std::set<vec2> set_queue;
	bool generate_path = true;

	// Clear the previous path so we can construct a new one.
	path.clear();

	// Push the starting pos into the path
	queue.push_back(starting_map_pos);

	while (queue.size() > 0 && do_generate_path == true) {
		// Get the first node in the queue
		// Important: node is a vec2 which means a node that looks like [0,1] means
		// that the x (the column) is 0 and the y (the row) is 1.

		vec2 n = queue.front();
		queue.erase(queue.begin());
		std::cout << "========================================" << std::endl;
		std::cout << "Looking at node [ " << n.x << ", " << n.y << "]" << std::endl;

		// Reached end
		if (n == ending_map_pos) {
			std::cout << "reached end" << std::endl;
			do_generate_path = false;

			path = trace(parent, starting_map_pos, ending_map_pos);

			// Debug: Print path
			std::cout << " Path is: " << std::endl;
			for (auto node : path) {
				std::cout << "    [" << node.x << ", " << node.y << "]" << std::endl;
			}

			do_pathfinding_movement = true;
		}
		else {
			// Get the list of traversable adjacent nodes
			std::vector<vec2> adjacent_nodes = get_adj_nodes(n);

			// Debug: Print list of adjacent nodes
			std::cout << "Adjacent nodes are..." << std::endl;
			for (auto adj_node : adjacent_nodes) {
				std::cout << "    [" << adj_node.x << ", " << adj_node.y << "]" << std::endl;
			}

			// if node is not in queue and not in visited.
			if (std::find(queue.begin(), queue.end(), n) == queue.end() && std::find(visited.begin(), visited.end(), n) == visited.end()) {
				std::cout << "Node is not in queue and not in visited" << std::endl;
				for (auto adj_node : adjacent_nodes) {

					// Debug
					std::cout << "Looking at node... [" << adj_node.x << ", " << adj_node.y << "]" << std::endl;

					// Set the parent node of the adjacent node to this current node
					parent.push_back({ adj_node, n });
					std::cout << "pushed adj node: [" << adj_node.x << ", " << adj_node.y << "] to parent map" << std::endl;
					std::cout << "parent map now looks like this:" << std::endl;
					for (auto pair : parent) {
						std::cout << "    [" << pair[0].x << ", " << pair[0].y << "] : [" << pair[1].x << ", " << pair[1].y << "]" << std::endl;
					}

					// Add the adjacent node to the queue to search later
					queue.push_back(adj_node);
					std::cout << "pushed adj node: [" << adj_node.x << ", " << adj_node.y << "] to queue to search later" << std::endl;
					std::cout << "queue now looks like this:" << std::endl;
					for (auto node : queue) {
						std::cout << "    [" << node.x << ", " << node.y << "]" << std::endl;
					}
				}

				// Add the current node to the visited list
				
				visited.push_back(n);
				std::cout << "visited now looks like this " << std::endl;
				for (auto node : visited) {
					std::cout << "    [" << node.x << ", " << node.y << "]" << std::endl;
				}
			}
			else {
				std::cout << "Already visited or is in queue" << std::endl;
			}
			
		}
	}

}

std::vector<vec2> AISystem::trace(std::vector<std::vector<vec2>> parent, vec2 starting_map_pos, vec2 ending_map_pos) {
	std::vector<vec2> result;

	vec2 curr_node;
	vec2 parent_node;
	result = { ending_map_pos };
	while (result.back() != starting_map_pos) {
		curr_node = result.back();
		std::cout << "Current node is: [" << curr_node.x << "," << curr_node.y << "]" << std::endl;
		for (auto pair : parent) {
			if (pair[0] == curr_node) {
				parent_node = pair[1];
				std::cout << "Found parent node: [" << parent_node.x << "," << parent_node.y << "]" << std::endl;
				break;
			}
		}
		result.push_back(parent_node);
		std::cout << "Pushed parent node [" << parent_node.x << "," << parent_node.y << "] to results" << std::endl;

		std::cout << "results look like this: " << std::endl;
		for (auto node : result) {
			std::cout << "    [" << node.x << "," << node.y << "]" << std::endl;
		}
	}
	std::reverse(result.begin(), result.end());

	return result;
}

std::vector<vec2> AISystem::get_adj_nodes(vec2 root_node) {
	// Important: map_tiles[0][1] refers to row 0, column 1
	
	std::vector<vec2> adj_nodes = {};

	// Assuming map is square
	size_t max_size = game_state.map_tiles.size();

	// For now, assuming map_tile == 0 is the only traversable tile
	// If adjacent node is within bounds and map_tile == 0, add to adjacency list.
	// Check right
	if (root_node.x + 1 <= max_size) {
		if (game_state.map_tiles[root_node.y][root_node.x + 1] == FREE_SPACE) {
			adj_nodes.push_back({ root_node.x + 1, root_node.y });
		}
	}
	// Check down
	if (root_node.y + 1 <= max_size) {
		if (game_state.map_tiles[root_node.y + 1][root_node.x] == FREE_SPACE) {
			adj_nodes.push_back({ root_node.x,root_node.y + 1 });
		}
	}
	// Check left
	if (root_node.x - 1 >= 0) {
		if (game_state.map_tiles[root_node.y][root_node.x - 1] == FREE_SPACE) {
			adj_nodes.push_back({ root_node.x - 1, root_node.y });
		}
	}
	// Check up
	if (root_node.y - 1 >= 0) {
		if (game_state.map_tiles[root_node.y - 1][root_node.x] == FREE_SPACE) {
			adj_nodes.push_back({ root_node.x, root_node.y - 1 });
		}
	}


	return adj_nodes;
}