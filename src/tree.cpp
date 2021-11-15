#include "tree.h"

#include <tomlplusplus/toml.h>

#include <sstream>

using namespace tog;

namespace tog {

Tree::Tree(std::unordered_map<std::string, Handle<Blob>> &&blobs,
           std::unordered_map<std::string, Handle<Tree>> &&trees)
    : _blobs{std::move(blobs)}, _trees{std::move(trees)} {}

const std::vector<unsigned char> &Tree::serialize() {
    // serialize lazily
    if (_serialized) {
        return *_serialized;
    }

    // encode as toml
    auto blobs_toml = toml::table();
    for (const auto &[name, blob] : _blobs) {
        blobs_toml.insert(name, blob.hash());
    }

    auto trees_toml = toml::table();
    for (const auto &[name, tree] : _trees) {
        trees_toml.insert(name, tree.hash());
    }

    auto tree_toml = toml::table{{
        {"blobs", blobs_toml},
        {"trees", trees_toml},
    }};

    // Serialize to string
    std::stringstream stream{};
    stream << tree_toml;

    std::string tree_str = stream.str();
    _serialized.emplace(tree_str.begin(), tree_str.end());

    return *_serialized;
}

}  // namespace tog
