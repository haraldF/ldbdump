#include <leveldb/db.h>

#include <iostream>
#include <memory>

#include <stdio.h>
#include <stdlib.h>

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#endif

static void dumpAllKeys(leveldb::DB* db, bool keysOnly) {
  auto iter = std::unique_ptr<leveldb::Iterator>{db->NewIterator({})};
  iter->SeekToFirst();
  while (iter->Valid()) {
    std::cout << iter->key().ToString();
    if (!keysOnly) {
      std::cout << " -> " << iter->value().size()
                << " bytes" << std::endl;
    }
    iter->Next();
  }
}

#ifdef EMSCRIPTEN
// note - this is an emscripten specific workaround to write binary to stdout
// for some reason, doing an frwrite to stdout produces garbage, probably
// because of utf-8 conversion ??
EM_JS(void, to_stdout, (const char* buffer, size_t length), {
    const array = new Uint8Array(length);
    for (let i = 0; i < length; i++) {
      array[i] = getValue(buffer + i, 'i8');
    }

    process.stdout.write(array);
});
#endif

static bool dumpValue(leveldb::DB* db, const std::string& key) {
  auto iter = std::unique_ptr<leveldb::Iterator>{db->NewIterator({})};
  iter->Seek(key);
  if (!iter->Valid() || iter->key() != key) {
    std::cerr << "No such key in database: " << key << std::endl;
    return false;
  }

#ifdef EMSCRIPTEN
  to_stdout(iter->value().data(), iter->value().size());
#else
  // use fwrite to make sure that \0 bytes are also written
  fwrite(iter->value().data(), 1, iter->value().size(), stdout);
#endif

  return true;
}

static void printUsage(const char* argv0) {
    std::cout << "Usage: " << argv0 << " <db_path> [-k] [key]" << std::endl;
    std::cout << "  A tool to introspect a leveldb database" << std::endl;
    std::cout << std::endl;
    std::cout << "  <db_path>    directory of the cache" << std::endl;
    std::cout << "  [key]        optional - dumps the value for the given key "
                 "to stdout"
              << std::endl;
    std::cout << "  [-k]         print only keys, one per line" << std::endl;
}

int main(int argc, char* argv[]) {

  bool keysOnly = false;
  std::string dbPath;
  std::string key;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      printUsage(argv[0]);
      return EXIT_SUCCESS;
    } else if (arg == "-k") {
      keysOnly = true;
    } else if (dbPath.empty()) {
      dbPath = std::move(arg);
    } else {
      key = std::move(arg);
    }
  }

  if (dbPath.empty()) {
    printUsage(argv[0]);
    return EXIT_FAILURE;
  }

  leveldb::DB* db_ptr;
  auto status = leveldb::DB::Open({}, dbPath, &db_ptr);
  std::unique_ptr<leveldb::DB> db{db_ptr};  // clean up on exit

  if (!status.ok()) {
    std::cerr << "Unable to open leveldb: " << status.ToString() << std::endl;
    return EXIT_FAILURE;
  }

  if (!key.empty()) {
    if (!dumpValue(db.get(), key)) return EXIT_FAILURE;
  } else {
    dumpAllKeys(db.get(), keysOnly);
  }

  return EXIT_SUCCESS;
}
