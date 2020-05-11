cmake_minimum_required(VERSION 3.17)
project(ldbdump)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external)

set(EXTRA_LIBRARIES)
if (EMSCRIPTEN)
    set(EXTRA_LIBRARIES "-lnodefs.js")
endif()

add_executable(ldbdump main.cpp)
target_link_libraries(ldbdump PRIVATE leveldb ${EXTRA_LIBRARIES})

if (EMSCRIPTEN)
  # ALLOW_MEMORY_GROWTH is required to have dynamic memory growth
  # EXIT_RUNTIME is required to flush stdout/stderr at exit
  set_target_properties(ldbdump PROPERTIES
    LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s EXIT_RUNTIME=1"
  )
endif()
