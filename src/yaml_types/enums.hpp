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

    template <>
    struct convert<TEXTURE_ASSET_ID> {
        static Node encode(const TEXTURE_ASSET_ID &val) { return Node((int) val); }

        static bool decode(const Node &node, TEXTURE_ASSET_ID &v) {
            if (!node.IsScalar()) {
                return false;
            }

            v = (TEXTURE_ASSET_ID) node.as<int>();
            return true;
        }
    };

    template <>
    struct convert<GEOMETRY_BUFFER_ID> {
        static Node encode(const GEOMETRY_BUFFER_ID &val) { return Node((int) val); }

        static bool decode(const Node &node, GEOMETRY_BUFFER_ID &v) {
            if (!node.IsScalar()) {
                return false;
            }

            v = (GEOMETRY_BUFFER_ID) node.as<int>();
            return true;
        }
    };
}
