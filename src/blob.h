#ifndef TOG_BLOB_H
#define TOG_BLOB_H

#include <filesystem>
#include <string>
#include <vector>

#include "object.h"

namespace tog {

// A blob is a sequence of bytes that corresponds to a file in the worktree
struct Blob : public TogObject {
public:
    // Create a blob from file
    Blob(const std::filesystem::path& path);
    const std::vector<unsigned char>& serialize();

private:
    std::vector<unsigned char> _data;
};

}  // namespace tog

#endif