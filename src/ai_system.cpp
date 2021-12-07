// internal
#include "ai_system.hpp"
#include <vector>
#include <iostream>
#include <algorithm> // for reversing a vector

// Pathfinding Datastructure
// Hold a vector of coordinates.
// For example: If starting at [0,1]:
//   [[0,1], [1,1], [2,1], [3,1]]
// corresponds to traveling 3 tiles to the right
std::vector<vec2> path;
std::vector<ChickAI> chick_ai;

// Variables
extern bool  do_pathfinding_movement	 = false;
	   bool	 do_calculate_new_enemy_path = false;
	   bool	 ai_debug					 = false; // set to true to print debug statements
	   float DIST_THRESHOLD			     = 200.f; // distance threshold between enemy and player
	   float swing_threshold			 = 200.f; // Enemy has to be close enough to register as a hit

void AISystem::step()
{
	entt::entity player = registry.view<Player>().begin()[0];
	Motion& motion = registry.get<Motion>(player);

	// For each entity
	for (entt::entity entity : registry.view<Motion>())
	{
		if (registry.view<Prey>().contains(entity))
			continue;
		Motion& entity_motion = registry.get<Motion>(entity);
		vec2 entity_velocity = entity_motion.velocity;

		if (registry.view<Item>().contains(entity)) {
			continue;
		}

		// If it's an enemy entity...
		if (entity != player)
		{

			// =========== Swinging Attack ===========
			// If player is attacking
			if (registry.view<Attack>().contains(player)) {

				// If entity is attackable and not a cutscene entity...
				if (entity_motion.can_be_attacked && !registry.view<Cutscene>().contains(entity)) {
					// If enemy is close to player for swinging...
					if (within_threshold(player, entity, swing_threshold)) {

						RenderRequest &render_request = registry.get<RenderRequest>(player);
					// If enemy is to the right of the player and the player is facing right...
   					if (entity_motion.position.x >= motion.position.x && !render_request.is_reflected) {
						
						// Destroy the enemy
						// registry.destroy(entity);
						entity_motion.can_collide = false;
						if (!registry.view<DeathTimer>().contains(entity)) registry.emplace<DeathTimer>(entity);
						// Don't process any of the code below for the enemy
						// because it's no longer in the registry
						break;
					}

					// If enemy is to the left of the player and the player is facing left
					else if (entity_motion.position.x < motion.position.x && render_request.is_reflected) {
						// registry.destroy(entity);
						entity_motion.can_collide = false;
						if (!registry.view<DeathTimer>().contains(entity)) registry.emplace<DeathTimer>(entity);
						break;
					}

					// If enemy is vertically close to the player, just destroy them
					else if (abs(entity_motion.position.y - motion.position.y) < 100) {
						// registry.destroy(entity);
						entity_motion.can_collide = false;
						if (!registry.view<DeathTimer>().contains(entity)) registry.emplace<DeathTimer>(entity);
						break;
					}
				}
			}
			}

			// ========================================================

			// If enemy...
			if (registry.view<Enemy>().contains(entity)) {

				// Is close enough for chasing && player isn't dead
				if (within_threshold(player, entity, DIST_THRESHOLD) && !registry.view<DeathTimer>().contains(player)) {
					vec2 direction_vector = motion.position - entity_motion.position;
					entity_motion.velocity = direction_vector;
				}


				// Not close enough for chasing
				else {

					// ========= Feature: Pathfinding (enemy maze navigation) =========

					// If enemy is travelling diagonally (ex from chasing a player),
					// randomly choose one or the other axis. This prevents the AI's
					// anticipated trajectory into a diagonal tile.
					if (entity_motion.velocity.x != 0 && entity_motion.velocity.y != 0) {
						bool go_y = rand() % 2;
						if (go_y) {
							entity_motion.velocity.x = 0;
							entity_motion.velocity.y = enemy_vel.y;
						}
						else {
							entity_motion.velocity.x = enemy_vel.x;
							entity_motion.velocity.y = 0;
						}
					}

					// Get the world coords for the projected position
					vec2 next_pos_world = { entity_motion.position.x + (entity_motion.velocity.x), entity_motion.position.y + (entity_motion.velocity.y) };

					// Get the world length in px for boundary checks
					vec2 world_length_px = { game_state.level.map_tiles.size() * map_scale.x, game_state.level.map_tiles.size() * map_scale.y };

					// Will get vector access error out of bounds if we don't do the following check
					// If the next coords are in-bounds...
					if (next_pos_world.x >= -70 && next_pos_world.y >= 0 && next_pos_world.x <= world_length_px.x && next_pos_world.y <= world_length_px.y) {
						vec2 next_pos_map = { (int)(next_pos_world.x / map_scale.x), (int)(next_pos_world.y / map_scale.y) };
						vec2 curr_pos_map = { (int)(entity_motion.position.x / map_scale.x), (int)(entity_motion.position.y / map_scale.y) };

						// Get the current tile's world coords
						vec2 tile_world_coord = { map_scale.x * curr_pos_map.x, map_scale.y * curr_pos_map.y };

						// Check to prevent wall corner collision. See function for more details
						bool in_safe_zone = safe_zone_check(entity_motion, tile_world_coord);

						// If enemy is in the safe zone...
						if (in_safe_zone) {

							// Debugging
							if (ai_debug) {
								std::cout << "The current tile the enemy is on is:   [" << curr_pos_map.x << ", " << curr_pos_map.y << "]" << std::endl;
								std::cout << "The next tile the enemy will touch is: [" << next_pos_map.x << ", " << next_pos_map.y << "]" << std::endl;
							}

							// If the next tile the enemy is projected to travel to is not a free space (i.e. it'll hit a wall)...
							// or if it's out of map bounds... (for now, hard coded to avoid [0][1]
							if (game_state.level.map_tiles[next_pos_map.y][next_pos_map.x] != FREE_SPACE ||
								(next_pos_map.x == 0 && next_pos_map.y == 1)) {

								// Get traversable adjacent map tiles
								std::vector<vec2> adjacent_nodes = get_adj_nodes(curr_pos_map);

								// Randomly select a node to travel down
								// Special case: If enemy is in exit tile, adjacent_nodes will be empty
								// To prevent error, have a special case that says:
								// If adjacent_nodes is empty, just move left
								if (adjacent_nodes.size() == 0) {
									// Travel left
									entity_motion.velocity.x = -1 * enemy_vel.x;
									entity_motion.velocity.y = 0;
								}
								else {
									vec2 random_node = adjacent_nodes[rand() % adjacent_nodes.size()];
									if (ai_debug) { std::cout << "Selected random node for enemy to travel: [" << random_node.x << ", " << random_node.y << "]" << std::endl; }

									// Travel in that direction
									// Travel right
									if (random_node.x > curr_pos_map.x) {
										entity_motion.velocity.x = enemy_vel.x;
										entity_motion.velocity.y = 0;
									}
									// Travel left
									else if (random_node.x < curr_pos_map.x) {
										entity_motion.velocity.x = -1 * enemy_vel.x;
										entity_motion.velocity.y = 0;
									}
									// Travel down
									if (random_node.y > curr_pos_map.y) {
										entity_motion.velocity.y = enemy_vel.y;
										entity_motion.velocity.x = 0;
									}
									// Travel up
									else if (random_node.y < curr_pos_map.y) {
										entity_motion.velocity.y = -1 * enemy_vel.y;
										entity_motion.velocity.x = 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	for (auto& ai : chick_ai) {
		ai.step();
	}

	// ========= Feature: Pathfinding (BFS search and player movement)=========
	if (do_generate_path) { 
		generate_path(starting_map_pos, ending_map_pos); 
	}

	if (do_pathfinding_movement) {

		if (path.size() > 0) {
			// Get the player's current map position
			vec2 player_map_pos = { (int)(motion.position.x / map_scale.x), (int)(motion.position.y / map_scale.y) };

			// Get the first destination in the path
			vec2 target_node = path.front();
			if (ai_debug) { std::cout << "Target node [" << target_node.x << ", " << target_node.y << "]" << std::endl; }

			// Must convert node map coords (ex. [0,1]) to world coords (ex. [400,600])
			// Otherwise player will stop at node edge, not node center.
			// Adding extra map_scale / 2 to get coords for center of map tile.
			vec2 target_node_coord = { map_scale.x * target_node.x + map_scale.x/2, map_scale.y * target_node.y + map_scale.y / 2 };

			// If the player has not reached the center* of the target node...
			// The center is determined by a small square, hence window threshold
			// Note target_node_coord is the center x,y pixels of a tile.
			int window = 15;
			if (!(motion.position.x < target_node_coord.x + window && motion.position.x > target_node_coord.x - window &&
				motion.position.y < target_node_coord.y + window && motion.position.y > target_node_coord.y - window)) {

				if (ai_debug) {
					std::cout << "Determining direction to move" << std::endl;
					std::cout << "Target coords: [" << target_node_coord.x << ", " << target_node_coord.y << "]" << std::endl;
					std::cout << "Player coords:  [" << motion.position.x << ", " << motion.position.y << "]" << std::endl;
				}

				//Move
				if		(target_node_coord.x > motion.position.x) {  motion.velocity.x =     player_vel.x;  } // Move right
				else if (target_node_coord.x < motion.position.x) {  motion.velocity.x = -1* player_vel.y;  } // Move left
				// Move down
				// Move down
				if		(target_node_coord.y > motion.position.y) {
					motion.velocity.y =		player_vel.y;
					if (abs(target_node_coord.x - motion.position.x) < 5) { motion.velocity.x = 0; } // To prevent left/right jitter
				}
				// Move up
				else if (target_node_coord.y < motion.position.y) {
					motion.velocity.y = -1* player_vel.y;
					if (abs(target_node_coord.x - motion.position.x) < 5) { motion.velocity.x = 0; } // To prevent left/right jitter
				}
			}

			// The player has reached the target node...
			else {
				if (ai_debug) { std::cout << "Reached node" << std::endl; }

				// ============ Stop movement so the player doesn't keep traveling forward ===========
				// This is important to be able to turn the corner and/or prevent running into the wall

				// In render.cpp, if velocity.x < 0, the sprite will be transformed to face left. When
				// the sprite is moving left, it'll face right when it reaches a node because it'll set
				// set the motion.velocity.x = 0. Then if it moves left again, it'll turn left. This
				// creates a jitter effect. The problem is resolved if the game still sees the sprite as
				// moving left as shown below.

				if (motion.velocity.x < 0) {  motion.velocity.x = -0.001f;  }
				else { motion.velocity.x = 0; }
				motion.velocity.y = 0;

				// Move the player to center of the tile to prevent jitter
				motion.position = target_node_coord;

				// Remove the first node in path because we've reached it
				path.erase(path.begin());
			}
		}

		// The player has reached the end of the BFS path
		else {
			if (ai_debug) { std::cout << "Path size is 0 - reached destination" << std::endl; }
			do_pathfinding_movement = false;

			motion.velocity.x = 0;
			motion.velocity.y = 0;
		}
	}
}


// ========= Feature: Pathfinding =========
void AISystem::generate_path(vec2 starting_map_pos, vec2 ending_map_pos) {
	std::vector<std::vector<vec2>> parent;
	std::vector<vec2> visited;
	std::vector<vec2>   queue;
	std::set<vec2>  set_queue;

	// Clear the previous path so we can construct a new one.
	path.clear();

	// Push the starting pos into the path
	queue.push_back(starting_map_pos);

	while (queue.size() > 0 && do_generate_path == true) {
		// Important: node is a vec2 which means a node that looks like [0,1] means
		// that the x (the column) is 0 and the y (the row) is 1.

		// Pop the first node
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
			// if list of adjacent nodes is not empty
			if (adjacent_nodes.size() > 0) {
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
	size_t max_size = game_state.level.map_tiles.size();

	// Try-catch. If clicked out of bounds, causing .map_tiles[][] to be out of vector bounds, will return empty {}.
	try {

		// For now, assuming map_tile == 0 is the only traversable tile
		// If adjacent node is within bounds and map_tile == 0, add to adjacency list.
		// Check right
		if (WorldSystem::is_within_bounds({ root_node.x + 1,root_node.y })) {
			if (WorldSystem::tile_is_walkable(game_state.level.map_tiles.at(root_node.y).at(root_node.x + 1))) {
				adj_nodes.push_back({ root_node.x + 1, root_node.y });
			}
		}
		// Check down
		/*if (root_node.y + 1 <= max_size) {
			if (WorldSystem::tile_is_walkable(game_state.level.map_tiles[root_node.y + 1][root_node.x])) {
				adj_nodes.push_back({ root_node.x,root_node.y + 1 });
			}
		}*/
		if (WorldSystem::is_within_bounds({ root_node.x,root_node.y + 1})) {
			if (WorldSystem::tile_is_walkable(game_state.level.map_tiles[root_node.y + 1][root_node.x])) {
				adj_nodes.push_back({ root_node.x,root_node.y + 1 });
			}
		}
		// Check left
		//if (root_node.x - 1 >= 0) {
		//	if (WorldSystem::tile_is_walkable(game_state.level.map_tiles[root_node.y][root_node.x - 1])) {
		//		adj_nodes.push_back({ root_node.x - 1, root_node.y });
		//	}
		//}
		if (WorldSystem::is_within_bounds({ root_node.x - 1,root_node.y })) {
			if (WorldSystem::tile_is_walkable(game_state.level.map_tiles[root_node.y][root_node.x - 1])) {
				adj_nodes.push_back({ root_node.x - 1, root_node.y });
			}
		}
		// Check up
		//if (root_node.y - 1 >= 0) {
		//	if (WorldSystem::tile_is_walkable(game_state.level.map_tiles[root_node.y - 1][root_node.x])) {
		//		adj_nodes.push_back({ root_node.x, root_node.y - 1 });
		//	}
		//}
		if (WorldSystem::is_within_bounds({ root_node.x,root_node.y - 1 })) {
			if (WorldSystem::tile_is_walkable(game_state.level.map_tiles[root_node.y - 1][root_node.x])) {
				adj_nodes.push_back({ root_node.x, root_node.y - 1 });
			}
		}
	}
	catch (...) {
		std::cout << "Must click in maze bounds for pathfinding!" << std::endl;
		do_generate_path = false;
	}

	return adj_nodes;
}

bool AISystem::within_threshold(entt::entity entity, entt::entity other, float threshold)
{
	float dist_threshold_x = 140.f * global_scaling_vector.x;
	float dist_threshold_y = 140.f * global_scaling_vector.y;
	float DIST_THRESHOLD = sqrt(pow(dist_threshold_x, 2) + pow(dist_threshold_y, 2)); // scaled distance threshold between enemy and player
	Motion& entity_motion = registry.get<Motion>(entity);
	Motion& other_motion = registry.get<Motion>(other);

	vec2 direction_vector = entity_motion.position - other_motion.position;
	float distance = sqrt(dot(direction_vector, direction_vector));
	return distance <= threshold;
}

bool AISystem::safe_zone_check(Motion &entity_motion, vec2 tile_world_coord) {
	// To prevent getting caught in corners make sure the enemy's position
	// is within a 'safe zone' that prevents the extremities of the object
	// from clipping the edges of a wall tile.
	//  _____________
	// |   |     |   |
	// |___|     |___|
	// |  safe zone  |
	// |___       ___|
	// |   |     |   |
	// |___|_____|___|
	// Each tile is [scale] pixels wide. ATM it's 150px.
	// Safe zone is between ~ 150/4 to 150*3/4

	// Debugging
	if (ai_debug) {
		std::cout << "Safe zone x range: " << tile_world_coord.x + map_scale.x * .25 << " - " << tile_world_coord.x + map_scale.x * .75 << std::endl;
		std::cout << "Safe zone y range: " << tile_world_coord.y + map_scale.x * .25 << " - " << tile_world_coord.y + map_scale.x * .75 << std::endl;
		std::cout << "Enemy position: " << entity_motion.position.x << ", " << entity_motion.position.y << std::endl;
	}

	// If the enemy is moving vertically, make sure they're in the vertical safe zone
	if (entity_motion.velocity.y != 0) {
		if (entity_motion.position.x <= tile_world_coord.x + map_scale.x * .25) {
			entity_motion.velocity.x = enemy_vel.x;
			return false;
		}
		else if (entity_motion.position.x >= tile_world_coord.x + map_scale.x * .75) {
			entity_motion.velocity.x = -1 * enemy_vel.x;
			return false;
		}
		else {
			return true;
		}

	}
	// If the enemy is moving horizontally, make sure they're in the horizontal safe zone
	else if (entity_motion.velocity.x != 0) {
		if (entity_motion.position.y <= tile_world_coord.y + map_scale.y * .25) {
			entity_motion.velocity.y = enemy_vel.y;
			return false;
		}
		else if (entity_motion.position.y >= tile_world_coord.y + map_scale.y * .75) {
			entity_motion.velocity.y = -1 * enemy_vel.y;
			return false;
		}
		else {
			return true;
		}
	}
	return false;
}
