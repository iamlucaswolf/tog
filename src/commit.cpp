#include "commit.h"

#include <tomlplusplus/toml.h>

namespace tog {

// TODO fix format in .clang-format
Commit::Commit(Handle<Tree> tree, std::optional<Handle<Commit>> parent,
               std::string message)
    : _tree{std::move(tree)}, _parent{parent}, _message{std::move(message)} {}

const std::vector<unsigned char>& Commit::serialize() {
    // serialize lazily
    if (_serialized) {
        return *_serialized;
    }

    // encode as toml
    auto commit_toml = toml::table{{
        {"message", _message},
        {"tree", _tree.hash()},
        {"parent", _parent ? _parent->hash() : ""},
    }};

    std::stringstream stream{};
    stream << commit_toml;

    std::string commit_str = stream.str();
    _serialized.emplace(commit_str.begin(), commit_str.end());

    return *_serialized;
}

}  // namespace tog