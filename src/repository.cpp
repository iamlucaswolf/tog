#include "repository.h"

#include <tomlplusplus/toml.h>

#include <iostream>

#include "blob.h"
#include "commit.h"
#include "crypto.h"
#include "handle.h"
#include "tree.h"

namespace fs = std::filesystem;

namespace tog {

void Repository::init(const fs::path& path) {
    auto togdir_path = path / ".tog";

    // Check if .tog directory exists
    if (fs::exists(togdir_path)) {
        throw TogException{"already a tog repository"};
        return;
    }

    // Create repository directories
    fs::create_directories(togdir_path / "objects");
    fs::create_directories(togdir_path / "refs" / "branches");

    // Create default config file
    auto config = toml::table{{
        {"version", "0.0.0-alpha"},
        {"worktree", ".."},
    }};

    std::ofstream config_file{togdir_path / "config.toml"};
    config_file << config << std::endl;

    // Create (empty) main and head refs
    std::ofstream{togdir_path / "refs" / "branches" / "main"};
    std::ofstream head_file{togdir_path / "refs" / "head"};
}

Repository::Repository(const fs::path& togdir_path)
    : _togdir_path{togdir_path} {
    auto config_path = togdir_path / "config.toml";
    toml::table config;

    try {
        config = toml::parse_file(config_path.string());

    } catch (const toml::parse_error& err) {
        throw TogException{"Unable to load repository."};
    }

    auto rel_worktree_path{config["worktree"].value<std::string>()};

    if (!rel_worktree_path) {
        throw TogException{"Unable to load repository"};
    } else {
        this->_worktree_path = fs::canonical(togdir_path / *rel_worktree_path);
    }

    _head = load_ref(togdir_path / "refs" / "head");
    _main = load_ref(togdir_path / "refs" / "branches" / "main");
}

std::string Repository::commit(const std::string& message) {
    // cannot commit if head is not at the latest commit of the main branch
    // (which is the only branch for now). In git, this is known as "detached
    // head".
    if (_head && (_head->hash() != _main->hash())) {
        throw TogException{"not at latest commit of current branch"};
    }

    auto tree_handle = add_directory(_worktree_path);
    auto commit =
        register_object(std::make_unique<Commit>(tree_handle, _head, message));

    // Update refs (by copy)
    _head = commit;
    _main = commit;

    // TODO unify persist-loops once I have a better understanding of templates
    auto objects_path = _togdir_path / "objects";

    // persist commit
    if (commit.dirty()) {
        auto bytes = commit.object()->serialize();

        std::ofstream file{objects_path / commit.hash(), std::ios_base::binary};
        file.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
    }

    // persist blob objects
    for (const auto& [hash, handle] : _blobs) {
        if (handle.dirty()) {
            auto bytes = handle.object()->serialize();

            std::ofstream file{objects_path / hash, std::ios_base::binary};
            file.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
        }
    }

    // persist commit objects
    for (const auto& [hash, handle] : _trees) {
        if (handle.dirty()) {
            auto bytes = handle.object()->serialize();

            std::ofstream file{objects_path / hash, std::ios_base::binary};
            file.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
        }
    }

    // persist refs
    persist_ref(_togdir_path / "refs" / "head", _head);
    persist_ref(_togdir_path / "refs" / "branches" / "main", _main);

    return commit.hash();
}

void Repository::checkout(const std::string& hash) {
    auto commit = Handle<Commit>{hash};
    resolve(commit);

    // clear working directory (except .tog)
    for (const auto& entry : fs::directory_iterator(_worktree_path)) {
        if (entry.path().filename() != ".tog") {
            fs::remove_all(entry.path());
        }
    }

    auto tree = commit.object()->tree();
    resolve(tree);
    restoreTree(tree, _worktree_path);

    _head = commit;
    persist_ref(_togdir_path / "refs" / "head", _head);
}

void Repository::resolve(Handle<Blob>& blob) {
    if (blob.resolved()) {
        return;
    }

    auto hash = blob.hash();
    auto path = _togdir_path / "objects" / hash;

    if (!fs::exists(path)) {
        throw TogException{"object not found"};
    }

    // cache for future resolutions
    if (!_blobs.contains(hash)) {
        // Explicitly copy hash/object to avoid dangling references
        _blobs.emplace(std::string{hash}, Handle<Blob>{blob});
    }

    auto& cached = _blobs.at(hash);

    if (!cached.resolved()) {
        // TODO error management; what if object is not a blob?

        // TODO using the blob constructor works, because blobs are not yet
        // compressed. Revisit this when we have compression.
        cached.resolve(std::make_shared<Blob>(path));
    }

    // copy assignment operator
    blob = cached;
}

void Repository::resolve(Handle<Tree>& tree) {
    if (tree.resolved()) {
        return;
    }

    auto hash = tree.hash();
    auto path = _togdir_path / "objects" / hash;

    if (!fs::exists(path)) {
        throw TogException{"object not found"};
    }

    // cache for future resolutions
    if (!_trees.contains(hash)) {
        // Explicitly copy hash/object to avoid dangling references
        _trees.emplace(std::string{hash}, Handle<Tree>{tree});
    }

    auto& cached = _trees.at(hash);

    if (!cached.resolved()) {
        // TODO error management; what if object is not a tree?

        // load commit from disk
        auto deserialized = toml::parse_file(path.string());

        // parse blobs
        std::unordered_map<std::string, Handle<Blob>> blobs;

        for (const auto& [key, value] : *deserialized["blobs"].as_table()) {
            std::string blob_hash{value.value_or("")};

            if (blob_hash.empty()) {
                throw TogException{"corrupt tree object " + hash};
            }

            blobs.emplace(key, Handle<Blob>{blob_hash});
        }

        // parse trees
        std::unordered_map<std::string, Handle<Tree>> trees;

        for (const auto& [key, value] : *deserialized["trees"].as_table()) {
            std::string tree_hash{value.value_or("")};

            if (tree_hash.empty()) {
                throw TogException{"corrupt tree object " + hash};
            }

            trees.emplace(key, Handle<Tree>{tree_hash});
        }

        cached.resolve(
            std::make_shared<Tree>(std::move(blobs), std::move(trees)));
    }

    // copy assignment operator
    tree = cached;
}

void Repository::resolve(Handle<Commit>& commit) {
    if (commit.resolved()) {
        return;
    }

    auto hash = commit.hash();
    auto path = _togdir_path / "objects" / hash;

    if (!fs::exists(path)) {
        throw TogException{"commit does not exist"};
    }

    // cache for future resolutions
    if (!_commits.contains(hash)) {
        // Explicitly copy hash/object to avoid dangling references
        _commits.emplace(std::string{hash}, Handle<Commit>{commit});
    }

    auto& cached = _commits.at(hash);

    if (!cached.resolved()) {
        // TODO error management; what if object is not a commit?

        // load commit from disk
        auto deserialized = toml::parse_file(path.string());

        // parse tree
        auto tree_hash = deserialized["tree"].value<std::string>();
        auto tree = Handle<Tree>{*tree_hash};

        // parse parent
        auto parent_hash = deserialized["parent"].value<std::string>();
        std::optional<Handle<Commit>> parent;

        if (parent_hash && !parent_hash->empty()) {
            parent = Handle<Commit>{*parent_hash};
        }

        // parse message
        auto message = deserialized["message"].value<std::string>();

        cached.resolve(
            std::make_shared<Commit>(tree, parent, message.value_or("")));
    }

    // copy assignment operator
    commit = cached;
}

void Repository::restoreTree(Handle<Tree>& tree, const fs::path& path) {
    resolve(tree);

    // Create the directory if it doesn't exist
    fs::create_directories(path);

    // Populate the directory with files
    for (auto& [name, blob] : tree.object()->blobs()) {
        restoreBlob(blob, path / name);
    }

    // Populate the directory with subdirectories
    for (auto& [name, sub_tree] : tree.object()->trees()) {
        restoreTree(sub_tree, path / name);
    }
}

void Repository::restoreBlob(Handle<Blob>& blob, const fs::path& path) {
    resolve(blob);
    const auto& data = blob.object()->data();

    std::ofstream stream{path, std::ios::out | std::ios::binary};
    stream.write(reinterpret_cast<const char*>(data.data()), data.size());
}

Handle<Blob>& Repository::add_file(const fs::path& file_path) {
    return register_object(std::make_unique<Blob>(file_path));
}

Handle<Tree>& Repository::add_directory(const fs::path& directory_path) {
    std::unordered_map<std::string, Handle<Blob>> files;
    std::unordered_map<std::string, Handle<Tree>> directories;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.is_directory()) {
            // skip togdir
            if (entry.path().filename() == ".tog") {
                continue;
            }

            directories.emplace(entry.path().filename(),
                                add_directory(entry.path()));
        } else if (entry.is_regular_file()) {
            files.emplace(entry.path().filename().string(),
                          add_file(entry.path()));
        }
    }

    return register_object(
        std::make_unique<Tree>(std::move(files), std::move(directories)));
}

Handle<Blob>& Repository::register_object(std::unique_ptr<Blob> blob) {
    auto bytes = blob->serialize();
    auto hash = sha256(bytes);

    if (!_blobs.contains(hash)) {
        // If there is a matching file in .tog/objects/ the blob is already
        // persisted. Otherwise, mark it as dirty to persist it later.
        auto dirty = !fs::exists(_togdir_path / "objects" / hash);
        _blobs.emplace(hash, Handle<Blob>{hash, std::move(blob), dirty});
    }

    return _blobs.at(hash);
}

Handle<Tree>& Repository::register_object(std::unique_ptr<Tree> tree) {
    auto bytes = tree->serialize();
    auto hash = sha256(bytes);

    if (!_trees.contains(hash)) {
        auto dirty = !fs::exists(_togdir_path / "objects" / hash);
        _trees.emplace(hash, Handle<Tree>{hash, std::move(tree), dirty});
    }

    return _trees.at(hash);
}

Handle<Commit>& Repository::register_object(std::unique_ptr<Commit> commit) {
    auto bytes = commit->serialize();
    auto hash = sha256(bytes);

    if (!_commits.contains(hash)) {
        auto dirty = !fs::exists(_togdir_path / "objects" / hash);
        _commits.emplace(hash, Handle<Commit>{hash, std::move(commit), dirty});
    }

    return _commits.at(hash);
}

std::optional<Handle<Commit>> Repository::load_ref(const fs::path& path) {
    std::ifstream stream{path};

    if (!stream) {
        throw TogException{"Unable to load ref " + path.string()};
    }

    std::string hash;
    std::getline(stream, hash);

    if (hash.empty()) {
        return std::nullopt;
    }

    if (!fs::exists(_togdir_path / "objects" / hash)) {
        throw TogException{"ref " + path.string() +
                           " points to non-existent commit " + hash};
    }

    auto commit = Handle<Commit>{hash};

    return std::optional<Handle<Commit>>{std::move(commit)};
}

void Repository::persist_ref(const fs::path& path,
                             const std::optional<Handle<Commit>>& commit) {
    std::ofstream stream{path, std::ofstream::trunc};

    if (!stream) {
        throw TogException{"Unable to save ref " + path.string()};
    }

    if (commit) {
        stream << commit->hash() << std::endl;
    }
}

}  // namespace tog