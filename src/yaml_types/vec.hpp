#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "yaml-cpp/yaml.h"

namespace YAML {
    template <>
    struct convert<vec2> {
        static Node encode(const vec2 &v) {
            Node node;
            node.push_back(v.x);
            node.push_back(v.y);
            return node;
        }

        static bool decode(const Node &node, vec2 &v) {
            if (!node.IsSequence() || node.size() != 2) {
                return false;
            }

            v.x = node[0].as<double>();
            v.y = node[1].as<double>();
            return true;
        }
    };

    template <>
    struct convert<vec3> {
        static Node encode(const vec3 &v) {
            Node node;
            node.push_back(v.x);
            node.push_back(v.y);
            node.push_back(v.z);
            return node;
        }

        static bool decode(const Node &node, vec3 &v) {
            if (!node.IsSequence() || node.size() != 2) {
                return false;
            }

            v.x = node[0].as<double>();
            v.y = node[1].as<double>();
            v.z = node[2].as<double>();
            return true;
        }
    };
}
