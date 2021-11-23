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
entt::entity createChick(RenderSystem* renderer, vec2 position)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = mesh.original_size * 25.f * global_scaling_vector;
	motion.mass = 50;
	motion.coeff_rest = 0.9f;
	const entt::entity e = registry.create();
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::CHICK,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE);
	Prey prey = Prey();
	prey.id = chick_ai.size();
	registry.emplace<Prey>(e, prey);
	ChickAI ai = ChickAI(e, prey.id);
	chick_ai.push_back(ai);
	return e;
}
entt::entity createCutscene(RenderSystem* renderer, vec2 position, Cutscene_enum element)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	Cutscene cutscene = Cutscene();
	motion.angle = 0.f;
	motion.velocity = {0,0};
	motion.position = position;
	motion.scale = mesh.original_size * 0.f * global_scaling_vector;
	motion.mass = 0;
	motion.coeff_rest = 0;
	motion.can_collide = false;
	const entt::entity e = registry.create();
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<Cutscene>(e, cutscene);


	TEXTURE_ASSET_ID texture_asset_id;
	switch (element)
	{
	case MINOTAUR:
		texture_asset_id = TEXTURE_ASSET_ID::CUTSCENE_MINOTAUR;
		break;
	case DRONE:
		texture_asset_id = TEXTURE_ASSET_ID::CUTSCENE_DRONE;
		break;
	case DRONE_SAD:
		texture_asset_id = TEXTURE_ASSET_ID::CUTSCENE_DRONE_SAD;
		break;
	case DRONE_LAUGHING:
		texture_asset_id = TEXTURE_ASSET_ID::CUTSCENE_DRONE_LAUGHING;
		break;
	case MINOTAUR_RTX_OFF:
		texture_asset_id = TEXTURE_ASSET_ID::CUTSCENE_MINOTAUR_RTX_OFF;
		break;
	case DRONE_RTX_OFF:
		texture_asset_id = TEXTURE_ASSET_ID::CUTSCENE_DRONE_RTX_OFF;
		break;
	default:
		break;
	}
	registry.emplace<RenderRequest>(e,
		texture_asset_id,
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

entt::entity createItem(RenderSystem* renderer, vec2 pos, std::string item_type)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 40.f * global_scaling_vector;
	
	Item item = Item();
	item.name = item_type;
	TEXTURE_ASSET_ID texture_type;
	ItemType item_enum = item_to_enum[item_type];
	switch (item_enum) {
		case ItemType::WALL_BREAKER:
			texture_type = TEXTURE_ASSET_ID::WALL_BREAKER;
			item.duration_ms = 20000;
			break;
		case ItemType::EXTRA_LIFE:
			texture_type = TEXTURE_ASSET_ID::EXTRA_LIFE;
			break;
		case ItemType::TELEPORT:
			texture_type = TEXTURE_ASSET_ID::TELEPORT;
			break;
		case ItemType::SPEED_BOOST:
			texture_type = TEXTURE_ASSET_ID::SPEED_BOOST;
			item.duration_ms = 10000;
			break;
		default:
			// unsupported item
			assert(false);
			break;
	}
	// vary depending on which item
	const entt::entity e = registry.create();
	registry.emplace<Item>(e, item);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		texture_type, // TEXTURE_COUNT indicates that no texture is needed
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE);

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