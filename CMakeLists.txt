cmake_minimum_required(VERSION 3.0.0)
project(iidx_notes VERSION 1.0.1)

# add include folders
#include_directories()
include_directories(external)

# external libraries
add_subdirectory(external/mxml EXCLUDE_FROM_ALL)

# sources
set(SOURCE_FILES ${SOURCE_FILES} iidx_note_count.c ifs.c iidx_1.c kbinxml.c binary_stream.c)

# iidx_notes.lib
add_library(iidx_notes STATIC ${SOURCE_FILES})
target_link_libraries(iidx_notes PRIVATE mxml)

# test.exe
add_executable(test EXCLUDE_FROM_ALL ${SOURCE_FILES} test/main.c)
target_link_libraries(test PRIVATE mxml)
