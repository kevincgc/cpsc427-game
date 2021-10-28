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
bool in_safe_zone = true;
bool do_calculate_new_enemy_path = false;

bool ai_debug = false;

// Temp
float enemy_move_speed = 100.f;
float player_move_speed = 200.f;

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

	// For each entity
	for (entt::entity entity : registry.view<Motion>())
	{
		Motion& entity_motion = registry.get<Motion>(entity);
		vec2 entity_velocity = entity_motion.velocity;
		float step_seconds = 1.0f * (elapsed_ms / 4000.f);

		// For each enemy entity
		if ( entity != player)
		{
			if (within_threshold(player, entity))
			{
				vec2 direction_vector = motion.position - entity_motion.position;
				entity_motion.velocity = direction_vector;
				//entity_motion.position += entity_motion.velocity * step_seconds;
			}
			else {

				// ========= Feature: Pathfinding (AI maze navgiation) =========
			
				// If enemy is travelling diagonally (ex from chasing a player), randomly choose one or the other axis
				// This helps prevent getting stuck in corners because the AI looks at the diagonal node that it's
				// projected to travel to, see's there's no wall, and tries to travel to it.
				if (entity_motion.velocity.x != 0 && entity_motion.velocity.y != 0) {
					bool go_y = rand() % 2;
					if (go_y) {
						entity_motion.velocity.x = 0;
						entity_motion.velocity.y = enemy_move_speed;
					}
					else {
						entity_motion.velocity.x = enemy_move_speed;
						entity_motion.velocity.y = 0;
					}

				}

				vec2 next_pos_world = { entity_motion.position.x + (entity_motion.velocity.x),
								        entity_motion.position.y + (entity_motion.velocity.y) };

				float world_length_px = game_state.map_tiles.size() * map_scale;
				if (next_pos_world.x > 0 && next_pos_world.y > 0 && next_pos_world.x < world_length_px && next_pos_world.y < world_length_px) {
					vec2 next_pos_map = { (int)(next_pos_world.x / map_scale), (int)(next_pos_world.y / map_scale) };
					vec2 curr_pos_map = { (int)(entity_motion.position.x / map_scale), (int)(entity_motion.position.y / map_scale) };

					// Make sure the enemy won't get stuck in the corner

					// Get the curr tile's world coords
					vec2 tile_world_coord = { map_scale * curr_pos_map.x, map_scale * curr_pos_map.y };

					// Make sure the enemy's position is within a 'safe zone'
					//  _____________
					// |   |     |   |
					// |___|     |___|
					// |  safe zone  |
					// |___       ___|
					// |   |     |   |
					// |___|_____|___|
					// Each tile is [scale] pixels wide. ATM it's 150px.
					// Safe zone is between ~ 150/4 to 150*3/4

					if (ai_debug) {
						std::cout << "Safe zone x range: " << tile_world_coord.x + map_scale * .25 << " - " << tile_world_coord.x + map_scale * .75 << std::endl;
						std::cout << "Safe zone y range: " << tile_world_coord.y + map_scale * .25 << " - " << tile_world_coord.y + map_scale * .75 << std::endl;
						std::cout << "Enemy position: " << entity_motion.position.x << ", " << entity_motion.position.y << std::endl;
					}

					// If the enemy is moving vertically, make sure they're in the vertical safe zone
					if (entity_motion.velocity.y != 0) {
						if (entity_motion.position.x <= tile_world_coord.x + map_scale * .25) { 
							entity_motion.velocity.x = enemy_move_speed; 
							in_safe_zone = false;
						}
						else if (entity_motion.position.x >= tile_world_coord.x + map_scale * .75) {
							entity_motion.velocity.x = -1 * enemy_move_speed; 
							in_safe_zone = false;
						}
						else {
							in_safe_zone = true;
						}

					}
					// If the enemy is moving horizontally, make sure they're in the horizontal safe zone
					else if (entity_motion.velocity.x != 0) {
						if (entity_motion.position.y <= tile_world_coord.y + map_scale * .25) { 
							entity_motion.velocity.y = enemy_move_speed; 
							in_safe_zone = false;
						}
						else if (entity_motion.position.y >= tile_world_coord.y + map_scale * .75) { 
							entity_motion.velocity.y = -1 * enemy_move_speed; 
							in_safe_zone = false;
						}
						else {
							in_safe_zone = true;
						}
					}

					if (in_safe_zone) {

						if (ai_debug) {
							std::cout << "The current tile the enemy is on is:   [" << curr_pos_map.x << ", " << curr_pos_map.y << "]" << std::endl;
							std::cout << "The next tile the enemy will touch is: [" << next_pos_map.x << ", " << next_pos_map.y << "]" << std::endl;
						}

						if (game_state.map_tiles[next_pos_map.y][next_pos_map.x] != FREE_SPACE) {

							// Get traversable adjacent map tiles
							std::vector<vec2> adjacent_nodes = get_adj_nodes(curr_pos_map);

							// Randomly select a node to travel down
							vec2 random_node = adjacent_nodes[rand() % adjacent_nodes.size()];
							if (ai_debug) { std::cout << "Selected random node for enemy to travel: [" << random_node.x << ", " << random_node.y << "]" << std::endl; }

							// Travel in that direction
							// Travel right
							if (random_node.x > curr_pos_map.x) {
								entity_motion.velocity.x = enemy_move_speed;
								entity_motion.velocity.y = 0;
							}
							// Travel left
							else if (random_node.x < curr_pos_map.x) {
								entity_motion.velocity.x = -1 * enemy_move_speed;
								entity_motion.velocity.y = 0;
							}
							// Travel down
							if (random_node.y > curr_pos_map.y) {
								entity_motion.velocity.y = enemy_move_speed;
								entity_motion.velocity.x = 0;
							}
							// Travel up
							else if (random_node.y < curr_pos_map.y) {
								entity_motion.velocity.y = -1 * enemy_move_speed;
								entity_motion.velocity.x = 0;
							}
						}
					}
				}
			}
		}
	}

	// ========= Feature: Pathfinding (BFS search and player movement)=========
	if (do_generate_path) {
		generate_path(starting_map_pos, ending_map_pos);
	}

	if (do_pathfinding_movement) {

		if (path.size() > 0) {
			// Get the player's current map position
			vec2 player_map_pos = { (int)(motion.position.x / map_scale), (int)(motion.position.y / map_scale) };

			// Get the first destination in the path
			vec2 target_node = path.front();
			if (ai_debug) { std::cout << "Target node [" << target_node.x << ", " << target_node.y << "]" << std::endl; }

			// Must convert node map coords (ex. [0,1]) to world coords (ex. [400,600])
			// Otherwise player will stop at node edge, not node center.
			// Adding extra map_scale / 2 to get coords for center of map tile.
			vec2 target_node_coord = { map_scale * target_node.x + map_scale/2, map_scale * target_node.y + map_scale / 2 };

			// If the player has not eached the target node...

			int window = 5;
			if (!(target_node_coord.x < motion.position.x + window && target_node_coord.x > motion.position.x - window &&
				target_node_coord.y < motion.position.y + window && target_node_coord.y > motion.position.y - window)) {

				if (ai_debug) {
					std::cout << "Determining direction to move" << std::endl;
					std::cout << "Target coords: [" << target_node_coord.x << ", " << target_node_coord.y << "]" << std::endl;
					std::cout << "Player coords:  [" << (int)motion.position.x << ", " << (int)motion.position.y << "]" << std::endl;
				}

				//Move
				if		(target_node_coord.x > motion.position.x) { motion.velocity.x = player_vel;		}
				else if (target_node_coord.x < motion.position.x) { motion.velocity.x = -1* player_vel;  }
				if		(target_node_coord.y > motion.position.y) { motion.velocity.y = player_vel;		}
				else if (target_node_coord.y < motion.position.y) { motion.velocity.y = -1* player_vel;  }

			}

			// The player has reached the target node...
			else {
				if (ai_debug) { std::cout << "Reached node" << std::endl; }

				// Stop movement
				motion.velocity.x = 0;
				motion.velocity.y = 0;

				// Sneakily set player to center
				motion.position = target_node_coord;

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
		if (ai_debug) {
			std::cout << "========================================" << std::endl;
			std::cout << "Looking at node [ " << n.x << ", " << n.y << "]" << std::endl;
		}

		// Reached end
		if (n == ending_map_pos) {
			if (ai_debug) { std::cout << "reached end" << std::endl; }
			do_generate_path = false;

			path = trace(parent, starting_map_pos, ending_map_pos);

			// Debug: Print path
			if (ai_debug) {
				std::cout << " Path is: " << std::endl;
				for (auto node : path) {
					std::cout << "    [" << node.x << ", " << node.y << "]" << std::endl;
				}
			}

			do_pathfinding_movement = true;
		}
		else {
			// Get the list of traversable adjacent nodes
			std::vector<vec2> adjacent_nodes = get_adj_nodes(n);

			// Debug: Print list of adjacent nodes
			if (ai_debug) {
				std::cout << "Adjacent nodes are..." << std::endl;
				for (auto adj_node : adjacent_nodes) {
					std::cout << "    [" << adj_node.x << ", " << adj_node.y << "]" << std::endl;
				}
			}

			// if node is not in queue and not in visited.
			if (std::find(queue.begin(), queue.end(), n) == queue.end() && std::find(visited.begin(), visited.end(), n) == visited.end()) {
				if (ai_debug) { std::cout << "Node is not in queue and not in visited" << std::endl; }
				for (auto adj_node : adjacent_nodes) {

					// Debug
					if (ai_debug) { std::cout << "Looking at node... [" << adj_node.x << ", " << adj_node.y << "]" << std::endl; }

					// Set the parent node of the adjacent node to this current node
					parent.push_back({ adj_node, n });

					// Debug
					if (ai_debug) {
						std::cout << "pushed adj node: [" << adj_node.x << ", " << adj_node.y << "] to parent map" << std::endl;
						std::cout << "parent map now looks like this:" << std::endl;
						for (auto pair : parent) {
							std::cout << "    [" << pair[0].x << ", " << pair[0].y << "] : [" << pair[1].x << ", " << pair[1].y << "]" << std::endl;
						}
					}

					// Add the adjacent node to the queue to search later
					queue.push_back(adj_node);

					// Debug
					if (ai_debug) {
						std::cout << "pushed adj node: [" << adj_node.x << ", " << adj_node.y << "] to queue to search later" << std::endl;
						std::cout << "queue now looks like this:" << std::endl;
						for (auto node : queue) {
							std::cout << "    [" << node.x << ", " << node.y << "]" << std::endl;
						}
					}
				}

				// Add the current node to the visited list
				visited.push_back(n);

				// Debug
				if (ai_debug) {
					std::cout << "visited now looks like this " << std::endl;
					for (auto node : visited) {
						std::cout << "    [" << node.x << ", " << node.y << "]" << std::endl;
					}
				}
			}
			else {
				if (ai_debug) { std::cout << "Already visited or is in queue" << std::endl; }
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

		// Debug
		if (ai_debug) { std::cout << "Current node is: [" << curr_node.x << "," << curr_node.y << "]" << std::endl; }

		for (auto pair : parent) {
			if (pair[0] == curr_node) {
				parent_node = pair[1];

				//Debug
				if (ai_debug) { std::cout << "Found parent node: [" << parent_node.x << "," << parent_node.y << "]" << std::endl; }

				break;
			}
		}
		result.push_back(parent_node);
		// Debug
		if (ai_debug) {
			std::cout << "Pushed parent node [" << parent_node.x << "," << parent_node.y << "] to results" << std::endl;
			std::cout << "results look like this: " << std::endl;
			for (auto node : result) {
				std::cout << "    [" << node.x << "," << node.y << "]" << std::endl;
			}
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