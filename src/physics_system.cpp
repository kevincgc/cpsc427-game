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

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
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
		(void)elapsed_ms; // placeholder to silence unused warning until implemented
	}


	entt::entity player = registry.view<Player>().begin()[0];
	Motion& motion = registry.get<Motion>(player);

	// Movement
	float& x_vel = motion.velocity.x;
	float& y_vel = motion.velocity.y;
	// Temporary implementation
	// Update minotaur speed if the spell is active
	float accel = spellbook[1]["active"] == "true" ?		60 : 30;
	float max_vel = spellbook[1]["active"] == "true" ?		500 : 200;
	float slow_factor = spellbook[1]["active"] == "true" ?	20 : 10;
	if (!registry.view<DeathTimer>().contains(player)) {
		if (move_right)		{ x_vel += accel; }
		else if (move_left) { x_vel += -1 * accel; }
		if (move_up)		{ y_vel += -1 * accel; }
		else if (move_down) { y_vel += accel; }
	}

	// Impose max velocity for minotaur
	if (x_vel > max_vel) { x_vel = max_vel; }
	else if (x_vel < -1 * max_vel) { x_vel = -1 * max_vel; }
	if (y_vel > max_vel) { y_vel = max_vel; }
	else if (y_vel < -1 * max_vel) { y_vel = -1 * max_vel; }


	// Friction to slow minotaur down
	
	if (x_vel != 0 || y_vel != 0) {
		if (x_vel > 0) { x_vel -= slow_factor; }
		else if (x_vel < 0) { x_vel += slow_factor; }
		if (y_vel > 0) { y_vel -= slow_factor; }
		else if (y_vel < 0) { y_vel += slow_factor; }
	}

	// Check for collisions between all moving entities
	for(entt::entity entity : registry.view<Motion>())
	{
		Motion motion = registry.get<Motion>(entity);
		for(entt::entity other : registry.view<Motion>()) // i+1
		{
			if (entity == other)
				continue;

			Motion motion_other = registry.get<Motion>(other);
			if (collides(motion, motion_other))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.emplace<Collision>(entity, other);
				//registry.emplace<Collision>(other, entity);
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
			//vec2 line_scale1 = { motion_i.scale.x / 10, 2*radius };
			//entt::entity line1 = createLine(motion_i.position, line_scale1);
			//vec2 line_scale2 = { 2*radius, motion_i.scale.x / 10};
			//entt::entity line2 = createLine(motion_i.position, line_scale2);
		}
	}
}