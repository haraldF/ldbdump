# ldbdump

![CI](https://github.com/haraldF/ldbdump/workflows/emscripten%20CI/badge.svg)

Simple node.js package to dump a [LevelDB](https://github.com/google/leveldb) database.

Uses `emscripten` to build the C++ code so it'll work everywhere.
No custom compilation on installation required.

## Usage

```
Usage: ldbdump <db_path> [key]
  A tool to introspect a leveldb database

  <db_path>    directory of the cache
  [key]        optional - dumps the value for the given key to stdout
```

Pass the path to the database directory as first argument to see all keys.
Pass a key as second parameter to dump the value for the given key.

## Building

In order to build this project, install [emscripten](https://emscripten.org/docs/getting_started/downloads.html) and [CMake](https://cmake.org/). Make sure to set up emscripten correctly and activate it.

Then, run the following commands to build the C++ parts:

```sh
emcmake cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

In order to create your own npm package, run the following command:

```sh
npm pack
```
