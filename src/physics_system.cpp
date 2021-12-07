// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>


// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle.
bool collides(const Motion& motion1, const Motion& motion2)
{
	if (motion1.can_collide && motion2.can_collide) {
		vec2 dp = motion1.position - motion2.position;
		float dist_squared = dot(dp, dp);
		const vec2 other_bounding_box = get_bounding_box(motion1) / 2.f;
		const float other_r_squared = dot(other_bounding_box, other_bounding_box);
		const vec2 my_bounding_box = get_bounding_box(motion2) / 2.f;
		const float my_r_squared = dot(my_bounding_box, my_bounding_box);
		const float r_squared = max(other_r_squared, my_r_squared);
		if (dist_squared < r_squared || dist_squared == 0)
			return true;
	}
	return false;
}

void impulseCollisionResolution(Motion& player_motion, Motion& motion_other) {
	// Based on math derived here: https://www.randygaul.net/2013/03/27/game-physics-engine-part-1-impulse-resolution/
	// Using impulse to resolve collisions
	vec2 norm = glm::normalize(motion_other.position - player_motion.position); // collision normal
	vec2 relative_vel = motion_other.velocity - player_motion.velocity;
	if (glm::dot(norm, relative_vel) > 0) // don't calculate this more than once
		return;
	float coeff_restitution = std::min(player_motion.coeff_rest, motion_other.coeff_rest);

	// calculated based on "VelocityA + Impulse(Direction) / MassA - VelocityB + Impulse(Direction) / MassB = -Restitution(VelocityRelativeAtoB) * Direction"
	float impulse_magnitude = -(coeff_restitution + 1) * glm::dot(norm, relative_vel) / (1 / player_motion.mass + 1 / motion_other.mass);
	vec2 impulse = impulse_magnitude * norm; // impulse along direction of collision norm

	// velocity is changed relative to the mass of objects in the collision
	player_motion.velocity = player_motion.velocity - impulse / player_motion.mass;
	motion_other.velocity = motion_other.velocity + impulse / motion_other.mass;
}

// returns true if successfull, false if it didn't set
bool setMotionPosition(Motion& motion, vec2 nextpos, bool is_player = false) {
	if (motion.can_collide) {
		vec2 bounding_box = get_bounding_box(motion);
		vec2 corners[] = {
			// upper right
			vec2(bounding_box.x / 2, -bounding_box.y / 2),

			// upper left
			vec2(-bounding_box.x / 2, -bounding_box.y / 2),

			// lower left
			vec2(-bounding_box.x / 2, bounding_box.y / 2),

			// lower right
			vec2(bounding_box.x / 2, bounding_box.y / 2),
		};

		bool collision_x = false;
		bool collision_y = false;
		for (const auto corner : corners) {
			const vec2 test_point_x = WorldSystem::position_to_map_coords({ nextpos.x + corner.x, motion.position.y + corner.y });
			const MapTile tile_x = WorldSystem::get_map_tile(test_point_x);
			if (!WorldSystem::tile_is_walkable(tile_x) || !WorldSystem::is_within_bounds(test_point_x)) {
				collision_x = true;
			}

			const vec2 test_point_y = WorldSystem::position_to_map_coords({ motion.position.x + corner.x, nextpos.y + corner.y });
			const MapTile tile_y = WorldSystem::get_map_tile(test_point_y);
			if (!WorldSystem::tile_is_walkable(tile_y) || !WorldSystem::is_within_bounds(test_point_y)) {
				collision_y = true;
			}
		}

		if (!collision_x) {
			if (is_player) { motion.prev_pos.x = motion.position.x; }
			motion.position.x = nextpos.x;
		}

		if (!collision_y) {
			if (is_player) { motion.prev_pos.y = motion.position.y; }
			motion.position.y = nextpos.y;
		}

		return !collision_x && !collision_y;
	}
	// Non-collidable objects
	// For entities like the background stars that have motion but no collision
	else {
		motion.position.x = nextpos.x;
		motion.position.y = nextpos.y;
	}

	return true;

}

// TODO: Optimization needed for overlap handling/clipping, weird behaviour occurring with certain collisions
void preventCollisionOverlap(entt::entity entity, entt::entity other) {
	Motion& player = registry.get<Motion>(entity);
	Motion& enemy = registry.get<Motion>(other);
	const float pen_scaling_factor = 10;
	vec2 box_player = get_bounding_box(player);
	vec2 box_enemy = get_bounding_box(enemy);
	float x_enemy_bound = enemy.position[0] - box_enemy[0] / 2.f;
	float x_player_bound = player.position[0] + box_player[0] / 2.f;
	float x_penetration = x_player_bound - x_enemy_bound;
	x_penetration /= pen_scaling_factor;
	float y_rectangle_bound = enemy.position[1] - box_enemy[1] / 2.f;
	float y_circle_bound = player.position[1] + box_player[1] / 2.f;
	float y_penetration = y_circle_bound - y_rectangle_bound;
	y_penetration /= pen_scaling_factor;

	if (glm::dot(enemy.velocity, player.velocity) > 0) {
		vec2 player_nextpos = {0.0, 0.0};
		vec2 enemy_nextpos = {0.0, 0.0};

		if (player.velocity[0] < 0) {
			player_nextpos[0] = player.position[0] + x_penetration / 2.f;
			enemy_nextpos[0] = enemy.position[0] - x_penetration / 2.f;
		}

		else {
			player_nextpos[0] = player.position[0] - x_penetration / 2.f;
			enemy_nextpos[0] = enemy.position[0] + x_penetration / 2.f;
		}

		if (player.velocity[1] > 0) {
			player_nextpos[1] = player.position[1] + y_penetration / 2.f;
			enemy_nextpos[1] = enemy.position[1] - y_penetration / 2.f;
		}

		else {
			player_nextpos[1] = player.position[1] - y_penetration / 2.f;
			enemy_nextpos[1] = enemy.position[1] + y_penetration / 2.f;
		}

		setMotionPosition(player, player_nextpos);
		setMotionPosition(enemy, enemy_nextpos);
	}
}

void drawDebuggingMeshes(entt::entity& entity) {
	Motion& motion = registry.get<Motion>(entity);
	Mesh* meshPtrs = registry.get<Mesh*>(entity);
	std::vector<uint16_t> vertex_indices = meshPtrs->vertex_indices;
	std::vector<ColoredVertex> vertices = meshPtrs->vertices;
	mat3 S = { { motion.scale.x, 0.f, 0.f },{ 0.f, motion.scale.y, 0.f },{ 0.f, 0.f, 1.f } };
	float c = cosf(motion.angle);
	float s = sinf(motion.angle);
	mat3 R = { { c, -s, 0.f }, { s, c, 0.f }, { 0.f, 0.f, 1.f } };
	mat3 T = { { 1.f, 0.f, motion.position.x }, { 0.f, 1.f, motion.position.y }, { 0.f, 0.f, 1.f } };
	mat3 proj = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f } };
	for (auto& vertex : vertices) {
		vec3 object_coord = { vertex.position.x, vertex.position.y, 1.f };
		vec3 scaled = object_coord * S;
		vec3 rotated = scaled * R;
		vec3 translated = rotated * T;
		vec3 projected = translated * proj;
		vec2 vertexCoord = { projected.x, projected.y };
		createLine(vertexCoord, { 5.f, 5.f });
	}
}

void drawDebuggingBoxes(entt::entity& entity) {
	Motion& motion = registry.get<Motion>(entity);
	const vec2 bonding_box = get_bounding_box(motion);
	// Bounding boxes for prey, minotaur, items, and chicks
	// Top left to bottom left
	createLine({ motion.position.x - (bonding_box.x / 2.f), motion.position.y }, { motion.scale.x / 20.f, bonding_box.y });
	// Top left to top right
	createLine({ motion.position.x, motion.position.y - (bonding_box.y / 2.f) }, { bonding_box.x,  motion.scale.x / 20.f });
	// Bottom left to bottom right
	createLine({ motion.position.x, motion.position.y + (bonding_box.y / 2.f) }, { bonding_box.x,  motion.scale.x / 20.f });
	// Top Right to bottom right
	createLine({ motion.position.x + (bonding_box.x / 2.f), motion.position.y }, { motion.scale.x / 20.f, bonding_box.y });
}

void PhysicsSystem::step(float elapsed_ms, float window_width_px, float window_height_px)
{

	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	for (entt::entity entity : registry.view<Motion>())
	{
		float step_seconds = 1.0f * (elapsed_ms / 1000.f);
		Motion& motion = registry.get<Motion>(entity);
		vec2 nextpos = motion.position + motion.velocity * step_seconds;

		// If the entity is not the player...
		if (!registry.view<Player>().contains(entity)) {
			// If the entity is not the background or hud either...
			// (we only want the background and hud to move if the
			//  player can pass the setMotionPosition collision 
			//  check)
			if (!registry.view<Background>().contains(entity) && !registry.view<HUD>().contains(entity)) {
				// Move the entity only if the player is moving
				if (player_is_manually_moving || do_pathfinding_movement) {
					setMotionPosition(motion, nextpos);
				}
			}

		}
		// If the entity is the player...
		else {
			if (!in_a_cutscene) {
				// Attempt to move the player, and if player is able to move (no collision = return True)...
				// Warning: If player is moving, ex rightwards in to a wall, setMotionPosition returns False
				// as expected due to collision. However, if moving into a wall AND moving up along a hallway,
				// setMotionPosition still returns false but player is still able to move up
				if (setMotionPosition(motion, nextpos, true)) {

					// Move only star background for parallax effect
					for (entt::entity entity : background_entities) {
						Motion& bg_motion = registry.get<Motion>(entity);
						vec2 bg_next_pos = bg_motion.position + bg_motion.velocity * step_seconds;
						setMotionPosition(bg_motion, bg_next_pos);
					}
					//Motion& bg_motion_1 = registry.get<Motion>(background_space2_entity);
					//vec2 bg_1_nextpos = bg_motion_1.position + bg_motion_1.velocity * step_seconds;
					//setMotionPosition(bg_motion_1, bg_1_nextpos);

					//Motion& bg_motion_2 = registry.get<Motion>(background_space3_entity);
					//vec2 bg_2_nextpos = bg_motion_2.position + bg_motion_2.velocity * step_seconds;
					//setMotionPosition(bg_motion_2, bg_2_nextpos);

					// Move all elements of the HUD
					for (entt::entity entity : registry.view<HUD>()) {
						Motion& hud_entity_motion = registry.get<Motion>(entity);
						vec2    hud_entity_nextpos = hud_entity_motion.position + hud_entity_motion.velocity * step_seconds;
						setMotionPosition(hud_entity_motion, hud_entity_nextpos);
					}
				}
			}
		}
	}

	entt::entity player = registry.view<Player>().begin()[0];
	Motion& player_motion = registry.get<Motion>(player);

	// Deal with spell speed while moving
	if (spellbook[1]["active"] == "true" || registry.view<SpeedBoostTimer>().contains(player)) {
		if (player_motion.velocity.x > 0) {
			player_motion.velocity.x = 600.f * global_scaling_vector.x;
		}
		else if (player_motion.velocity.x < 0) {
			player_motion.velocity.x = -600.f * global_scaling_vector.x;
		}
		if (player_motion.velocity.y > 0) {
			player_motion.velocity.y = 600.f * global_scaling_vector.y;
		}
		else if (player_motion.velocity.y < 0) {
			player_motion.velocity.y = -600.f * global_scaling_vector.y;
		}
	}

	// Check for collisions between all moving entities
	auto items_registry = registry.view<Item>();
	for(entt::entity entity : registry.view<Motion>())
	{
		Motion& motion = registry.get<Motion>(entity);
		for(entt::entity other : registry.view<Motion>()) // i+1
		{
			bool other_is_item = items_registry.contains(other);
			bool ent_is_item = items_registry.contains(entity);

			// Allow enemies to pass over items
			if (entity == other || (ent_is_item && other != player) || (other_is_item && entity != player))
				continue;

			Motion& motion_other = registry.get<Motion>(other);
			if (collides(motion, motion_other))
			{
				// player-prey collision
				if ((entity == player && registry.view<Prey>().contains(other)) || (other == player && registry.view<Prey>().contains(entity))) {
					notify(entity, other, Event::PLAYER_PREY_COLLISION);
					continue;
				}

				// player-enemy collision
				if ((entity == player && registry.view<Enemy>().contains(other)) || (other == player && registry.view<Enemy>().contains(entity))) {
					notify(entity, other, Event::PLAYER_ENEMY_COLLISION);
					preventCollisionOverlap(other, entity);
					impulseCollisionResolution(registry.get<Motion>(entity), registry.get<Motion>(other));
					continue;
				}

				// If collision involves an item and the player
				if (other_is_item || ent_is_item) {
					// Add item to inventory, prompt text from tips
					// remove item from world
					game_state.sound_requests.push_back({SoundEffects::PLAYER_ITEM});
					if (ent_is_item) {
						Item& item = items_registry.get<Item>(entity);
						inventory[item_to_enum[item.name]]++;
						most_recent_collected_item = items_registry.get<Item>(entity);
						std::cout << "Picked up a " << item.name << "!" << std::endl;
						registry.destroy(entity);
					} else {
						Item& item = items_registry.get<Item>(other);
						inventory[item_to_enum[item.name]]++;
						most_recent_collected_item = items_registry.get<Item>(other);
						std::cout << "Picked up a " << item.name << "!" << std::endl;
						registry.destroy(other);
					}
					// Start timer for 5 second text tip
					registry.emplace_or_replace<TextTimer>(player);
					tips.picked_up_item = 1;
					tips.basic_help = 0;
					return;
				}
				if (!(registry.view<Prey>().contains(entity)||registry.view<Prey>().contains(other)) ||
					(registry.view<Prey>().contains(entity) && registry.view<Prey>().contains(other))) {
					impulseCollisionResolution(motion, motion_other);
				}
				registry.emplace_or_replace<Collision>(entity, other);

				// TODO: Optimization needed for overlap handling/clipping
				if (!(registry.view<Prey>().contains(entity) || registry.view<Prey>().contains(other)) ||
					(registry.view<Prey>().contains(entity) && registry.view<Prey>().contains(other))) {
					preventCollisionOverlap(other, entity);
				}
			}
		}
	}

	// debugging of bounding boxes
	if (debugging.in_debug_mode)
	{
		// Dots for every vertex of enemy meshes
		for (auto enemy_entity : registry.view<Enemy>()) {
			drawDebuggingMeshes(enemy_entity);
		}

		// Draw bounding boxes for all moving entities (not including HUD, cutscenes, etc)
		for (auto prey : registry.view<Prey>()) {
			drawDebuggingBoxes(prey);
		}
		for (auto player : registry.view<Player>()) {
			drawDebuggingBoxes(player);
		}
		for (auto item : registry.view<Item>()) {
			drawDebuggingBoxes(item);
		}

	}
}
