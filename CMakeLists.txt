cmake_minimum_required(VERSION 3.0.0)
project(note_counter VERSION 1.1.0)

# disable warnings about using non _s variants like strncpy
if (MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# add include folders
include_directories(external)

# external libraries
add_subdirectory(external/mxml EXCLUDE_FROM_ALL)

# sources
set(SOURCE_FILES ${SOURCE_FILES} iidx_note_count.c ifs.c iidx_1.c kbinxml.c)

# note_counter.lib
add_library(note_counter STATIC ${SOURCE_FILES})
target_link_libraries(note_counter PRIVATE mxml)

# test.exe
add_executable(test EXCLUDE_FROM_ALL ${SOURCE_FILES} test/main.c)
target_link_libraries(test PRIVATE mxml)
