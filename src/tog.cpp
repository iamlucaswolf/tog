#include <CLI11/CLI11.h>
#include <tomlplusplus/toml.h>

#include <filesystem>
#include <iostream>

#include "blob.h"
#include "repository.h"

namespace fs = std::filesystem;

void init() {
    try {
        tog::Repository::init(fs::current_path());
    } catch (const tog::TogException &err) {
        std::cerr << "Error " << err.what() << std::endl;
    }

    std::cout << "Initialized tog repository" << std::endl;
}

void commit(const std::string &message) {
    auto togdir_path = fs::current_path() / ".tog";

    if (!fs::exists(togdir_path)) {
        std::cout << "Not a tog repository" << std::endl;
        return;
    }

    try {
        auto repo = tog::Repository{togdir_path};
        auto hash = repo.commit(message);

        std::cout << "Created commit " << hash << std::endl;

    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void checkout(const std::string &hash) {
    auto togdir_path = fs::current_path() / ".tog";

    if (!fs::exists(togdir_path)) {
        std::cout << "Not a tog repository" << std::endl;
        return;
    }

    try {
        auto repo = tog::Repository{togdir_path};
        repo.checkout(hash);

        std::cout << "Checked out commit " << hash << std::endl;

    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char **argv) {
    CLI::App app{"The simple version control system", "tog"};

    // Require exactly one argument (such as "init", "commit", ...)
    app.require_subcommand(1);

    // tog init
    auto init_cmd = app.add_subcommand(
        "init", "Creates a new repository in the current directory");
    init_cmd->callback(init);

    // tog commit [-m <message>]
    auto commit_cmd = app.add_subcommand("commit", "Creates a new commit");
    std::string commit_message;
    commit_cmd->add_option("-m,--message", commit_message, "Commit message");
    commit_cmd->callback([&commit_message]() { commit(commit_message); });

    // tog checkout <commit>
    auto checkout_cmd = app.add_subcommand("checkout", "Checkout a commit");
    std::string checkout_hash;

    // TODO fix formatting in .clang-format
    checkout_cmd->add_option("commit", checkout_hash, "Commit hash")
        ->required();
    checkout_cmd->callback([&checkout_hash]() { checkout(checkout_hash); });

    try {
        CLI11_PARSE(app, argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    return 0;
}