file(GLOB_RECURSE GAMELIBFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_library(game_common_lib STATIC ${GAMELIBFILES})

target_link_libraries(game_common_lib PUBLIC JamTemplateLib asio::asio)

target_include_directories(game_common_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (MSVC)
    target_compile_options(game_common_lib PRIVATE "/W3")
    target_compile_options(game_common_lib PRIVATE "/EHsc")
else ()
    target_compile_options(game_common_lib PRIVATE "-Wall")
    target_compile_options(game_common_lib PRIVATE "-Wextra")
endif ()

