#ifndef TOG_HANDLE_H
#define TOG_HANDLE_H

#include <memory>
#include <string>

namespace tog {

// TODO: use C++20 concepts to enforce that T is a TogObject
template <class T>

// A handle represents a TogObject that may have not yet been loaded into
// memory.
class Handle {
public:
    Handle() = default;
    virtual ~Handle() = default;

    // Constructs an unresolved handle to an existing object with a given hash.
    Handle(std::string hash)
        : _hash{std::move(hash)}, _dirty{false}, _object{nullptr} {};

    // Constructs a resolved handle to an existing object with a given hash.
    Handle(std::string hash, std::shared_ptr<T> object, bool dirty)
        : _hash{std::move(hash)}, _dirty{dirty}, _object{std::move(object)} {};

    bool dirty() const {
        return _dirty;
    }

    const std::string& hash() const {
        return _hash;
    }

    const std::shared_ptr<T>& object() const {
        return _object;
    }

private:
    // The hash of the object referenced by this handle.
    std::string _hash;

    // Indicates whether the object referenced by this handle needs to be
    // persisted on disk.
    bool _dirty;

    // The object referenced by this handle. This may be nullptr if the handle
    // is not resolved yet.
    std::shared_ptr<T> _object;
};
}  // namespace tog

#endif  // TOG_REF_H