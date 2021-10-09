// internal
#include "ai_system.hpp"




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
				entity_motion.position += entity_motion.velocity * step_seconds;
			}
			else {
				entity_motion.velocity = entity_velocity;
				entity_motion.position += entity_velocity * step_seconds * 4.f;
			}
		}
		
	}

}