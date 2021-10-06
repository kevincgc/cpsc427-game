#include "world_init.hpp"

entt::entity createSalmon(RenderSystem* renderer, vec2 pos)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	Motion motion = Motion();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 150.f;
	motion.scale.x *= -1;

	const entt::entity e = registry.create();
	registry.emplace<Player>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
			TEXTURE_ASSET_ID::MINOTAUR, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::SALMON, // was TEXTURED
			GEOMETRY_BUFFER_ID::SPRITE);

	return e;
}

entt::entity createFish(RenderSystem* renderer, vec2 position)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	motion.angle = 0.f;
	motion.velocity = { -50, 0 };
	motion.position = position;
	motion.scale = vec2({ -FISH_BB_WIDTH, FISH_BB_HEIGHT });

	const entt::entity e = registry.create();
	registry.emplace<SoftShell>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::FISH,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE);

	return e;
}

entt::entity createTurtle(RenderSystem* renderer, vec2 position)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	motion.angle = 0.f;
	motion.velocity = { -100.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ -TURTLE_BB_WIDTH, TURTLE_BB_HEIGHT });

	const entt::entity e = registry.create();
	registry.emplace<HardShell>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::TURTLE,
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
	motion.scale = mesh.original_size * 150.f;
	motion.scale.x *= -1;

	const entt::entity e = registry.create();
	registry.emplace<Player>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
		EFFECT_ASSET_ID::MINOTAUR,
		GEOMETRY_BUFFER_ID::MINOTAUR);

	return e;
}

entt::entity createEnemy(RenderSystem* renderer, vec2 pos)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	Motion motion = Motion();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 150.f;
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

//entt::entity createLine(vec2 position, vec2 scale)
//{
//	Entity entity = Entity();
//
//	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
//	registry.renderRequests.insert(
//		entity,
//		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
//		 EFFECT_ASSET_ID::PEBBLE,
//		 GEOMETRY_BUFFER_ID::DEBUG_LINE });
//
//	// Create motion
//	Motion& motion = registry.motions.emplace(entity);
//	motion.angle = 0.f;
//	motion.velocity = { 0, 0 };
//	motion.position = position;
//	motion.scale = scale;
//
//	registry.debugComponents.emplace(entity);
//	return entity;
//}

//entt::entity createPebble(vec2 pos, vec2 size)
//{
//	auto entity = Entity();
//
//	// Setting initial motion values
//	Motion& motion = registry.motions.emplace(entity);
//	motion.position = pos;
//	motion.angle = 0.f;
//	motion.velocity = { 0.f, 0.f };
//	motion.scale = size;
//
//	// Create and (empty) Salmon component to be able to refer to all turtles
//	registry.players.emplace(entity);
//	registry.renderRequests.insert(
//		entity,
//		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
//			EFFECT_ASSET_ID::PEBBLE,
//			GEOMETRY_BUFFER_ID::PEBBLE });
//
//	return entity;
//}