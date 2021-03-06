#ifndef TOG_COMMIT_H
#define TOG_COMMIT_H

#include <filesystem>
#include <optional>
#include <string>

#include "handle.h"
#include "object.h"
#include "tree.h"

namespace tog {

// A commit represents a single commit in a repository
class Commit : public TogObject {
public:
    Commit(Handle<Tree> tree, std::optional<Handle<Commit>> parent,
           std::string message);

    const std::vector<unsigned char>& serialize();

    Handle<Tree>& tree() {
        return _tree;
    }

    const std::string& message() const {
        return _message;
    }

    const std::optional<Handle<Commit>>& parent() {
        return _parent;
    }

private:
    // the top-level tree of the commit (i.e. the worktree)
    Handle<Tree> _tree;

    // the preceding commit to this commit (if any)
    std::optional<Handle<Commit>> _parent;

    // the user-specified commit message
    std::string _message;

    // cached serialized commit, for lazy serialization
    std::optional<std::vector<unsigned char>> _serialized;
};

}  // namespace tog

#endif  // TOG_COMMIT_H