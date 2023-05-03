file(GLOB GAMEFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_executable(game_client ${GAMEFILES})
if (JT_ENABLE_WEB)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif ()

target_link_libraries(game_client game_client_lib)
target_include_directories(game_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(game_client PRIVATE "/W3")
    target_compile_options(game_client PRIVATE "/EHsc")
else ()
    target_compile_options(game_client PRIVATE "-Wall")
    target_compile_options(game_client PRIVATE "-Wextra")
endif ()


jt_use_assets(game_client)
