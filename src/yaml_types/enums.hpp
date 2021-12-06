#pragma once

#include "yaml-cpp/yaml.h"
#include "components.hpp"

namespace YAML {
    template <>
    struct convert<MapTile> {
        static Node encode(const MapTile &val) { return Node((int) val); }

        static bool decode(const Node &node, MapTile &v) {
            if (!node.IsScalar()) {
                return false;
            }

            v = (MapTile) node.as<int>();
            return true;
        }
    };

    template <>
    struct convert<ItemType> {
        static Node encode(const ItemType &val) { return Node((int) val); }

        static bool decode(const Node &node, ItemType &v) {
            if (!node.IsScalar()) {
                return false;
            }

            v = (ItemType) node.as<int>();
            return true;
        }
    };
}
