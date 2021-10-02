#pragma once

#include "common.hpp"
//#include "tiny_ecs.hpp"
#include <entt.hpp>
#include "render_system.hpp"
#include "world_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;
extern entt::registry registry;

// the player
entt::entity createSalmon(RenderSystem* renderer, vec2 pos);
// the prey
entt::entity createFish(RenderSystem* renderer, vec2 position);
// the enemy
entt::entity createTurtle(RenderSystem* renderer, vec2 position);

// New Entities
entt::entity createMinotaur(RenderSystem* renderer, vec2 position);

entt::entity createEnemy(RenderSystem* renderer, vec2 position);

entt::entity createItem(RenderSystem* renderer, vec2 position);

entt::entity createTrap(RenderSystem* renderer, vec2 position);
//// a red line for debugging purposes
//entt::entity createLine(vec2 position, vec2 size);
//// a pebble
//entt::entity createPebble(vec2 pos, vec2 size);


