#include "world_init.hpp"

// entt::entity createSalmon(RenderSystem* renderer, vec2 pos)
// {
// 	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
// 	Motion motion = Motion();
// 	motion.position = pos;
// 	motion.angle = 0.f;
// 	motion.velocity = { 0.f, 0.f };
// 	motion.scale = mesh.original_size * 75.f;
// 	motion.scale.x *= 1.5;

// 	const entt::entity e = registry.create();
// 	registry.emplace<Player>(e);
// 	registry.emplace<Motion>(e, motion);
// 	registry.emplace<Mesh*>(e, &mesh);
// 	registry.emplace<RenderRequest>(e,
// 			TEXTURE_ASSET_ID::MINOTAUR, // TEXTURE_COUNT indicates that no texture is needed
// 			EFFECT_ASSET_ID::SALMON, // TEXTURED
// 			GEOMETRY_BUFFER_ID::SALMON);
// 	return e;
// }

entt::entity createSpike(RenderSystem* renderer, vec2 position)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	motion.angle = 0.f;
	motion.velocity = { -50.f * global_scaling_vector.x, 0.f * global_scaling_vector.y };
	motion.position = position;
	motion.scale = mesh.original_size * 75.f * global_scaling_vector;
	motion.mass = 200;
	motion.coeff_rest = 0.9f;
	const entt::entity e = registry.create();
	registry.emplace<Enemy>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::SPIKE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE);

	return e;
}

entt::entity createDrone(RenderSystem* renderer, vec2 position)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	motion.angle = 0.f;
	motion.velocity = { -100.f * global_scaling_vector.x, 0.f * global_scaling_vector.y };
	motion.position = position;
	motion.scale = mesh.original_size * 60.f * global_scaling_vector;
	motion.mass = 200;
	motion.coeff_rest = 0.9f;
	const entt::entity e = registry.create();
	registry.emplace<Enemy>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::DRONE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE);

	return e;
}

// New Entities

entt::entity createMinotaur(RenderSystem* renderer, vec2 pos)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	Motion motion = Motion();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 60.f * global_scaling_vector;
	motion.scale.x *= 1.5f;
	const entt::entity e = registry.create();
	registry.emplace<Player>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
			TEXTURE_ASSET_ID::MINOTAUR, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::MINOTAUR, // SALMON
			GEOMETRY_BUFFER_ID::MINOTAUR); //SALMON
	return e;

}

entt::entity createEnemy(RenderSystem* renderer, vec2 pos)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	Motion motion = Motion();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 75.f;
	motion.scale.x *= -1;

	const entt::entity e = registry.create();
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
		EFFECT_ASSET_ID::ENEMY,
		GEOMETRY_BUFFER_ID::ENEMY);

	return e;
}

entt::entity createItem(RenderSystem* renderer, vec2 pos)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	Motion motion = Motion();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 150.f;
	motion.scale.x *= -1;

	const entt::entity e = registry.create();
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
		EFFECT_ASSET_ID::ITEM,
		GEOMETRY_BUFFER_ID::ITEM);

	return e;
}

entt::entity createTraps(RenderSystem* renderer, vec2 pos)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	Motion motion = Motion();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 150.f;
	motion.scale.x *= -1;

	const entt::entity e = registry.create();
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
		EFFECT_ASSET_ID::TRAP,
		GEOMETRY_BUFFER_ID::TRAP);

	return e;
} 