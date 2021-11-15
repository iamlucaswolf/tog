#ifndef TOG_CLI_H
#define TOG_CLI_H

#include <filesystem>
#include <string>

#include "repository.h"

namespace tog::cli {

// helper function to load reposiotries
tog::Repository load_repository();

// initializes a new tog repository
void init();

// commits the current workdir contents with the given commit message
void commit(const std::string &message);

// restores workdir contents to the commti with the given hash
void checkout(const std::string &hash);

// prints out status information about the current branch/commit
void status();

// prints the hashes of the last n commits
void log(int n);

}  // namespace tog::cli

#endif  // TOG_TOG_H