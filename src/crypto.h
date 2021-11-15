#ifndef TOG_CRYPTO_H
#define TOG_CRYPTO_H

#include <string>
#include <vector>

namespace tog {

// computes the SHA-256 hash of the given data
std::string sha256(const std::vector<unsigned char>& data);

// verifies that the given hash signature is valid for the given data
inline bool verify_sha256(const std::vector<unsigned char>& data,
                          const std::string& hash) {
    return sha256(data) == hash;
}

}  // namespace tog

#endif