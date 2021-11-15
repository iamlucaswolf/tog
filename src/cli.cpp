#include "cli.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "repository.h"

namespace fs = std::filesystem;

namespace tog::cli {

using namespace tog;

tog::Repository load_repository() {
    const auto path = fs::current_path() / ".tog";

    if (!fs::exists(path)) {
        throw TogException("Not a tog repository");
    }

    return Repository{path};
}

void init() {
    try {
        Repository::init(fs::current_path());
    } catch (const TogException &err) {
        std::cerr << "Error " << err.what() << std::endl;
    }

    std::cout << "Initialized tog repository" << std::endl;
}

void commit(const std::string &message) {
    try {
        auto repo = load_repository();
        auto hash = repo.commit(message);

        std::cout << "Created commit " << hash << std::endl;

    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void checkout(const std::string &hash) {
    try {
        auto repo = load_repository();
        repo.checkout(hash);

        std::cout << "Checked out commit " << hash << std::endl;

    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void status() {
    try {
        auto repo = load_repository();

        auto head = repo.head();
        auto main = repo.main();

        std::cout << "On branch main" << std::endl;

        if (main) {
            std::cout << "Current commit: " << *head << std::endl;
            std::cout << "Latest commit: " << *main << std::endl;
        } else {
            std::cout << "No commits yet" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void log(int history_length) {
    try {
        auto repo = load_repository();
        auto history = repo.history(history_length);

        if (history.empty()) {
            std::cout << "No commits yet" << std::endl;
        }

        for (const auto &hash : history) {
            std::cout << hash << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}
}  // namespace tog::cli