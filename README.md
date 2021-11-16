# tog

`tog` is a simple version control system, similar to [git](https://git-scm.com/).

## Usage
To create a new repository, run `tog init`:
```bash
> mkdir repo && cd repo
> tog init
```

To create commits, simply run `tog commit` at the top-level directory of the
repository:
```bash
> tog commit -m "Initial commit"
Created commit 64A1DB832731D1F41F18305E850495935C87CE9ABF44E39BF31B7B3BD714AAB9
```

**Note**: As of now, tog commits all files and subdirectories of the repository,
i.e. there is no staging area as in git. There also is only one branch, the
"main" branch. I hope to add staging, branching and merging in future versions.

A history of commits can be viewed with `tog log`:
```bash
> tog log -n 3 # the -n flag cuts off the first 3 commits
2FA3BA362E27964A473F18CF73800ADC1E4576C523F4D0817950F6A3532DCE14
E4A04F492CE824E6DEA4029227880FF45E1D7DACD05F7F3E119755765B1D48A0
24EC46E9D4F0B105BA1DAA717375AFB4F856A0C5C484E4D6F891EB1706C12184
```

To check out a previous commit, run `tog checkout`. This restores the state
of the repository to that of the specified commit:
```bash
> tog checkout 24EC46E9D4F0B105BA1DAA717375AFB4F856A0C5C484E4D6F891EB1706C12184
Checked out commit 24EC46E9D4F0B105BA1DAA717375AFB4F856A0C5C484E4D6F891EB1706C12184
```

To view the commit currently checked out, as well as the latest commit on the
main branch, run
```bash
> tog status
On branch main
Current commit: 24EC46E9D4F0B105BA1DAA717375AFB4F856A0C5C484E4D6F891EB1706C12184
Latest commit: 2FA3BA362E27964A473F18CF73800ADC1E4576C523F4D0817950F6A3532DCE14
```

## Project Layout
The project is split into the following files:

- `main.cpp`: The main function. Parses CLI arguments and calls the respective
    handler function from `cli.h`
- `cli.h/cli.cpp`: Handler functions for the several `tog commands`
- `repository.h/repository.cpp`: The repository class, which loads, manipulates,
    and stores the current state of the repository
- `object.h/object.cpp`: Superclass for all objects in the repository (blobs,
    trees, commits)
- `blob.h/blob.cpp`: A blob is a string of bytes. Files in the repository are
    represented as blobs.
- `tree.h/tree.cpp`: A tree is a collection of blobs and sub-trees. Directories
    in the repository are represented as trees.
- `commit.h/commit.cpp`: A commit is a tree and optionally, a pointer to a
    parent commit. The tree represents the repository's top-level directory.
- `handle.h/handle.cpp`: A handle refers to an object. It is used to defer
    loading objects from disk to the point in time when they are needed. This
    "lazy loading" strategy allows for doing work on large repositiories
    efficiently, withotu having to load all objects into memory.


## Dependencies

`tog` makes use of features from the C++20 standard, so make sure you have an
up-to-date compiler.

Additionally, `tog` depends on the following third-party libraries:

- [`toml++`](https://marzer.github.io/tomlplusplus/) (for parsing and
  writing `toml` files)
- [`CLI11`](https://github.com/CLIUtils/CLI11) (for parsing command line
  arguments)
- [`Crypto++`](https://cryptopp.com/) (for cryptographic hashing)

`toml++` and `CLI11` are included with this repository as header-only libraries.
`Crypto++` needs to be installed separately:

On Ubuntu, `Crypto++` can be installed with the following command:
```bash
sudo apt update
sudo apt install libcrypto++-dev libcrypto++-doc libcrypto++-utils libcrypto++6
libcrypto++6-dbg
```

For other OSes, please refer to the Crypto++ documentation
([Windows](https://www.cryptopp.com/wiki/Visual_Studio) |
[macOS](https://www.cryptopp.com/wiki/OS_X_(Command_Line))).

## Building tog

tog uses CMake as a build system. After installing the dependencies (see above),
simply run:

```bash
mkdir build
cd build
cmake ..
make
```

## License
tog is licensed under the terms of the MIT license. See [LICENSE](LICENSE) for
more information.