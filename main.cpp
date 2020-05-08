#include <leveldb/db.h>

#include <iostream>
#include <memory>

#include <stdio.h>
#include <stdlib.h>

static void dumpAllKeys(leveldb::DB* db) {
  auto iter = std::unique_ptr<leveldb::Iterator>{db->NewIterator({})};
  iter->SeekToFirst();
  while (iter->Valid()) {
    std::cout << iter->key().ToString() << " -> " << iter->value().size()
              << " bytes" << std::endl;
    iter->Next();
  }
}

static bool dumpValue(leveldb::DB* db, const std::string& key) {
  auto iter = std::unique_ptr<leveldb::Iterator>{db->NewIterator({})};
  iter->Seek(key);
  if (!iter->Valid() || iter->key() != key) {
    std::cerr << "No such key in database: " << key << std::endl;
    return false;
  }

  // use fwrite to make sure that \0 bytes are also written
  fwrite(iter->value().data(), 1, iter->value().size(), stdout);

  return true;
}

int main(int argc, char* argv[]) {
  if (argc < 2 || argv[1] == std::string("--help") ||
      argv[1] == std::string("-h")) {
    std::cout << "Usage: " << argv[0] << " <db_path> [key]" << std::endl;
    std::cout << "  A tool to introspect a leveldb database" << std::endl;
    std::cout << std::endl;
    std::cout << "  <db_path>    directory of the cache" << std::endl;
    std::cout << "  [keys]       optional - dumps the value for the given key "
                 "to stdout"
              << std::endl;
    return EXIT_FAILURE;
  }

  auto dbPath = argv[1];
  auto key = argc >= 2 ? argv[2] : nullptr;

  leveldb::DB* db_ptr;
  auto status = leveldb::DB::Open({}, dbPath, &db_ptr);
  std::unique_ptr<leveldb::DB> db{db_ptr};  // clean up on exit

  if (!status.ok()) {
    std::cerr << "Unable to open leveldb: " << status.ToString() << std::endl;
    return EXIT_FAILURE;
  }

  if (key != nullptr) {
    if (!dumpValue(db.get(), key)) return EXIT_FAILURE;
  } else {
    dumpAllKeys(db.get());
  }

  return EXIT_SUCCESS;
}