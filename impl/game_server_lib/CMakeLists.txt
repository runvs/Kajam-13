file(GLOB_RECURSE GAMELIBFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_library(game_server_lib STATIC ${GAMELIBFILES})

target_link_libraries(game_server_lib PUBLIC game_common_lib)

target_include_directories(game_server_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (MSVC)
    target_compile_options(game_server_lib PRIVATE "/W3")
    target_compile_options(game_server_lib PRIVATE "/EHsc")
else ()
    target_compile_options(game_server_lib PRIVATE "-Wall")
    target_compile_options(game_server_lib PRIVATE "-Wextra")
endif ()

