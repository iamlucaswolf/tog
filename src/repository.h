#ifndef TOG_REPOSITORY_H
#define TOG_REPOSITORY_H

#include <exception>
#include <filesystem>
#include <memory>
#include <string>

#include "blob.h"
#include "commit.h"
#include "handle.h"
#include "tree.h"

namespace tog {

class TogException : public std::exception {
public:
    TogException(const std::string& message) : message(message) {}
    const char* what() const noexcept override {
        return message.c_str();
    }

private:
    const std::string message;
};

class Repository {
public:
    Repository(const std::filesystem::path& togdir_path);

    // commits the current worktree contents with the given commit message and
    // returns the commit object's hash
    std::string commit(const std::string& message);

    // restores the worktree to the state captured by the given commit
    void checkout(const std::string& hash);

    // returns the current branch's last n commit hashes in
    // reverse-chronological order (newest first).
    std::vector<std::string> history(int n);

    // Initialized a new repository in the given directory.
    static void init(const std::filesystem::path& path);

    std::optional<std::string> head() const {
        return _head ? std::optional<std::string>{_head->hash()} : std::nullopt;
    }

    std::optional<std::string> main() const {
        return _main ? std::optional<std::string>{_main->hash()} : std::nullopt;
    }

private:
    // add_<object> loads an object from the given path, creates an object
    // from it and adds it to the repository.
    // TODO: Unify these methods once I have better understanding of templates
    Handle<Blob>& add_file(const std::filesystem::path& file_path);
    Handle<Tree>& add_directory(const std::filesystem::path& directory_path);

    // register_object will move the given object into the repository's object
    // store and return a (resolved) handle to it. Note that this handle may not
    // refer to the same (as in "identical") object as the one passed in.
    // TODO: Unify these methods once I have better understanding of templates
    Handle<Blob>& register_object(std::unique_ptr<Blob> blob);
    Handle<Tree>& register_object(std::unique_ptr<Tree> tree);
    Handle<Commit>& register_object(std::unique_ptr<Commit> commit);

    // Resolves the given handle. If the hash is not found in the repository,
    // an exception is thrown. Note that the returned handle may not be the
    // same as the one passed in.
    void resolve(Handle<Blob>& blob);
    void resolve(Handle<Tree>& tree);
    void resolve(Handle<Commit>& commit);

    // recursively restores the contents of the given tree at the given path.
    void restoreTree(Handle<Tree>& tree, const std::filesystem::path& path);
    void restoreBlob(Handle<Blob>& blob, const std::filesystem::path& path);

    // load/store refs from .tog/refs
    std::optional<Handle<Commit>> load_ref(const std::filesystem::path& path);
    void persist_ref(const std::filesystem::path& path,
                     const std::optional<Handle<Commit>>& commit);

    // togdir_path is the path to the .tog, _worktree_path is the path to the
    // directory tracked by this repository. Currently, the worktree is always
    // togdir/..
    std::filesystem::path _togdir_path;
    std::filesystem::path _worktree_path;

    // lazily stores handles to objects in the repository
    // TODO: Unify to single object store once I have better understanding of
    // templates
    std::unordered_map<std::string, Handle<Blob>> _blobs;
    std::unordered_map<std::string, Handle<Tree>> _trees;
    std::unordered_map<std::string, Handle<Commit>> _commits;

    // stores the currently checked-out commit (if any)
    std::optional<Handle<Commit>> _head;

    // stores the latest commit on the main branch (currently the only branch)
    std::optional<Handle<Commit>> _main;
};

}  // namespace tog

#endif  // TOG_REPOSITORY_H