file(GLOB_RECURSE UNITTESTFILES_ALL CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)


add_executable(common_lib_tests ${UNITTESTFILES_ALL})
if (MSVC)
    target_compile_options(common_lib_tests PRIVATE "/W3")
    target_compile_options(common_lib_tests PRIVATE "/EHsc")
else ()
    target_compile_options(common_lib_tests PRIVATE "-Wall")
    target_compile_options(common_lib_tests PRIVATE "-Wextra")
endif ()


target_include_directories(common_lib_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_lib_tests
        gtest
        gmock_main
        game_common_lib
        )

jt_use_assets(common_lib_tests)
add_test(common_lib_tests common_lib_tests)
