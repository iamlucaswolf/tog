#include <CLI11/CLI11.h>
#include <tomlplusplus/toml.h>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void init() {
    auto togdir_path = fs::current_path() / ".tog";

    // Check if .tog directory exists
    if (fs::exists(togdir_path)) {
        std::cout << "Already a tog repository" << std::endl;
        return;
    }

    // Create repository directories
    fs::create_directories(togdir_path / "objects");
    fs::create_directories(togdir_path / "refs" / "branches");

    // Create default config file
    auto config = toml::table{{
        {"version", "0.0.0-alpha"},
        {"worktree", ".."},
    }};

    std::ofstream config_file{togdir_path / "config.toml"};
    config_file << config << std::endl;

    // Create (empty) main branch
    std::ofstream{togdir_path / "refs" / "branches" / "main"};

    // Create head (pointing to main)
    std::ofstream head_file{togdir_path / "head.toml"};
    head_file << toml::table{{{"branch", "main"}}} << std::endl;

    std::cout << "Initialized tog repository" << std::endl;
}

int main(int argc, char **argv) {
    CLI::App app{"A simple version control system", "tog"};
    app.require_subcommand(1);

    app.add_subcommand("init",
                       "Creates a new repository in the current directory")
        ->callback(init);

    try {
        CLI11_PARSE(app, argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    return 0;
}