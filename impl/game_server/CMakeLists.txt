file(GLOB GAMEFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_executable(game_server ${GAMEFILES})

target_link_libraries(game_server game_server_lib)
target_include_directories(game_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(game_server PRIVATE "/W3")
    target_compile_options(game_server PRIVATE "/EHsc")
else ()
    target_compile_options(game_server PRIVATE "-Wall")
    target_compile_options(game_server PRIVATE "-Wextra")
endif ()


jt_use_assets(game_server)
