file(GLOB GAMEFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_executable(${PROJECTNAME}_client ${GAMEFILES})
if (JT_ENABLE_WEB)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif ()

target_link_libraries(${PROJECTNAME}_client game_client_lib)
target_include_directories(${PROJECTNAME}_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(${PROJECTNAME}_client PRIVATE "/W3")
    target_compile_options(${PROJECTNAME}_client PRIVATE "/EHsc")
else ()
    target_compile_options(${PROJECTNAME}_client PRIVATE "-Wall")
    target_compile_options(${PROJECTNAME}_client PRIVATE "-Wextra")
endif ()


jt_use_assets(${PROJECTNAME}_client)
