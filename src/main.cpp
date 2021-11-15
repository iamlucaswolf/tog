#include <CLI11/CLI11.h>
#include <tomlplusplus/toml.h>

#include <filesystem>
#include <iostream>

#include "cli.h"

int main(int argc, char **argv) {
    CLI::App app{"The simple version control system", "tog"};

    // Require exactly one argument (such as "init", "commit", ...)
    app.require_subcommand(1);

    // tog init
    auto init_cmd = app.add_subcommand(
        "init", "Creates a new repository in the current directory");
    init_cmd->callback(tog::cli::init);

    // tog commit [-m <message>]
    auto commit_cmd = app.add_subcommand("commit", "Creates a new commit");
    std::string commit_message;
    commit_cmd->add_option("-m,--message", commit_message, "Commit message");
    commit_cmd->callback(
        [&commit_message]() { tog::cli::commit(commit_message); });

    // tog checkout <commit>
    auto checkout_cmd = app.add_subcommand("checkout", "Checkout a commit");
    std::string checkout_hash;

    // TODO fix formatting in .clang-format
    checkout_cmd->add_option("commit", checkout_hash, "Commit hash")
        ->required();
    checkout_cmd->callback(
        [&checkout_hash]() { tog::cli::checkout(checkout_hash); });

    // tog status command
    auto status_cmd =
        app.add_subcommand("status", "Display the current branch/commit");
    status_cmd->callback(tog::cli::status);

    // tog log command
    auto log_cmd = app.add_subcommand("log", "Display the commit history");
    int history_length;
    log_cmd
        ->add_option("-n,--number", history_length,
                     "Number of commits to display")
        ->default_val<int>(10);
    log_cmd->callback([&history_length]() { tog::cli::log(history_length); });

    try {
        CLI11_PARSE(app, argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    return 0;
}