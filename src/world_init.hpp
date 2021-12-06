#pragma once

#include "common.hpp"
#include <entt.hpp>
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"

extern entt::registry registry;

// Enemies
entt::entity createSpike(RenderSystem* renderer, vec2 position);

entt::entity createDrone(RenderSystem* renderer, vec2 position);

// New Entities
entt::entity createMinotaur(RenderSystem* renderer, vec2 position);

entt::entity createEnemy(RenderSystem* renderer, vec2 position);

entt::entity createItem(RenderSystem* renderer, vec2 position, std::string item_type);

entt::entity createTrap(RenderSystem* renderer, vec2 position);

// Prey
entt::entity createChick(RenderSystem* renderer, vec2 position);

// Story progression cutscenes
entt::entity createCutscene(RenderSystem* renderer, Cutscene_enum element);

// Scrolling backgroundd
entt::entity createBackground(RenderSystem* renderer, vec2 position, int element);

// HUD for displaying inventory and health
entt::entity createHUD(RenderSystem* renderer, int element);

// A red line for debugging purposes
entt::entity createLine(vec2 position, vec2 size);
