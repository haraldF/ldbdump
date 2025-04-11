set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY ON)

set(SNAPPY_URL "https://github.com/google/snappy.git" CACHE STRING "Url to Snappy git")
set(SNAPPY_TAG "1.2.2" CACHE STRING "Git tag to build")
set(SNAPPY_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}" -DSNAPPY_BUILD_TESTS=OFF)

set(LEVELDB_URL "https://github.com/google/leveldb.git" CACHE STRING "URL to LevelDB git")
set(LEVELDB_TAG "1.23" CACHE STRING "Git tag to build")
set(LEVELDB_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}" -DLEVELDB_BUILD_TESTS=OFF -DLEVELDB_BUILD_BENCHMARKS=OFF)

set(EXTERNAL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/external-install)

include(ExternalProject)

ExternalProject_Add(snappy_external
  GIT_REPOSITORY    ${SNAPPY_URL}
  GIT_TAG           ${SNAPPY_TAG}
  CMAKE_ARGS        ${CMAKE_ARGS} ${SNAPPY_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
  INSTALL_DIR       ${EXTERNAL_INSTALL_DIR}
  TEST_COMMAND      ""
)

ExternalProject_Get_Property(snappy_external INSTALL_DIR)
set(SNAPPY_INSTALL_DIR ${INSTALL_DIR})

# LevelDB uses check_library_exists, which requires CMAKE_REQUIRED_LINK_OPTIONS and CMAKE_REQUIRED_FLAGS
# Once snappy is found, it doesn't propagate the -I and -L, so we need to manually override that as well
set (LEVELDB_REQUIRED_FLAGS
  -DCMAKE_REQUIRED_LINK_OPTIONS=-L${SNAPPY_INSTALL_DIR}/lib
  -DCMAKE_REQUIRED_FLAGS=-I${SNAPPY_INSTALL_DIR}/include
  -DCMAKE_CXX_FLAGS=-I${SNAPPY_INSTALL_DIR}/include
  -DCMAKE_SHARED_LINKER_FLAGS=-L${SNAPPY_INSTALL_DIR}/lib
  -DCMAKE_EXE_LINKER_FLAGS=-L${SNAPPY_INSTALL_DIR}/lib
)

ExternalProject_Add(leveldb_external
  GIT_REPOSITORY    ${LEVELDB_URL}
  GIT_TAG           ${LEVELDB_TAG}
  PATCH_COMMAND     patch --batch --dry-run --reverse --silent -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/leveldb.compaction.patch && echo "Patch already applied" || patch --forward -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/leveldb.compaction.patch
  CMAKE_ARGS        ${CMAKE_ARGS}
                    ${LEVELDB_CMAKE_ARGS}
                    "-DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}"
                    ${LEVELDB_REQUIRED_FLAGS}
  INSTALL_DIR       ${EXTERNAL_INSTALL_DIR}
  TEST_COMMAND      ""
)
ExternalProject_Add_StepDependencies(leveldb_external build snappy_external)

ExternalProject_Get_Property(leveldb_external INSTALL_DIR)
set(LEVELDB_INSTALL_DIR ${INSTALL_DIR})

# Cannot set INTERFACE_INCLUDE_DIRECTORIES to a non-existing path.
# Create it here to solve the configuration time issue
file(MAKE_DIRECTORY ${LEVELDB_INSTALL_DIR}/include)

add_library(leveldb STATIC IMPORTED GLOBAL)
set_target_properties(leveldb PROPERTIES
    IMPORTED_LOCATION ${LEVELDB_INSTALL_DIR}/lib/libleveldb.a
    IMPORTED_LINK_INTERFACE_LIBRARIES ${SNAPPY_INSTALL_DIR}/lib/libsnappy.a
    INTERFACE_INCLUDE_DIRECTORIES ${LEVELDB_INSTALL_DIR}/include
)
add_dependencies(leveldb leveldb_external)
