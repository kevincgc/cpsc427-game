#pragma once
#include <vector>
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
	std::vector<vec2> direction = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

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
                vec2 new_coord(current_node->coord + direction[i]);
                if (!is_valid_tile(new_coord) || find_node(close, new_coord)) {
                    continue;
                }

                uint totalCost = current_node->g + 10;

                Node* new_node = find_node(open, new_coord);
                if (new_node == nullptr) {
                    new_node = new Node(new_coord, current_node);
                    new_node->g = totalCost;
                    new_node->h = get_cost(new_node->coord, target);
                    open.push_back(new_node);
                }
                else if (totalCost < new_node->g) {
                    new_node->parent = current_node;
                    new_node->g = totalCost;
                }
            }
        }

        while (current_node != nullptr) {
            path.push_back({ current_node->coord.x, current_node->coord.y });
            current_node = current_node->parent;
        }
        for (auto& coordinate : path) {
            printf("%f, %f\n",coordinate.x,coordinate.y);
        }

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