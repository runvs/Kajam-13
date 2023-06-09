configure_file(${CMAKE_SOURCE_DIR}/ext/googletest/CMakeLists.txt.in ${CMAKE_BINARY_DIR}/ext/googletest/googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ext/googletest/googletest-download)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ext/googletest/googletest-download)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/ext/googletest/googletest-src
        ${CMAKE_BINARY_DIR}/ext/googletest/googletest-build
        EXCLUDE_FROM_ALL)


add_subdirectory(unit)
add_subdirectory(integration)
