#include "blob.h"

#include <filesystem>
#include <fstream>
#include <iterator>

namespace fs = std::filesystem;

namespace tog {

Blob::Blob(const fs::path& path) {
    // Read file in binary mode, without skipping white space
    std::ifstream file(path, std::ios::binary);
    file.unsetf(std::ios::skipws);

    // Get file size by seeking to the end of the stream and getting the cursor
    // position
    file.seekg(0, std::ios::end);
    auto file_size = file.tellg();

    // Seek back to the beginning of the stream
    file.seekg(0, std::ios::beg);

    _data.reserve(file_size);
    _data.insert(_data.begin(), std::istream_iterator<unsigned char>(file),
                 std::istream_iterator<unsigned char>());
}

const std::vector<unsigned char>& Blob::serialize() {
    // as of now, the serialization is just the raw data (i.e. no custom binary
    // layout is used, like in git). this may change in the future
    return _data;
}

}  // namespace tog