#pragma once

#include "common.hpp"
//#include "tiny_ecs.hpp"
#include <entt.hpp>
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"

extern entt::registry registry;

// the prey
entt::entity createSpike(RenderSystem* renderer, vec2 position);
// the enemy
entt::entity createDrone(RenderSystem* renderer, vec2 position);

// New Entities
entt::entity createMinotaur(RenderSystem* renderer, vec2 position);

entt::entity createEnemy(RenderSystem* renderer, vec2 position);

entt::entity createItem(RenderSystem* renderer, vec2 position);

entt::entity createTrap(RenderSystem* renderer, vec2 position);

entt::entity createChick(RenderSystem* renderer, vec2 position);


