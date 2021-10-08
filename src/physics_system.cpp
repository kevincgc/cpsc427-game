// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include<iostream>

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
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bounding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bounding_box, other_bounding_box);
	const vec2 my_bounding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bounding_box, my_bounding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
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
		if (player.velocity[0] < 0) {
			player.position[0] = player.position[0] + x_penetration / 2.f;
			enemy.position[0] = enemy.position[0] - x_penetration / 2.f;
		}
		else {
			player.position[0] = player.position[0] - x_penetration / 2.f;
			enemy.position[0] = enemy.position[0] + x_penetration / 2.f;
		}
		if (player.velocity[1] > 0) {
			player.position[1] = player.position[1] + y_penetration / 2.f;
			enemy.position[1] = enemy.position[1] - y_penetration / 2.f;
		}
		else {
			player.position[1] = player.position[1] - y_penetration / 2.f;
			enemy.position[1] = enemy.position[1] + y_penetration / 2.f;
		}
	}
}

void PhysicsSystem::step(float elapsed_ms, float window_width_px, float window_height_px)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	for(entt::entity entity: registry.view<Motion>())
	{
		Motion& motion = registry.get<Motion>(entity);
		float step_seconds = 1.0f * (elapsed_ms / 1000.f);
		motion.position[0] += motion.velocity[0] * step_seconds;
		motion.position[1] += motion.velocity[1] * step_seconds;
	}

	entt::entity player = registry.view<Player>().begin()[0];
	Motion& motion = registry.get<Motion>(player);

	// Deal with spell speed while moving
	if (spellbook[1]["active"] == "true") {
		if (motion.velocity.x > 0) {
			motion.velocity.x = 800.f;
		}
		else if (motion.velocity.x < 0) {
			motion.velocity.x = -800.f;
		}
		if (motion.velocity.y > 0) {
			motion.velocity.y = 800.f;
		}
		else if (motion.velocity.y < 0) {
			motion.velocity.y = -800.f;
		}
	}
	

	// ========= Below is the implementation for an acceleration/flag-based movement system (for more accurate movement later) ========= //

	// // Movement
	// float& x_vel = motion.velocity.x;
	// float& y_vel = motion.velocity.y;
	// // Temporary implementation
	// // Update minotaur speed if the spell is active
	// float accel = spellbook[1]["active"] == "true" ?		60 : 30;
	// float max_vel = spellbook[1]["active"] == "true" ?		500 : 200;
	// float slow_factor = spellbook[1]["active"] == "true" ?	20 : 10;
	// if (!registry.view<DeathTimer>().contains(player)) {
	// 	if (move_right)		{ x_vel += accel; }
	// 	else if (move_left) { x_vel += -1 * accel; }
	// 	if (move_up)		{ y_vel += -1 * accel; }
	// 	else if (move_down) { y_vel += accel; }
	// }

	// // Impose max velocity for minotaur
	// if (x_vel > max_vel) { x_vel = max_vel; }
	// else if (x_vel < -1 * max_vel) { x_vel = -1 * max_vel; }
	// if (y_vel > max_vel) { y_vel = max_vel; }
	// else if (y_vel < -1 * max_vel) { y_vel = -1 * max_vel; }


	// // Friction to slow minotaur down
	
	// if (x_vel != 0 || y_vel != 0) {
	// 	if (x_vel > 0) { x_vel -= slow_factor; }
	// 	else if (x_vel < 0) { x_vel += slow_factor; }
	// 	if (y_vel > 0) { y_vel -= slow_factor; }
	// 	else if (y_vel < 0) { y_vel += slow_factor; }
	// }



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
				registry.emplace<Collision>(entity, other);

				// TODO: Optimization needed for overlap handling/clipping
				//preventCollisionOverlap(other, entity);
				
			}
		}
	}

	// you may need the following quantities to compute wall positions
	(float)window_width_px; (float)window_height_px;

	auto motion_view = registry.view<Motion>();
	// debugging of bounding boxes
	if (debugging.in_debug_mode)
	{
		uint size_before_adding_new = (uint)motion_view.size();
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			entt::entity entity_i = motion_view.begin()[i];
			Motion& motion_i = registry.get<Motion>(entity_i);

			// visualize the radius with two axis-aligned lines
			const vec2 bonding_box = get_bounding_box(motion_i);
			float radius = sqrt(dot(bonding_box/2.f, bonding_box/2.f));
		}
	}
}