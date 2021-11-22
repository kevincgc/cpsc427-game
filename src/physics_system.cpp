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
	float coeff_restitution = min(player_motion.coeff_rest, motion_other.coeff_rest);

	// calculated based on "VelocityA + Impulse(Direction) / MassA - VelocityB + Impulse(Direction) / MassB = -Restitution(VelocityRelativeAtoB) * Direction"
	float impulse_magnitude = -(coeff_restitution + 1) * glm::dot(norm, relative_vel) / (1 / player_motion.mass + 1 / motion_other.mass);
	vec2 impulse = impulse_magnitude * norm; // impulse along direction of collision norm

	// velocity is changed relative to the mass of objects in the collision
	player_motion.velocity = player_motion.velocity - impulse / player_motion.mass;
	motion_other.velocity = motion_other.velocity + impulse / motion_other.mass;
}

// returns true if successfull, false if it didn't set
bool setMotionPosition(Motion& motion, vec2 nextpos) {
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
		const vec2 test_point_x = WorldSystem::position_to_map_coords({nextpos.x + corner.x, motion.position.y + corner.y});
		const MapTile tile_x = WorldSystem::get_map_tile(test_point_x);
		if (!WorldSystem::tile_is_walkable(tile_x) || !WorldSystem::is_within_bounds(test_point_x)) {
			collision_x = true;
		}

		const vec2 test_point_y = WorldSystem::position_to_map_coords({motion.position.x + corner.x, nextpos.y + corner.y});
		const MapTile tile_y = WorldSystem::get_map_tile(test_point_y);
		if (!WorldSystem::tile_is_walkable(tile_y) || !WorldSystem::is_within_bounds(test_point_y)) {
			collision_y = true;
		}
	}

	if (!collision_x) {
		motion.position.x = nextpos.x;
	}

	if (!collision_y) {
		motion.position.y = nextpos.y;
	}

	return !collision_x && !collision_y;
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

void PhysicsSystem::step(float elapsed_ms, float window_width_px, float window_height_px)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	for (entt::entity entity : registry.view<Motion>())
	{
		float step_seconds = 1.0f * (elapsed_ms / 1000.f);
		vec2 nextpos;
		Motion& motion = registry.get<Motion>(entity);

		if (!tips.in_help_mode) { nextpos = motion.position + motion.velocity * step_seconds; }
		else { nextpos = motion.position; }

		if (player_is_manually_moving || do_pathfinding_movement) {
			setMotionPosition(motion, nextpos);
		}

		//if (!registry.view<Player>().contains(entity)) {
		//	if (player_is_manually_moving || do_pathfinding_movement) {
		//		setMotionPosition(motion, nextpos);
		//	}
		//}
		//else {
		//	setMotionPosition(motion, nextpos);
		//}
		//Motion& motion = registry.get<Motion>(entity);
		//float step_seconds = 1.0f * (elapsed_ms / 1000.f);
		//
		//vec2 nextpos;
		//if   (!tips.in_help_mode) { nextpos = motion.position + motion.velocity * step_seconds; }
		//else					  { nextpos = motion.position; }

		//setMotionPosition(motion, nextpos);


	}
	
	// Move enemy
	// If player is moving, move enemies too
	//if (player_is_manually_moving || do_pathfinding_movement) {
	//	for (int i = 1; i <= registry.size<Motion>; i++) {
	//		Motion& enemy_motion = registry.get<Motion>.begin()[i];
	//	}
	//}

	entt::entity player = registry.view<Player>().begin()[0];
	Motion& player_motion = registry.get<Motion>(player);

	// Deal with spell speed while moving
	if (spellbook[1]["active"] == "true") {
		if (player_motion.velocity.x > 0) {
			player_motion.velocity.x = 800.f;
		}
		else if (player_motion.velocity.x < 0) {
			player_motion.velocity.x = -800.f;
		}
		if (player_motion.velocity.y > 0) {
			player_motion.velocity.y = 800.f;
		}
		else if (player_motion.velocity.y < 0) {
			player_motion.velocity.y = -800.f;
		}
	}

	// Check for collisions between all moving entities
	for(entt::entity entity : registry.view<Motion>())
	{
		Motion& motion = registry.get<Motion>(entity);
		for(entt::entity other : registry.view<Motion>()) // i+1
		{
			if (entity == other)
				continue;

			Motion& motion_other = registry.get<Motion>(other);
			if (collides(motion, motion_other))
			{
				impulseCollisionResolution(motion, motion_other);
				registry.emplace_or_replace<Collision>(entity, other);

				// TODO: Optimization needed for overlap handling/clipping
				preventCollisionOverlap(other, entity);
			}
		}
	}
}
