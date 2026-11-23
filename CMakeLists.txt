cmake_minimum_required(VERSION 3.16)
project(ds_team_project CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Benchmarks must use an optimized build. Release is the default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_compile_options(-Wall -Wextra)

# -DSANITIZE=ON builds everything with AddressSanitizer and UBSan.
option(SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address,undefined)
endif()

# -DCOUNTERS=OFF compiles every KIT_COUNT(...) away, for timing without counter overhead.
option(COUNTERS "Enable operation counters" ON)
if(COUNTERS)
  add_compile_definitions(KIT_COUNTERS_ENABLED=1)
else()
  add_compile_definitions(KIT_COUNTERS_ENABLED=0)
endif()

# Record the build flags so every benchmark CSV row says how it was built.
string(TOUPPER "${CMAKE_BUILD_TYPE}" _bt)
add_compile_definitions(KIT_BUILD_FLAGS="${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS_${_bt}} sanitize=${SANITIZE} counters=${COUNTERS}")

include_directories(include third_party)

# Your application: every .cpp in src/ except test files. main.cpp must exist.
file(GLOB APP_SOURCES CONFIGURE_DEPENDS src/*.cpp)
if(APP_SOURCES)
  add_executable(app ${APP_SOURCES})
endif()

# Unit tests: every .cpp in tests/. Your headers live in include/ so tests can use them.
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(tests ${TEST_SOURCES})

# Workload generators (provided, do not change).
add_executable(gen_orderbook tools/gen_orderbook.cpp)
add_executable(gen_alerts tools/gen_alerts.cpp)

enable_testing()
add_test(NAME unit_tests COMMAND tests)
