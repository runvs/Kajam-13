file(GLOB_RECURSE GAMELIBFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_library(game_client_lib STATIC ${GAMELIBFILES})

target_link_libraries(game_client_lib PUBLIC game_common_lib)

target_include_directories(game_client_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (MSVC)
    target_compile_options(game_client_lib PRIVATE "/W3")
    target_compile_options(game_client_lib PRIVATE "/EHsc")
else ()
    target_compile_options(game_client_lib PRIVATE "-Wall")
    target_compile_options(game_client_lib PRIVATE "-Wextra")
endif ()

