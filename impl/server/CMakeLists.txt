file(GLOB GAMEFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_executable(${PROJECTNAME}_server ${GAMEFILES})

target_link_libraries(${PROJECTNAME}_server game_server_lib)
target_include_directories(${PROJECTNAME}_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(${PROJECTNAME}_server PRIVATE "/W3")
    target_compile_options(${PROJECTNAME}_server PRIVATE "/EHsc")
else ()
    target_compile_options(${PROJECTNAME}_server PRIVATE "-Wall")
    target_compile_options(${PROJECTNAME}_server PRIVATE "-Wextra")
endif ()


jt_use_assets(${PROJECTNAME}_server)
