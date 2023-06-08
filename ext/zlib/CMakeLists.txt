message(STATUS "Fetching zlib")

# Like `FetchContent_MakeAvailable` but passes EXCLUDE_FROM_ALL to `add_subdirectory`.
# This is needed because zlib examples assumes it is the top level cmake file, which
# is not the case when using FetchContent
macro(FetchContent_MakeAvailableExcludeFromAll)
    foreach (contentName IN ITEMS ${ARGV})
        string(TOLOWER ${contentName} contentNameLower)
        FetchContent_GetProperties(${contentName})
        if (NOT ${contentNameLower}_POPULATED)
            FetchContent_Populate(${contentName})
            if (EXISTS ${${contentNameLower}_SOURCE_DIR}/CMakeLists.txt)
                add_subdirectory(${${contentNameLower}_SOURCE_DIR}
                        ${${contentNameLower}_BINARY_DIR} EXCLUDE_FROM_ALL)
            endif ()
        endif ()
    endforeach ()
endmacro()

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0135 OLD)
include(FetchContent)

FetchContent_Declare(zlib
        URL https://www.zlib.net/zlib-1.2.13.tar.gz
        https://www.zlib.net/fossils/zlib-1.2.13.tar.gz
        https://github.com/madler/zlib/releases/download/v1.2.13/zlib-1.2.13.tar.gz
        URL_HASH MD5=9b8aa094c4e5765dabf4da391f00d15c
        )
FetchContent_MakeAvailableExcludeFromAll(zlib)

add_library(ZLIB::ZLIB ALIAS zlibstatic)
target_include_directories(zlibstatic INTERFACE ${zlib_BINARY_DIR} ${zlib_SOURCE_DIR})

# 1. Set the variables that are usually set by FindZLIB.cmake.
# 2. Add the module that stubs out `find_package(ZLIB ...)` calls.
set(ZLIB_FOUND ON PARENT_SCOPE)
set(ZLIB_LIBRARY ZLIB::ZLIB PARENT_SCOPE)
set(ZLIB_LIBRARIES ZLIB::ZLIB PARENT_SCOPE)
set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} PARENT_SCOPE)
set(ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} PARENT_SCOPE)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/CMake" PARENT_SCOPE)
