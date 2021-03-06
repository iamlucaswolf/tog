#ifndef TOG_TREE_H
#define TOG_TREE_H

#include <string>
#include <unordered_map>

#include "blob.h"
#include "handle.h"
#include "object.h"

namespace tog {

class Tree : public TogObject {
public:
    Tree(std::unordered_map<std::string, Handle<Blob>> &&blobs,
         std::unordered_map<std::string, Handle<Tree>> &&trees);

    const std::vector<unsigned char> &serialize();

    std::unordered_map<std::string, Handle<Blob>> &blobs() {
        return _blobs;
    }

    std::unordered_map<std::string, Handle<Tree>> &trees() {
        return _trees;
    }

private:
    // cached serialized tree, for lazy serialization
    std::optional<std::vector<unsigned char>> _serialized;

    std::unordered_map<std::string, Handle<Blob>> _blobs;
    std::unordered_map<std::string, Handle<Tree>> _trees;
};

}  // namespace tog

#endif  // TOG_TREE_H