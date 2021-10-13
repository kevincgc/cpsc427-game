#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
// const float FISH_BB_WIDTH = 0.4f * 296.f;
// const float FISH_BB_HEIGHT = 0.4f * 165.f;
// const float TURTLE_BB_WIDTH = 0.4f * 300.f;
// const float TURTLE_BB_HEIGHT = 0.4f * 202.f;
const float FISH_BB_WIDTH = 0.4f * 200.f;
const float FISH_BB_HEIGHT = 0.4f * 200.f;
const float TURTLE_BB_WIDTH = 0.4f * 200.f;
const float TURTLE_BB_HEIGHT = 0.4f * 200.f;
extern entt::registry registry;

// the player
// entt::entity createSalmon(RenderSystem* renderer, vec2 pos);
// the prey
Entity createFish(RenderSystem* renderer, vec2 position);
// the enemy
entt::entity createTurtle(RenderSystem* renderer, vec2 position);

// New Entities
entt::entity createMinotaur(RenderSystem* renderer, vec2 position);

entt::entity createEnemy(RenderSystem* renderer, vec2 position);

entt::entity createItem(RenderSystem* renderer, vec2 position);

entt::entity createTrap(RenderSystem* renderer, vec2 position);
