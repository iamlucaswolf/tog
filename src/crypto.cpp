#include "crypto.h"

#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

namespace tog {

std::string sha256(const std::vector<unsigned char> &data) {
    CryptoPP::SHA256 hash;
    std::string digest;

    // CryptoPP assumes ownership of the created raw pointers, so no
    // need to delete them.
    auto sink = new CryptoPP::StringSink(digest);
    auto hex = new CryptoPP::HexEncoder(sink);
    auto filter = new CryptoPP::HashFilter(hash, hex);
    CryptoPP::ArraySource(data.data(), data.size(), true, filter);

    return digest;
}

}  // namespace tog