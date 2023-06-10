include_guard(GLOBAL)

# helper script to copy contents of folder to destination
# Note: we want to copy files only if they changed
set(script_copy_directory "${CMAKE_BINARY_DIR}/_cmake/copy_directory.cmake")
file(WRITE "${script_copy_directory}" "
    if(NOT IS_DIRECTORY \"\${DIR_SRC}\" OR \"\${DIR_DST}\" STREQUAL \"\")
        message(FATAL_ERROR \"Internal error while calling copy_directory script!
            src[\${DIR_SRC}]
            dst[\${DIR_DST}]
        \")
    endif()
    file(GLOB_RECURSE files RELATIVE \"\${DIR_SRC}\" \"\${DIR_SRC}/*.*\")
    foreach(entry \${files})
        set(src \"\${DIR_SRC}/\${entry}\")
        set(dst \"\${DIR_DST}/\${entry}\")
        file(TIMESTAMP \"\${src}\" src_ts UTC)
        file(TIMESTAMP \"\${dst}\" dst_ts UTC)
        if(src_ts STRGREATER dst_ts)
            execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy \"\${src}\" \"\${dst}\")
        endif()
    endforeach()
")

function(jt_use_assets TGT)
    add_custom_command(TARGET ${TGT} PRE_BUILD
            COMMAND ${CMAKE_COMMAND}
                "-DDIR_SRC=${CMAKE_SOURCE_DIR}/assets"
                "-DDIR_DST=${CMAKE_CURRENT_BINARY_DIR}/assets"
                -P "${script_copy_directory}")

    if (MSVC)
        add_custom_command(TARGET ${TGT} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} 
                "-DDIR_SRC=${CMAKE_SOURCE_DIR}/assets"
                "-DDIR_DST=${CMAKE_CURRENT_BINARY_DIR}/Debug/assets"
                -P "${script_copy_directory}")
        add_custom_command(TARGET ${TGT} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} 
                "-DDIR_SRC=${CMAKE_SOURCE_DIR}/assets"
                "-DDIR_DST=${CMAKE_CURRENT_BINARY_DIR}/Release/assets"
                -P "${script_copy_directory}")
    endif ()
endfunction()

function(target_link_libraries_system target)
    set(libs ${ARGN})
    foreach (lib ${libs})
        get_target_property(lib_include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
        target_include_directories(${target} SYSTEM PRIVATE ${lib_include_dirs})
        target_link_libraries(${target} ${lib})
    endforeach (lib)
endfunction(target_link_libraries_system)
