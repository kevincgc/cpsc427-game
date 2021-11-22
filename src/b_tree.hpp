#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <entt.hpp>
#include <algorithm>
#include "common.hpp"
#include "components.hpp"
#include "world_system.hpp"

enum class Direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

enum class BTState {
	Running,
	Success,
	Failure
};

class BTNode {
public:
	virtual void init(entt::entity e) {};

	virtual BTState process(entt::entity e) = 0;
};

class BTConditional : public BTNode
{
public:
	BTConditional(BTNode* child, std::function<bool(entt::entity)> condition)
		: m_child(child), m_condition(condition) {
	}

	virtual void init(entt::entity e) override {
		m_child->init(e);
	}

	virtual BTState process(entt::entity e) override {
		if (m_condition(e))
			return m_child->process(e);
		else
			return BTState::Success;
	}

private:
	BTNode* m_child;
	std::function<bool(entt::entity)> m_condition;
};

class BTSequence : public BTNode {
private:
	int m_index;
	std::vector<BTNode*> m_children;

public:
	BTSequence(std::vector<BTNode*> children) : m_index(0), m_children(children){
	}

	void init(entt::entity e) override
	{
		m_index = 0;
		const auto& child = m_children[m_index];
		child->init(e);
	}

	BTState process(entt::entity e) override {
		if (m_index >= m_children.size())
			return BTState::Success;

		BTState state = m_children[m_index]->process(e);

		if (state == BTState::Success) {
			m_index++;
			if (m_index >= m_children.size())
				return BTState::Success;
			else {
				m_children[m_index]->init(e);
				return BTState::Running;
			}
		}
		else {
			return state;
		}
	}
};

class BTSelector : public BTNode {
private:
	int m_index;
	std::vector<BTNode*> m_children;

public:
	BTSelector(std::vector<BTNode*> children) : m_index(0), m_children(children) {
	}

	void init(entt::entity e) override
	{
		m_index = 0;
		const auto& child = m_children[m_index];
		child->init(e);
	}

	BTState process(entt::entity e) override {
		if (m_index >= m_children.size())
			return BTState::Failure;

		BTState state = m_children[m_index]->process(e);

		if (state == BTState::Failure) {
			m_index++;
			if (m_index >= m_children.size())
				return BTState::Failure;
			else {
				m_children[m_index]->init(e);
				return BTState::Running;
			}
		}
		else {
			return state;
		}
	}
};

class BTInverter : BTNode {
	BTNode* m_child;
public:
	BTInverter(BTNode* child): m_child(child) {
	}

	void init(entt::entity e) override
	{
		m_child->init(e);
	}

	BTState process(entt::entity e) override {
		BTState state = m_child->process(e);

		if (state == BTState::Failure) {
			return BTState::Success;
		}
		else if (state == BTState::Success) {
			return BTState::Failure;
		}
		else {
			return state;
		}
	}
};

class BTSucceeder : BTNode {
	BTNode* m_child;
public:
	BTSucceeder(BTNode* child) : m_child(child) {
	}

	void init(entt::entity e) override
	{
		m_child->init(e);
	}

	BTState process(entt::entity e) override {
		BTState state = m_child->process(e);

		if (state == BTState::Running) {
			return state;
		}
		else {
			return BTState::Success;
		}
	}
};

class BTRepeatUntilSuccess : BTNode {
	BTNode* m_child;
public:
	BTRepeatUntilSuccess(BTNode* child) : m_child(child) {
	}

	void init(entt::entity e) override
	{
		m_child->init(e);
	}

	BTState process(entt::entity e) override {
		BTState state = m_child->process(e);

		if (state == BTState::Success) {
			return state;
		}
		else if (state == BTState::Failure) {
			m_child->init(e);
		}
		return BTState::Running;
	}
};

class BTRepeatUntilFailure : BTNode {
	BTNode* m_child;
public:
	BTRepeatUntilFailure(BTNode* child) : m_child(child) {
	}

	void init(entt::entity e) override
	{
		m_child->init(e);
	}

	BTState process(entt::entity e) override {
		BTState state = m_child->process(e);

		if (state == BTState::Failure) {
			return state;
		}
		else if (state == BTState::Success) {
			m_child->init(e);
		}
		return BTState::Running;
	}
};

class BTRepeat : BTNode {
	BTNode* m_child;
public:
	BTRepeat(BTNode* child) : m_child(child) {
	}

	void init(entt::entity e) override
	{
		m_child->init(e);
	}

	BTState process(entt::entity e) override {
		BTState state = m_child->process(e);

		if (state == BTState::Success) {
			m_child->init(e);
		}
		else if (state == BTState::Failure) {
			m_child->init(e);
		}
		return BTState::Running;
	}
};

class MoveOneTile : public BTNode {
	vec2 current_tile;
	vec2 target_tile;
	Direction d;
	vec2 last_pos;
public:
	MoveOneTile(Direction d) : d(d) {
	}

	void init(entt::entity e) override {
		current_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		switch (d) {
		case Direction::UP:
			target_tile = current_tile + vec2{ 0, -1 };
			break;
		case Direction::DOWN:
			target_tile = current_tile + vec2{ 0, 1 };
			break;
		case Direction::LEFT:
			target_tile = current_tile + vec2{ -1, 0 };
			break;
		case Direction::RIGHT:
			target_tile = current_tile + vec2{ 1, 0 };
			break;
		}
		//printf("current tile: %f, %f\n", current_tile[0], current_tile[1]);
		//printf("target tile: %f, %f\n", target_tile[0], target_tile[1]);
		last_pos = { -1,-1 };
	}

	bool is_approx_eq(float f1, float f2) {
		return abs(f1 - f2) < 75;
	}

	BTState process(entt::entity e) override {
		current_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		Motion& motion = registry.get<Motion>(e);
		if (current_tile == target_tile) {
			float x = motion.position.x - floor(motion.position.x / map_scale.x) * map_scale.x;
			float y = motion.position.y - floor(motion.position.y / map_scale.y) * map_scale.y;
			if (is_approx_eq(x, map_scale.x/2) && is_approx_eq(y, map_scale.y / 2)) {
				motion.velocity = { 0, 0 };
				return BTState::Success;
			}
		}
		if (motion.position == last_pos && !(state == ProgramState::PAUSED)) {
			return BTState::Failure;
		}
		last_pos = motion.position;
		if (!WorldSystem::tile_is_walkable(WorldSystem::get_map_tile(target_tile)) || !WorldSystem::is_within_bounds(target_tile)) {
			return BTState::Failure;
		}
		switch (d) {
		case Direction::UP:
			motion.velocity = { 0, -200 };
			break;
		case Direction::DOWN:
			motion.velocity = { 0, 200 };
			break;
		case Direction::LEFT:
			motion.velocity = { -200, 0 };
			break;
		case Direction::RIGHT:
			motion.velocity = { 200, 0 };
			break;
		}
		return BTState::Running;
	}
};

class BTRandomSelector : public BTNode {
private:
	int m_index;
	std::vector<BTNode*> m_children;
	std::vector<int> randomized_indexes;

public:
	BTRandomSelector(std::vector<BTNode*> children) : m_index(0), m_children(children){
	}

	void reset() {
		randomized_indexes.clear();
		for (int i = 0; i < m_children.size(); i++) {
			randomized_indexes.push_back(i);
		}
		for (int i = 0; i < m_children.size(); i++) {
			std::swap(randomized_indexes[i], randomized_indexes[rand() % m_children.size()]);
		}
	}

	void init(entt::entity e) override
	{
		reset();
		printf("init\n");
		m_index = 0;
		const auto& child = m_children[randomized_indexes[m_index]];
		child->init(e);
	}

	BTState process(entt::entity e) override {
		if (m_index >= m_children.size())
			return BTState::Failure;

		BTState state = m_children[randomized_indexes[m_index]]->process(e);

		if (state == BTState::Failure) {
			m_index++;
			if (m_index >= m_children.size())
				return BTState::Failure;
			else {
				m_children[m_index]->init(e);
				return BTState::Running;
			}
		}
		else {
			return state;
		}
	}
};

class CheckIsClose : public BTNode {
private:
	void init(entt::entity e) override {
	}

	bool is_close(entt::entity e) {
		entt::entity player_entity = registry.view<Player>().begin()[0];
		Motion& player_motion = registry.get<Motion>(player_entity);
		Motion& motion = registry.get<Motion>(e);
		vec2 difference = motion.position - player_motion.position;
		float distance = sqrt(dot(difference, difference));
		return distance <= 150;
	}

	BTState process(entt::entity e) override {
		return is_close(e) ? BTState::Failure : BTState::Success;
	}
};

class AStarSearch {
	class Node {
	public:
		vec2 coord;
		Node* parent;
		float g; // dist from start
		float h; // dist to target
		float f; // g + h
		Node(vec2 c) : coord(c), parent(nullptr), f(0), g(0), h(0) { }
		//Node(vec2 c, vec2 p, float f_) : coord(c), parent(p), f(f_) { }
		Node(vec2 c, Node* p, float f_, float g_, float h_) : coord(c), parent(p), f(f_), g(g_), h(h_) { }
	};
	std::vector<vec2> path;
	std::vector<Node> closed;
	std::vector<Node> open;
	vec2 start;
	vec2 target;
	std::vector<int> x_adj = { 1, -1, 0, 0 };
	std::vector<int> y_adj = { 0, 0, 1, -1 };

	bool eq(float f1, float f2) {
		return abs(f1 - f2) < 1e-6;
	}

	// Manhattan distance to calculate h cost
	int get_cost(vec2 target, vec2 current) {
		return abs(target[0] - current[0]) + abs(target[1] - current[1]);
	}

	void trace_path(Node& node) {
		path.push_back(node.coord);
		Node* parent = node.parent;
		while (parent != nullptr) {
			path.push_back(parent->coord);
			parent = parent->parent;
		}
		path.pop_back();
		//std::reverse(path.begin(), path.end());
	}

	void search() {
		open.push_back(Node(start));
		while (open.size() > 0) {
			float min_f = FLT_MAX;
			int min_f_index;
			for (int i = 0; i < open.size(); i++) {
				if (open[i].f < min_f) {
					min_f_index = i;
					min_f = open[i].f;
				}
			}
			Node& parent = open[min_f_index];
			closed.push_back(parent);
			open.erase(open.begin() + min_f_index);
			if (parent.coord == target) {
				trace_path(parent);
				return;
			}
			for (int i = 0; i < x_adj.size(); i++) {
				vec2 coordinates = { parent.coord.x + x_adj[i], parent.coord.y + y_adj[i] };
				if (WorldSystem::is_within_bounds(coordinates)) {
					bool found = false;
					for (auto& e : closed) {
						if (e.coord == coordinates) {
							found = true;
						}
					}
					if (found)
						continue;
					Node* found_node;
					for (auto& e : open) {
						if (e.coord == coordinates) {
							found = true;
							found_node = &e;
						}
					}
					float h_cost = get_cost(coordinates, target);
					float g_cost = parent.g + 1;
					float f_cost = h_cost + g_cost;
					if (!found) {
						open.push_back(Node(coordinates, &parent, f_cost, g_cost, h_cost));
					}
					else {
						if (f_cost < found_node->f) {
							found_node->f = f_cost;
							found_node->g = g_cost;
							found_node->h = h_cost;
							found_node->parent = &parent;
						}
					}
				}
			}
		}
	}

	void init(vec2 t, vec2 s) {
		path.clear();
		closed.clear();
		open.clear();
		target = t;
		start = s;
	}

public:
	std::vector<vec2> get_path(vec2 t, vec2 s) {
		if ((!(target == t) || !(start == s))) {
			init(t, s);
			search();
		}
		return path;
	}
};

class Escape : public BTNode {
	std::vector<vec2> path;
	vec2 current_target;
	MoveOneTile move;
	vec2 get_target_tile(entt::entity e) {
		size_t max_size = game_state.level.map_tiles.size();
		entt::entity player_entity = registry.view<Player>().begin()[0];
		Motion& player_motion = registry.get<Motion>(player_entity);
		Motion& motion = registry.get<Motion>(e);
		bool is_x_greater = motion.position.x - player_motion.position.x > 0;
		bool is_y_greater = motion.position.y - player_motion.position.y > 0;
		int x = is_x_greater ? max_size : 0;
		int y = is_y_greater ? max_size : 0;
		for (int i = 0; i < max_size; i++) {
			for (int j = 0; j < max_size; j++) {
				if (!WorldSystem::is_within_bounds({ i,j }))
					continue;
				if (WorldSystem::tile_is_walkable(WorldSystem::get_map_tile({ i,j }))) {
					return { i,j };
				}
			}
		}
		return { -1,-1 };
	}

public:
	Escape() : move(Direction::LEFT){}
	void init(entt::entity e) override {
		vec2 target_tile = get_target_tile(e);
		vec2 starting_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		AStarSearch search;
		path = search.get_path(target_tile, starting_tile);
		current_target = { -1,-1 };
	}

	BTState process(entt::entity e) override {
		if (path.size() == 0) {
			return BTState::Success;
		}
		vec2 current_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		if (current_target == vec2{ -1,-1 } || current_tile == current_target) {
			current_target = path.back();
			path.pop_back();
			vec2 diff = current_target - current_tile;
			if (diff[0] == 1 && diff[1] == 0) {
				move = MoveOneTile(Direction::RIGHT);
			}
			else if (diff[0] == -1 && diff[1] == 0) {
				move = MoveOneTile(Direction::LEFT);
			}
			else if (diff[0] == 0 && diff[1] == 1) {
				move = MoveOneTile(Direction::DOWN);
			}
			else if (diff[0] == 0 && diff[1] == -1) {
				move = MoveOneTile(Direction::UP);
			}
			else {
				move = MoveOneTile(Direction::UP);
				printf("Escape error\n");
			}
		}
		move.process(e);
		return BTState::Running;
	}
};

class ChickAI {
private:
	MoveOneTile move_up{ Direction::UP };
	MoveOneTile move_down{ Direction::DOWN };
	MoveOneTile move_left{ Direction::LEFT };
	MoveOneTile move_right{ Direction::RIGHT };
	BTRandomSelector* select_random_dir;
	Escape* escape;
	CheckIsClose* check_dist;
	BTSequence* roam;
	BTSelector* roam_or_escape;
	BTRepeat* root;
	entt::entity e;
	bool is_init;

public:
	//ChickAI(const entt::entity e): e(e),
	//	escape(),
	//	move_up(MoveOneTile::Direction::UP), 
	//	move_down(MoveOneTile::Direction::DOWN),
	//	move_left(MoveOneTile::Direction::LEFT),
	//	move_right(MoveOneTile::Direction::RIGHT),
	//	select_random_dir(vector<BTNode*>{&move_up, &move_down, &move_left, &move_right}),
	//	check_dist(),
	//	roam(vector<BTNode*>{&check_dist, &select_random_dir}),
	//	roam_or_escape(vector<BTNode*>{&roam, & escape}),
	//	root(&roam_or_escape),
	//	is_init(false) {
	//}

	ChickAI(const entt::entity e) : e(e), is_init(false) {
	}

	void step() {
		if (!is_init) {
			check_dist = new CheckIsClose();
			select_random_dir = new BTRandomSelector{ std::vector<BTNode*>{&move_up,& move_down,& move_left,& move_right} };
			roam = new BTSequence{ std::vector<BTNode*>{check_dist,select_random_dir} };
			//escape = new Escape();
			//roam_or_escape = new BTSelector{ std::vector<BTNode*>{ roam,escape };
			root = new BTRepeat(roam);
			root->init(e);
			is_init = true;
		}
		BTState state = root->process(e);
	}

	void clear() {
		delete select_random_dir;
		delete root;
		//delete roam_or_escape;
		delete check_dist;
		//delete escape;
		delete roam;
	}
};

