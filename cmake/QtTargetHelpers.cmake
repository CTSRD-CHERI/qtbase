# This function can be used to add sources/libraries/etc. to the specified CMake target
# if the provided CONDITION evaluates to true.
function(qt_extend_target target)
    # Don't try to extend_target when cross compiling an imported host target (like a tool).
    qt_is_imported_target("${target}" is_imported)
    if(is_imported)
        return()
    endif()

    if (NOT TARGET "${target}")
        message(FATAL_ERROR "Trying to extend non-existing target \"${target}\".")
    endif()
    qt_parse_all_arguments(arg "qt_extend_target" "HEADER_MODULE" "PRECOMPILED_HEADER"
        "CONDITION;${__default_public_args};${__default_private_args};${__default_private_module_args};COMPILE_FLAGS;NO_PCH_SOURCES" ${ARGN})
    if ("x${arg_CONDITION}" STREQUAL x)
        set(arg_CONDITION ON)
    endif()

    qt_evaluate_config_expression(result ${arg_CONDITION})
    if (${result})
        if(QT_CMAKE_DEBUG_EXTEND_TARGET)
            message("qt_extend_target(${target} CONDITION ${arg_CONDITION} ...): Evaluated")
        endif()
        set(dbus_sources "")
        foreach(adaptor ${arg_DBUS_ADAPTOR_SOURCES})
            qt_create_qdbusxml2cpp_command("${target}" "${adaptor}" ADAPTOR BASENAME "${arg_DBUS_ADAPTOR_BASENAME}" FLAGS "${arg_DBUS_ADAPTOR_FLAGS}")
            list(APPEND dbus_sources "${sources}")
        endforeach()

        foreach(interface ${arg_DBUS_INTERFACE_SOURCES})
            qt_create_qdbusxml2cpp_command("${target}" "${interface}" INTERFACE BASENAME "${arg_DBUS_INTERFACE_BASENAME}" FLAGS "${arg_DBUS_INTERFACE_FLAGS}")
            list(APPEND dbus_sources "${sources}")
        endforeach()

        get_target_property(target_type ${target} TYPE)
        set(is_library FALSE)
        if (${target_type} STREQUAL "STATIC_LIBRARY" OR ${target_type} STREQUAL "SHARED_LIBRARY")
            set(is_library TRUE)
        endif()
        foreach(lib ${arg_PUBLIC_LIBRARIES} ${arg_LIBRARIES})
            # Automatically generate PCH for 'target' using dependencies
            # if 'target' is a library/module!
            if (${is_library})
                qt_update_precompiled_header_with_library("${target}" "${lib}")
            endif()

            string(REGEX REPLACE "_nolink$" "" base_lib "${lib}")
            if(NOT base_lib STREQUAL lib)
                qt_create_nolink_target("${base_lib}" ${target})
            endif()
        endforeach()

        # Set-up the target
        target_sources("${target}" PRIVATE ${arg_SOURCES} ${dbus_sources})
        if (arg_COMPILE_FLAGS)
            set_source_files_properties(${arg_SOURCES} PROPERTIES COMPILE_FLAGS "${arg_COMPILE_FLAGS}")
        endif()

        set(public_visibility_option "PUBLIC")
        set(private_visibility_option "PRIVATE")
        if(arg_HEADER_MODULE)
            set(public_visibility_option "INTERFACE")
            set(private_visibility_option "INTERFACE")
        endif()
        target_include_directories("${target}"
                                   ${public_visibility_option} ${arg_PUBLIC_INCLUDE_DIRECTORIES}
                                   ${private_visibility_option} ${arg_INCLUDE_DIRECTORIES})
        target_compile_definitions("${target}"
                                    ${public_visibility_option} ${arg_PUBLIC_DEFINES}
                                    ${private_visibility_option} ${arg_DEFINES})
        target_link_libraries("${target}"
                              ${public_visibility_option} ${arg_PUBLIC_LIBRARIES}
                              ${private_visibility_option} ${arg_LIBRARIES})
        target_compile_options("${target}"
                               ${public_visibility_option} ${arg_PUBLIC_COMPILE_OPTIONS}
                               ${private_visibility_option} ${arg_COMPILE_OPTIONS})
        target_link_options("${target}"
                            ${public_visibility_option} ${arg_PUBLIC_LINK_OPTIONS}
                            ${private_visibility_option} ${arg_LINK_OPTIONS})

        if(NOT arg_HEADER_MODULE)
            set_property (TARGET "${target}" APPEND PROPERTY
                AUTOMOC_MOC_OPTIONS "${arg_MOC_OPTIONS}"
            )
        endif()

        # When computing the private library dependencies, we need to check not only the known
        # modules added by this repo's qt_build_repo(), but also all module dependencies that
        # were found via find_package().
        qt_internal_get_qt_all_known_modules(known_modules)

        # When a public module depends on a private module (Gui on CorePrivate)
        # make its private module depend on the other private module (GuiPrivate will depend on
        # CorePrivate).
        set(qt_libs_private "")
        foreach(it ${known_modules})
            list(FIND arg_LIBRARIES "Qt::${it}Private" pos)
            if(pos GREATER -1)
                list(APPEND qt_libs_private "Qt::${it}Private")
            endif()
        endforeach()

        set(target_private "${target}Private")
        if(TARGET "${target_private}")
          target_link_libraries("${target_private}"
                                INTERFACE ${arg_PRIVATE_MODULE_INTERFACE})
        endif()
        qt_register_target_dependencies("${target}"
                                        "${arg_PUBLIC_LIBRARIES}"
                                        "${qt_libs_private};${arg_LIBRARIES}")


        qt_autogen_tools(${target}
                         ENABLE_AUTOGEN_TOOLS ${arg_ENABLE_AUTOGEN_TOOLS}
                         DISABLE_AUTOGEN_TOOLS ${arg_DISABLE_AUTOGEN_TOOLS})

        qt_update_precompiled_header("${target}" "${arg_PRECOMPILED_HEADER}")
        qt_update_ignore_pch_source("${target}" "${arg_NO_PCH_SOURCES}")
        ## Ignore objective-c files for PCH (not supported atm)
        qt_ignore_pch_obj_c_sources("${target}" "${arg_SOURCES}")

    else()
        if(QT_CMAKE_DEBUG_EXTEND_TARGET)
            message("qt_extend_target(${target} CONDITION ${arg_CONDITION} ...): Skipped")
        endif()
    endif()
endfunction()

function(qt_is_imported_target target out_var)
    if(NOT TARGET "${target}")
        set(target "${QT_CMAKE_EXPORT_NAMESPACE}::${target}")
    endif()
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "Invalid target given to qt_is_imported_target: ${target}")
    endif()
    get_target_property(is_imported "${target}" IMPORTED)
    set(${out_var} "${is_imported}" PARENT_SCOPE)
endfunction()

# Add Qt::target and Qt6::target as aliases for the target
function(qt_internal_add_target_aliases target)
    get_target_property(type "${target}" TYPE)
    if (type STREQUAL EXECUTABLE)
        add_executable("Qt::${target}" ALIAS "${target}")
        add_executable("Qt${PROJECT_VERSION_MAJOR}::${target}" ALIAS "${target}")
    else()
        add_library("Qt::${target}" ALIAS "${target}")
        add_library("Qt${PROJECT_VERSION_MAJOR}::${target}" ALIAS "${target}")
    endif()
endfunction()

function(qt_get_cmake_configurations out_var)
    set(possible_configs "${CMAKE_BUILD_TYPE}")
    if(CMAKE_CONFIGURATION_TYPES)
        set(possible_configs "${CMAKE_CONFIGURATION_TYPES}")
    endif()
    set(${out_var} "${possible_configs}" PARENT_SCOPE)
endfunction()

function(qt_clone_property_for_configs target property configs)
    get_target_property(value "${target}" "${property}")
    foreach(config ${configs})
        string(TOUPPER "${config}" upper_config)
        set_property(TARGET "${target}" PROPERTY "${property}_${upper_config}" "${value}")
    endforeach()
endfunction()

function(qt_handle_multi_config_output_dirs target)
    qt_get_cmake_configurations(possible_configs)
    qt_clone_property_for_configs(${target} LIBRARY_OUTPUT_DIRECTORY "${possible_configs}")
    qt_clone_property_for_configs(${target} RUNTIME_OUTPUT_DIRECTORY "${possible_configs}")
    qt_clone_property_for_configs(${target} ARCHIVE_OUTPUT_DIRECTORY "${possible_configs}")
endfunction()

# Set target properties that are the same for all modules, plugins, executables
# and 3rdparty libraries.
function(qt_set_common_target_properties target)
   if(FEATURE_static_runtime)
       if(MSVC)
           set_property(TARGET ${target} PROPERTY
               MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
       elseif(MINGW)
           target_link_options(${target} INTERFACE "LINKER:-static")
       endif()
   endif()
endfunction()

# Set common, informational target properties.
#
# On Windows, these properties are used to generate the version information resource.
function(qt_set_target_info_properties target)
    cmake_parse_arguments(arg "" "${__default_target_info_args}" "" ${ARGN})
    if("${arg_TARGET_VERSION}" STREQUAL "")
        set(arg_TARGET_VERSION "${PROJECT_VERSION}.0")
    endif()
    if("${arg_TARGET_PRODUCT}" STREQUAL "")
        set(arg_TARGET_PRODUCT "Qt6")
    endif()
    if("${arg_TARGET_DESCRIPTION}" STREQUAL "")
        set(arg_TARGET_DESCRIPTION "C++ Application Development Framework")
    endif()
    if("${arg_TARGET_COMPANY}" STREQUAL "")
        set(arg_TARGET_COMPANY "The Qt Company Ltd.")
    endif()
    if("${arg_TARGET_COPYRIGHT}" STREQUAL "")
        set(arg_TARGET_COPYRIGHT "Copyright (C) 2020 The Qt Company Ltd.")
    endif()
    set_target_properties(${target} PROPERTIES
        QT_TARGET_VERSION "${arg_TARGET_VERSION}"
        QT_TARGET_COMPANY_NAME "${arg_TARGET_COMPANY}"
        QT_TARGET_DESCRIPTION "${arg_TARGET_DESCRIPTION}"
        QT_TARGET_COPYRIGHT "${arg_TARGET_COPYRIGHT}"
        QT_TARGET_PRODUCT_NAME "${arg_TARGET_PRODUCT}")
endfunction()

# Uses the QT_DELAYED_TARGET_* property values to set the final QT_TARGET_* properties.
# Needed when doing executable finalization at the end of a subdirectory scope
# (aka scope finalization).
function(qt_internal_set_target_info_properties_from_delayed_properties target)
    set(args "")
    foreach(prop ${__default_target_info_args})
        get_target_property(prop_value "${target}" "QT_DELAYED_${prop}")
        list(APPEND args "${prop}" "${prop_value}")
    endforeach()
    qt_set_target_info_properties(${target} ${args})
endfunction()

# Updates the QT_DELAYED_ properties with values from the QT_ variants, in case if they were
# set in-between a qt_add_* call and before scope finalization.
function(qt_internal_update_delayed_target_info_properties target)
    foreach(prop ${__default_target_info_args})
        get_target_property(prop_value "${target}" "QT_${prop}")
        get_target_property(delayed_prop_value ${target} "QT_DELAYED_${prop}")
        set(final_value "${delayed_prop_value}")
        if(prop_value)
            set(final_value "${prop_value}")
        endif()
        set_target_properties(${target} PROPERTIES "QT_DELAYED_${prop}" "${final_value}")
    endforeach()
endfunction()

function(qt_internal_check_directory_or_type name dir type default result_var)
    if ("x${dir}" STREQUAL x)
        if("x${type}" STREQUAL x)
            message(FATAL_ERROR "qt_internal_add_plugin called without setting either TYPE or ${name}.")
        endif()
        set(${result_var} "${default}" PARENT_SCOPE)
    else()
        set(${result_var} "${dir}" PARENT_SCOPE)
    endif()
endfunction()

function(qt_internal_apply_win_prefix_and_suffix target)
    if(WIN32)
        # Table of prefix / suffixes for MSVC libraries as qmake expects them to be created.
        # static - Qt6EdidSupport.lib (platform support libraries / or static QtCore, etc)
        # shared - Qt6Core.dll
        # shared import library - Qt6Core.lib
        # module aka Qt plugin - qwindows.dll
        # module import library - qwindows.lib
        #
        # The CMake defaults are fine for us.

        # Table of prefix / suffixes for MinGW libraries as qmake expects them to be created.
        # static - libQt6EdidSupport.a (platform support libraries / or static QtCore, etc)
        # shared - Qt6Core.dll
        # shared import library - libQt6Core.a
        # module aka Qt plugin - qwindows.dll
        # module import library - libqwindows.a
        #
        # CMake for Windows-GNU platforms defaults the prefix to "lib".
        # CMake for Windows-GNU platforms defaults the import suffix to ".dll.a".
        # These CMake defaults are not ok for us.

        # This should cover both MINGW with GCC and CLANG.
        if(NOT MSVC)
            set_property(TARGET "${target}" PROPERTY IMPORT_SUFFIX ".a")

            get_target_property(target_type ${target} TYPE)
            if(target_type STREQUAL "STATIC_LIBRARY")
                set_property(TARGET "${target}" PROPERTY PREFIX "lib")
            else()
                set_property(TARGET "${target}" PROPERTY PREFIX "")
                set_property(TARGET "${target}" PROPERTY IMPORT_PREFIX "lib")
            endif()
        endif()
    endif()
endfunction()

function(qt_internal_strip_target_directory_scope_token target out_var)
    # In CMake versions earlier than CMake 3.18, a subdirectory scope id is appended to the
    # target name if the target is referenced in a target_link_libraries command from a
    # different directory scope than where the target was created.
    # Strip it.
    #
    # For informational purposes, in CMake 3.18, the target name looks as follows:
    # ::@(0x5604cb3f6b50);Threads::Threads;::@
    # This case doesn't have to be stripped (at least for now), because when we iterate over
    # link libraries, the tokens appear as separate target names.
    #
    # Example: Threads::Threads::@<0x5604cb3f6b50>
    # Output: Threads::Threads
    string(REGEX REPLACE "::@<.+>$" "" target "${target}")
    set("${out_var}" "${target}" PARENT_SCOPE)
endfunction()

function(qt_internal_export_modern_cmake_config_targets_file)
    cmake_parse_arguments(__arg "" "EXPORT_NAME_PREFIX;CONFIG_INSTALL_DIR" "TARGETS" ${ARGN})

    set(export_name "${__arg_EXPORT_NAME_PREFIX}VersionlessTargets")
    foreach(target ${__arg_TARGETS})
        if (TARGET "${target}Versionless")
            continue()
        endif()

        add_library("${target}Versionless" INTERFACE)
        target_link_libraries("${target}Versionless" INTERFACE "${target}")
        set_target_properties("${target}Versionless" PROPERTIES
            EXPORT_NAME "${target}"
            _qt_is_versionless_target "TRUE")
        set_property(TARGET "${target}Versionless"
                     APPEND PROPERTY EXPORT_PROPERTIES _qt_is_versionless_target)

        qt_install(TARGETS "${target}Versionless" EXPORT ${export_name})
    endforeach()
    qt_install(EXPORT ${export_name} NAMESPACE Qt:: DESTINATION "${__arg_CONFIG_INSTALL_DIR}")
endfunction()

function(qt_create_tracepoints name tracepoints_file)
    string(TOLOWER "${name}" provider_name)
    string(PREPEND provider_name "qt")
    set(header_filename "${provider_name}_tracepoints_p.h")
    set(header_path "${CMAKE_CURRENT_BINARY_DIR}/${header_filename}")

    if(QT_FEATURE_lttng OR QT_FEATURE_etw)
        set(source_path "${CMAKE_CURRENT_BINARY_DIR}/${provider_name}_tracepoints.cpp")
        qt_configure_file(OUTPUT "${source_path}"
            CONTENT "#define TRACEPOINT_CREATE_PROBES
#define TRACEPOINT_DEFINE
#define include \"${header_filename}\"")
        target_sources(${name} PRIVATE "${source_path}")
        target_compile_definitions(${name} PRIVATE Q_TRACEPOINT)

        if(QT_FEATURE_lttng)
            set(tracegen_arg "lttng")
            target_link_libraries(${name} PRIVATE LTTng::UST)
        elseif(QT_FEATURE_etw)
            set(tracegen_arg "etw")
        endif()

        qt_get_tool_target_name(tracegen_target tracegen)
        get_filename_component(tracepoints_filepath "${tracepoints_file}" ABSOLUTE)
        add_custom_command(OUTPUT "${header_path}"
            COMMAND ${tracegen_target} ${tracegen_arg} "${tracepoints_filepath}" "${header_path}"
            VERBATIM)
        add_custom_target(${name}_tracepoints_header DEPENDS "${header_path}")
        add_dependencies(${name} ${name}_tracepoints_header)
    else()
        qt_configure_file(OUTPUT "${header_path}" CONTENT "#include <private/qtrace_p.h>\n")
    endif()
endfunction()
