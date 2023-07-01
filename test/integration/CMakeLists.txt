file(GLOB_RECURSE INTEGRATIONTESTFILES_ALL CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*)


add_executable(integration_test ${INTEGRATIONTESTFILES_ALL})
if (MSVC)
    target_compile_options(integration_test PRIVATE "/W3")
    target_compile_options(integration_test PRIVATE "/EHsc")
else ()
    target_compile_options(integration_test PRIVATE "-Wall")
    target_compile_options(integration_test PRIVATE "-Wextra")
endif ()


target_include_directories(integration_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(integration_test
        gtest
        gmock_main
        game_common_lib
        game_server_lib
        game_client_lib
        )

jt_use_assets(integration_test)
add_test(integration_test integration_test)
