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
	int cnt = 0;
	int speed = 600;
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
		cnt = 0;
	}

	void update_dir(Direction dir) {
		d = dir;
	}

	bool is_approx_eq(float f1, float f2) {
		return abs(f1 - f2) < 130;
	}

	BTState process(entt::entity e) override {
		current_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		Motion& motion = registry.get<Motion>(e);
		if (!(player_is_manually_moving || do_pathfinding_movement)) {
			last_pos = motion.position;
			return BTState::Running;
		}
		if (current_tile == target_tile) {
			float x = motion.position.x - floor(motion.position.x / map_scale.x) * map_scale.x;
			float y = motion.position.y - floor(motion.position.y / map_scale.y) * map_scale.y;
			if (is_approx_eq(x, map_scale.x/2) && is_approx_eq(y, map_scale.y / 2)) {
				motion.velocity = { 0, 0 };
				return BTState::Success;
			}
		}
		if (motion.position == last_pos && (player_is_manually_moving || do_pathfinding_movement)) {
			if (cnt > 10) {
				cnt = 0;
				return BTState::Failure;
			}
			cnt++;
		}
		last_pos = motion.position;
		if (!WorldSystem::tile_is_walkable(WorldSystem::get_map_tile(target_tile)) || !WorldSystem::is_within_bounds(target_tile)) {
			return BTState::Failure;
		}
		switch (d) {
		case Direction::UP:
			motion.velocity = { 0, -speed };
			break;
		case Direction::DOWN:
			motion.velocity = { 0, speed };
			break;
		case Direction::LEFT:
			motion.velocity = { -speed, 0 };
			break;
		case Direction::RIGHT:
			motion.velocity = { speed, 0 };
			break;
		}
		return BTState::Running;
	}
};

class AStarSearch {
	class Node {
	public:
		vec2 coord;
		Node* parent;
		float g; // dist from start
		float h; // dist to target
		Node(vec2 c, Node* p = nullptr) : coord(c), parent(p), g(0), h(0) { }
		float get_f() { return g + h; }
	};
	std::vector<vec2> path;
	vec2 start;
	vec2 target;
	std::vector<vec2> adj = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

	// Manhattan distance
	int get_cost(vec2 target, vec2 current) {
		return abs(target.x - current.x) + abs(target.y - current.y);
	}

	bool is_valid_tile(vec2 coord) {
		return WorldSystem::is_within_bounds(coord) && WorldSystem::tile_is_walkable(WorldSystem::get_map_tile(coord));
	}

	Node* find_node(std::vector<Node*>& nodes, vec2 coord) {
		for (auto node : nodes) {
			if (node->coord == coord) {
				return node;
			}
		}
		return nullptr;
	}

	void search()
	{
		Node* current_node = nullptr;
		std::vector<Node*> open, close;
		open.push_back(new Node(start));

		while (!open.empty()) {
			current_node = open[0];
			int current_index = 0;
			for (int i = 0; i < open.size(); i++) {
				Node* node = open[i];
				if (node->get_f() <= current_node->get_f()) {
					current_node = node;
					current_index = i;
				}
			}
			if (current_node->coord == target)
				break;
			close.push_back(current_node);
			open.erase(open.begin() + current_index);

			for (int i = 0; i < 4; ++i) {
				vec2 new_coord(current_node->coord + adj[i]);
				if (!is_valid_tile(new_coord) || find_node(close, new_coord)) {
					continue;
				}

				float f_cost = current_node->g + 1;

				Node* new_node = find_node(open, new_coord);
				if (new_node == nullptr) {
					new_node = new Node(new_coord, current_node);
					new_node->g = f_cost;
					new_node->h = get_cost(new_node->coord, target);
					open.push_back(new_node);
				}
				else if (f_cost < new_node->g) {
					new_node->parent = current_node;
					new_node->g = f_cost;
				}
			}
		}

		while (current_node != nullptr) {
			path.push_back({ current_node->coord.x, current_node->coord.y });
			current_node = current_node->parent;
		}
		//for (auto& coordinate : path) {
		//	printf("%f, %f\n", coordinate.x, coordinate.y);
		//}

		for (int i = 0; i < open.size(); i++) {
			delete open[i];
		}
		for (int i = 0; i < close.size(); i++) {
			delete close[i];
		}

	}

	void init(vec2 t, vec2 s) {
		path.clear();
		target = t;
		start = s;
	}

public:
	std::vector<vec2> get_path(vec2 t, vec2 s) {
		init(t, s);
		search();
		return path;
	}
};

class MoveToRandomTile : public BTNode {
	vec2 target_tile;
	std::vector<vec2> path;
	MoveOneTile* child;

	bool is_valid_tile(vec2 coord) {
		return WorldSystem::is_within_bounds(coord) && WorldSystem::tile_is_walkable(WorldSystem::get_map_tile(coord));
	}

	bool is_close(entt::entity e) {
		entt::entity player_entity = registry.view<Player>().begin()[0];
		Motion& player_motion = registry.get<Motion>(player_entity);
		Motion& motion = registry.get<Motion>(e);
		vec2 difference = motion.position - player_motion.position;
		float distance = sqrt(dot(difference, difference));
		return distance <= 300;
	}

public:
	MoveToRandomTile(): child(nullptr) {
	}

	void init(entt::entity e) override {
		vec2 current_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		bool is_valid = false;
		while (!is_valid) {
			vec2 new_coord = { rand() % game_state.level.map_tiles.size(), rand() % game_state.level.map_tiles.size()};
			if (is_valid_tile(new_coord)) {
				target_tile = new_coord;
				is_valid = true;
			}
		}
		AStarSearch search;
		path = search.get_path(target_tile, current_tile);
	}

	BTState process(entt::entity e) override {
		vec2 current_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		Motion& motion = registry.get<Motion>(e);
		BTState child_state = child ? child->process(e) : BTState::Success;
		if (child_state == BTState::Success && current_tile != target_tile && path.size() != 0) {
			vec2 next_tile = path.back();
			if (next_tile == current_tile) {
				path.pop_back();
				next_tile = path.back();
			}
			path.pop_back();
			vec2 diff = next_tile - current_tile;
			if (!child) {
				child = new MoveOneTile(Direction::UP);
			}
			if (diff == vec2{ 1, 0 }) {
				child->update_dir(Direction::RIGHT);
				child->init(e);
			}
			else if (diff == vec2{ -1, 0 }) {
				child->update_dir(Direction::LEFT);
				child->init(e);
			}
			else if (diff == vec2{ 0, 1 }) {
				child->update_dir(Direction::DOWN);
				child->init(e);
			}
			else if (diff == vec2{ 0, -1 }) {
				child->update_dir(Direction::UP);
				child->init(e);
			}
		}

		if (is_close(e)) {
			return BTState::Failure;
		}

		if (current_tile == target_tile && child_state == BTState::Success) {
			delete child;
			child = nullptr;
			return BTState::Success;
		}


		if (child_state == BTState::Failure || (child_state == BTState::Success && current_tile != target_tile && path.size() == 0)) {
			if ((player_is_manually_moving || do_pathfinding_movement) && state != ProgramState::PAUSED) {
				delete child;
				child = nullptr;
				init(e);
			}
		}
		return BTState::Running;
	}
};

class MoveToTargetTile : public BTNode {
	vec2 target_tile;
	std::vector<vec2> path;
	MoveOneTile* child;

	bool is_valid_tile(vec2 coord) {
		return WorldSystem::is_within_bounds(coord) && WorldSystem::tile_is_walkable(WorldSystem::get_map_tile(coord));
	}

public:
	MoveToTargetTile() : child(nullptr) {
	}

	void set_target(vec2 target) {
		target_tile = target;
	}

	void init(entt::entity e) override {
		vec2 current_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		AStarSearch search;
		path = search.get_path(target_tile, current_tile);
	}

	BTState process(entt::entity e) override {
		vec2 current_tile = WorldSystem::position_to_map_coords(registry.get<Motion>(e).position);
		Motion& motion = registry.get<Motion>(e);
		BTState child_state = child ? child->process(e) : BTState::Success;
		if (child_state == BTState::Success && current_tile != target_tile && path.size() != 0) {
			vec2 next_tile = path.back();
			if (next_tile == current_tile) {
				path.pop_back();
				next_tile = path.back();
			}
			path.pop_back();
			vec2 diff = next_tile - current_tile;
			if (!child) {
				child = new MoveOneTile(Direction::UP);
			}
			if (diff == vec2{ 1, 0 }) {
				child->update_dir(Direction::RIGHT);
				child->init(e);
			}
			else if (diff == vec2{ -1, 0 }) {
				child->update_dir(Direction::LEFT);
				child->init(e);
			}
			else if (diff == vec2{ 0, 1 }) {
				child->update_dir(Direction::DOWN);
				child->init(e);
			}
			else if (diff == vec2{ 0, -1 }) {
				child->update_dir(Direction::UP);
				child->init(e);
			}
		}
		if (current_tile == target_tile && child_state == BTState::Success) {
			delete child;
			child = nullptr;
			return BTState::Success;
		}
		if (child_state == BTState::Failure || (child_state == BTState::Success && current_tile != target_tile && path.size() == 0)) {
			if ((player_is_manually_moving || do_pathfinding_movement) && state != ProgramState::PAUSED) {
				delete child;
				child = nullptr;
				init(e);
			}
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
		//printf("init\n");
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
		return distance <= 0;
	}

	BTState process(entt::entity e) override {
		return is_close(e) ? BTState::Failure : BTState::Success;
	}
};

class Escape : public BTNode {
	std::vector<vec2> path;
	vec2 current_target;
	MoveToTargetTile* move;
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

	vec2 get_furthest_corner_tile(entt::entity e) {
		size_t max_size = game_state.level.map_tiles.size();
		entt::entity player_entity = registry.view<Player>().begin()[0];
		Motion& player_motion = registry.get<Motion>(player_entity);
		vec2 pos = WorldSystem::position_to_map_coords(player_motion.position);
		bool is_x_greater = pos.x > max_size / 2.;
		bool is_y_greater = pos.y > max_size / 2.;
		int x = !is_x_greater ? max_size - 2 : 1;
		int y = !is_y_greater ? max_size - 2 : 1;
		std::vector<vec2> adj = {{ 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 },{ -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 }};
		for (int i = 0; i < adj.size(); i++) {
			vec2 coord = { x + adj[i][0], y + adj[i][1] };
			if (!WorldSystem::is_within_bounds(coord))
				continue;
			if (WorldSystem::tile_is_walkable(WorldSystem::get_map_tile(coord))) {
				return coord;
			}
		}
		return { -1,-1 };
	}

public:
	Escape(){}
	void init(entt::entity e) override {
		vec2 target_tile = get_furthest_corner_tile(e);
		//printf("moving to %f, %f\n", target_tile.x, target_tile.y);
		move = new MoveToTargetTile();
		move->set_target(target_tile);
		move->init(e);
	}

	BTState process(entt::entity e) override {
		BTState st = move->process(e);
		if (st == BTState::Success) {
			delete move;
			move = nullptr;
		}
		if (st == BTState::Failure) {
			delete move;
			move = nullptr;
			init(e);
		}
		return st;
	}
};

class ChickAI {
private:
	MoveOneTile move_up{ Direction::UP };
	MoveOneTile move_down{ Direction::DOWN };
	MoveOneTile move_left{ Direction::LEFT };
	MoveOneTile move_right{ Direction::RIGHT };
	MoveToRandomTile* move_random;
	BTRandomSelector* select_random_dir;
	Escape* escape;
	CheckIsClose* check_dist;
	BTSequence* roam;
	BTSelector* roam_or_escape;
	BTRepeat* root;
	entt::entity e;
	bool is_init;
	int id;

public:
	ChickAI(const entt::entity e, int id) : e(e), is_init(false), id(id) {
	}

	void step() {
		if (!is_init) {
			check_dist = new CheckIsClose();
			select_random_dir = new BTRandomSelector{ std::vector<BTNode*>{&move_up,& move_down,& move_left,& move_right} };
			//roam = new BTSequence{ std::vector<BTNode*>{check_dist,select_random_dir} };
			roam = new BTSequence{ std::vector<BTNode*>{select_random_dir} };
			escape = new Escape();
			move_random = new MoveToRandomTile();
			roam_or_escape = new BTSelector{ std::vector<BTNode*>{ move_random,escape } };
			root = new BTRepeat(roam_or_escape);
			root->init(e);
			is_init = true;
		}
		BTState state = root->process(e);
	}

	void clear() {
		delete select_random_dir;
		delete root;
		delete roam_or_escape;
		delete check_dist;
		delete escape;
		delete roam;
		delete move_random;
	}

	int get_id() {
		return id;
	}
};

