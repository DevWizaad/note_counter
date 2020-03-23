cmake_minimum_required(VERSION 3.0.0)
project(mxml VERSION 3.2.0)

set (SOURCE_FILES ${SOURCE_FILES} mxml-attr.c mxml-entity.c mxml-file.c mxml-get.c mxml-index.c mxml-node.c mxml-private.c mxml-search.c mxml-set.c mxml-string.c)

add_library(mxml STATIC ${SOURCE_FILES})
