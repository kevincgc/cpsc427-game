#include "world_init.hpp"
#include <iostream>

entt::entity createSpike(RenderSystem* renderer, vec2 position)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ENEMY);
	Motion motion = Motion();
	motion.angle = 0.f;
	motion.velocity = { -50.f * global_scaling_vector.x, 0.f * global_scaling_vector.y };
	motion.position = position;
	motion.scale = mesh.original_size * 400.f * global_scaling_vector;
	motion.scale.y *= -1.0;
	motion.mass = 200;
	motion.coeff_rest = 0.9f;
	const entt::entity e = registry.create();
	registry.emplace<Enemy>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::SPIKE,
		EFFECT_ASSET_ID::ENEMY,
		GEOMETRY_BUFFER_ID::ENEMY);

	// Debug
	std::cout << "Entity [" << (int)e << "] is spike with starting pos " << position.x << ", " << position.y << std::endl;

	return e;
}

entt::entity createDrone(RenderSystem* renderer, vec2 position)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DRONE);
	Motion motion = Motion();
	motion.angle = 0.f;
	motion.velocity = { -100.f * global_scaling_vector.x, 0.f * global_scaling_vector.y };
	motion.position = position;
	motion.scale = mesh.original_size * 400.f * global_scaling_vector;
	motion.scale.y *= -1;
	motion.mass = 200;
	motion.coeff_rest = 0.9f;
	const entt::entity e = registry.create();
	registry.emplace<Enemy>(e);
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<RenderRequest>(e,
		TEXTURE_ASSET_ID::DRONE,
		EFFECT_ASSET_ID::ENEMY,
		GEOMETRY_BUFFER_ID::DRONE);

	// Debug
	std::cout << "Entity [" << (int)e << "] is drone with starting pos " << position.x << ", " << position.y << std::endl;

	return e;
}

entt::entity createChick(RenderSystem* renderer, vec2 position)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = mesh.original_size * 20.f * global_scaling_vector;
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

entt::entity createCutscene(RenderSystem* renderer, Cutscene_enum element)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	Cutscene cutscene = Cutscene();
	motion.angle = 0.f;
	motion.velocity = {0,0};
	motion.position = {0,0};
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

	// Debug
	std::cout << "Entity [" << (int)e << "] is cutscene element " << element << std::endl;

	return e;
}

entt::entity createBackground(RenderSystem* renderer, vec2 position, int element) {
	// Set up handles
	Mesh& mesh			  = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion		  = Motion();
	Background background = Background();

	// Set initial motion values
	motion.angle	   = 0.f;
	motion.velocity    = { 0,0 };
	motion.position	   = { position.x * global_scaling_vector.x, position.y * global_scaling_vector.y };
	//motion.scale	   = mesh.original_size * 3000.f * global_scaling_vector;
	motion.mass		   = 0;
	motion.coeff_rest  = 0;
	motion.can_reflect = false;
	motion.can_collide = false;
	motion.can_be_attacked = false;

	// Set texture_asset_id
	TEXTURE_ASSET_ID texture_asset_id;
	switch (element) {
		case 1:
			texture_asset_id = TEXTURE_ASSET_ID::BACKGROUND_SPACE1;
			motion.scale = mesh.original_size * 6000.f * global_scaling_vector;
			break;
		case 2:
			texture_asset_id = TEXTURE_ASSET_ID::BACKGROUND_SPACE2;
			motion.scale = mesh.original_size * 4000.f * global_scaling_vector;
			break;
		case 3:
			texture_asset_id = TEXTURE_ASSET_ID::BACKGROUND_SPACE2;
			motion.scale = mesh.original_size * 4500.f * global_scaling_vector;
			break;
		case 4:
			texture_asset_id = TEXTURE_ASSET_ID::BACKGROUND_MOON;
			motion.scale = mesh.original_size * 500.f * global_scaling_vector;
			break;
		case 5:
			texture_asset_id = TEXTURE_ASSET_ID::BACKGROUND_SATELLITE;
			motion.scale = mesh.original_size * 200.f * global_scaling_vector;
			break;
		case 10:
			texture_asset_id = TEXTURE_ASSET_ID::HUD_HEART;
			motion.scale = mesh.original_size * 100.f * global_scaling_vector;
		default:
			break;
	}

	// Create and emplace entity
	const entt::entity e = registry.create();
	registry.emplace<Motion>	   (e, motion);
	registry.emplace<Mesh*>		   (e, &mesh);
	registry.emplace<Background>   (e, background);
	registry.emplace<RenderRequest>(e, texture_asset_id, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE);

	// Debug
	std::cout << "Entity [" << (int)e << "] is background element " << element << std::endl;

	return e;
}

entt::entity createHUD(RenderSystem* renderer, int element) {
	// Set up handles
	Mesh& mesh	  = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion motion = Motion();
	HUD hud		  = HUD();

	// Set initial motion values
	motion.angle		   = 0.f;
	motion.velocity		   = { 0,0 };
	motion.position		   = { 0,0 };
	motion.mass			   = 0;
	motion.coeff_rest      = 0;
	motion.can_reflect     = false;
	motion.can_collide     = false;
	motion.can_be_attacked = false;

	// Set texture_asset_id
	TEXTURE_ASSET_ID texture_asset_id;
	switch (element) {
	case 1: // Heart
		texture_asset_id = TEXTURE_ASSET_ID::HUD_HEART;
		motion.scale = mesh.original_size * 50.f * global_scaling_vector;
		break;
	case 2: // Background
		texture_asset_id = TEXTURE_ASSET_ID::HUD_BACKGROUND;
		motion.scale.x = mesh.original_size.x * 400.f * global_scaling_vector.x;
		motion.scale.y = mesh.original_size.y * 200.f * global_scaling_vector.y;
		break;
	case 3: // Hammer
		texture_asset_id = TEXTURE_ASSET_ID::WALL_BREAKER;
		motion.scale = mesh.original_size * 50.f * global_scaling_vector;
		break;
	case 4: // Teleport
		texture_asset_id = TEXTURE_ASSET_ID::TELEPORT;
		motion.scale = mesh.original_size * 50.f * global_scaling_vector;
		break;
	case 5: // Speed
		texture_asset_id = TEXTURE_ASSET_ID::SPEED_BOOST;
		motion.scale = mesh.original_size * 50.f * global_scaling_vector;
		break;
	case 6: // No Hammer
		texture_asset_id = TEXTURE_ASSET_ID::NO_HAMMER;
		motion.scale = mesh.original_size * 50.f * global_scaling_vector;
		break;
	case 7: // No Teleport
		texture_asset_id = TEXTURE_ASSET_ID::NO_TELEPORT;
		motion.scale = mesh.original_size * 50.f * global_scaling_vector;
		break;
	case 8: // No Speed
		texture_asset_id = TEXTURE_ASSET_ID::NO_SPEEDBOOST;
		motion.scale = mesh.original_size * 50.f * global_scaling_vector;
		break;
	case 9: // Key
		texture_asset_id = TEXTURE_ASSET_ID::KEY;
		motion.scale = mesh.original_size * 120.f * global_scaling_vector;
		break;
	case 10: // No Key
		texture_asset_id = TEXTURE_ASSET_ID::NO_KEY;
		motion.scale = mesh.original_size * 120.f * global_scaling_vector;
		break;
	default:
		break;
	}

	// Create and emplace entity
	const entt::entity e = registry.create();
	registry.emplace<Motion>(e, motion);
	registry.emplace<Mesh*>(e, &mesh);
	registry.emplace<HUD>(e, hud);
	registry.emplace<RenderRequest>(e, texture_asset_id, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE);

	// Debug
	std::cout << "Entity [" << (int)e << "] is hud element " << element << std::endl;

	return e;
}

entt::entity createMinotaur(RenderSystem* renderer, vec2 pos)
{
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	Motion motion = Motion();
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 60.f * global_scaling_vector;
	motion.scale.x *= 1.5f;
	motion.mass = 120.f;
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

	Item item = Item();
	item.name = item_type;
	TEXTURE_ASSET_ID texture_type;
	ItemType item_enum = item_to_enum[item_type];
	switch (item_enum) {
		case ItemType::WALL_BREAKER:
			texture_type = TEXTURE_ASSET_ID::WALL_BREAKER;
			item.duration_ms = 20000;
			motion.scale = mesh.original_size * 40.f * global_scaling_vector;
			break;
		case ItemType::KEY: //Key
			texture_type = TEXTURE_ASSET_ID::KEY;
			motion.scale = mesh.original_size * 80.f * global_scaling_vector;
			break;
		case ItemType::TELEPORT:
			texture_type = TEXTURE_ASSET_ID::TELEPORT;
			motion.scale = mesh.original_size * 40.f * global_scaling_vector;
			break;
		case ItemType::SPEED_BOOST:
			texture_type = TEXTURE_ASSET_ID::SPEED_BOOST;
			item.duration_ms = 10000;
			motion.scale = mesh.original_size * 40.f * global_scaling_vector;
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
	motion.scale = mesh.original_size * 75.f * global_scaling_vector;
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

entt::entity createLine(vec2 position, vec2 scale)
{
	const entt::entity e = registry.create();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.emplace<RenderRequest>(e,
		 TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::PEBBLE,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE);

	// Create motion
	Motion& motion = registry.emplace<Motion>(e);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale * global_scaling_vector;

	registry.emplace<DebugComponent>(e);
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
