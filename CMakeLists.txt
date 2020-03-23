cmake_minimum_required(VERSION 3.0.0)
project(iidx_notes VERSION 1.0.1)

# add include folders
include_directories(external)

# set project sources
set(SOURCE_FILES ${SOURCE_FILES} iidx_note_count.c ifs.c iidx_1.c kbinxml.c binary_stream.c)

# mxml
set(SOURCE_FILES ${SOURCE_FILES} external/mxml/mxml-attr.c external/mxml/mxml-entity.c external/mxml/mxml-file.c external/mxml/mxml-get.c external/mxml/mxml-index.c external/mxml/mxml-node.c external/mxml/mxml-private.c external/mxml/mxml-search.c external/mxml/mxml-set.c external/mxml/mxml-string.c)

# executable
#add_executable(iidx_notes ${SOURCE_FILES})

# library
add_library(iidx_notes STATIC ${SOURCE_FILES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
