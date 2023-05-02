file(GLOB_RECURSE GAMELIBFILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)

add_library(GameLib_common STATIC ${GAMELIBFILES})

target_link_libraries(GameLib_common PUBLIC JamTemplateLib asio::asio)

target_include_directories(GameLib_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (MSVC)
    target_compile_options(GameLib_common PRIVATE "/W3")
    target_compile_options(GameLib_common PRIVATE "/EHsc")
else ()
    target_compile_options(GameLib_common PRIVATE "-Wall")
    target_compile_options(GameLib_common PRIVATE "-Wextra")
endif ()

