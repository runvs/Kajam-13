message(STATUS "Add FetchContent")
include(FetchContent)

message(STATUS "Add CMP.cmake")
# download CPM.cmake
file(
        DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
        ${CMAKE_BINARY_DIR}/cmake/CPM.cmake
        EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

message(STATUS "Fetching external libraries started")
add_subdirectory(asio)
CPMAddPackage(
        NAME aselib
        GITHUB_REPOSITORY Laguna1989/aselib
        GIT_TAG v0.1.0
        OPTIONS
        "ASE_LIB_ENABLE_UNIT_TESTS OFF"
)
add_subdirectory(box2d)
add_subdirectory(catch2)
add_subdirectory(imgui)
add_subdirectory(openalpp)
add_subdirectory(tileson)
if (USE_SFML)
    add_subdirectory(sfml)
    add_subdirectory(imgui-sfml)
else ()
    add_subdirectory(imgui-sdl)
endif ()

message(STATUS "Fetching external libraries done")
