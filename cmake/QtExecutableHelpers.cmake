# This function creates a CMake target for a generic console or GUI binary.
# Please consider to use a more specific version target like the one created
# by qt_add_test or qt_add_tool below.
function(qt_add_executable name)
    qt_parse_all_arguments(arg "qt_add_executable"
        "${__qt_add_executable_optional_args}"
        "${__qt_add_executable_single_args}"
        "${__qt_add_executable_multi_args}"
        ${ARGN})

    if ("x${arg_OUTPUT_DIRECTORY}" STREQUAL "x")
        set(arg_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_BINDIR}")
    endif()

    get_filename_component(arg_OUTPUT_DIRECTORY "${arg_OUTPUT_DIRECTORY}"
        ABSOLUTE BASE_DIR "${QT_BUILD_DIR}")

    if ("x${arg_INSTALL_DIRECTORY}" STREQUAL "x")
        set(arg_INSTALL_DIRECTORY "${INSTALL_BINDIR}")
    endif()

    if (ANDROID)
        add_library("${name}" MODULE)
        qt_android_apply_arch_suffix("${name}")
        qt_android_generate_deployment_settings("${name}")
        qt_android_add_apk_target("${name}")
        # On our qmake builds we don't compile the executables with
        # visibility=hidden. Not having this flag set will cause the
        # executable to have main() hidden and can then no longer be loaded
        # through dlopen()
        set_property(TARGET ${name} PROPERTY C_VISIBILITY_PRESET default)
        set_property(TARGET ${name} PROPERTY CXX_VISIBILITY_PRESET default)
    else()
        add_executable("${name}" ${arg_EXE_FLAGS})
    endif()

    if (arg_VERSION)
        if(arg_VERSION MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+")
            # nothing to do
        elseif(arg_VERSION MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+")
            set(arg_VERSION "${arg_VERSION}.0")
        elseif(arg_VERSION MATCHES "[0-9]+\\.[0-9]+")
            set(arg_VERSION "${arg_VERSION}.0.0")
        elseif (arg_VERSION MATCHES "[0-9]+")
            set(arg_VERSION "${arg_VERSION}.0.0.0")
        else()
            message(FATAL_ERROR "Invalid version format")
        endif()
    endif()

    if(arg_DELAY_TARGET_INFO)
        # Delay the setting of target info properties if requested. Needed for scope finalization
        # of Qt apps.
        set_target_properties("${name}" PROPERTIES
            QT_DELAYED_TARGET_VERSION "${arg_VERSION}"
            QT_DELAYED_TARGET_PRODUCT "${arg_TARGET_PRODUCT}"
            QT_DELAYED_TARGET_DESCRIPTION "${arg_TARGET_DESCRIPTION}"
            QT_DELAYED_TARGET_COMPANY "${arg_TARGET_COMPANY}"
            QT_DELAYED_TARGET_COPYRIGHT "${arg_TARGET_COPYRIGHT}"
            )
    else()
        if("${arg_TARGET_DESCRIPTION}" STREQUAL "")
            set(arg_TARGET_DESCRIPTION "Qt ${name}")
        endif()
        qt_set_target_info_properties(${name} ${ARGN}
            TARGET_DESCRIPTION "${arg_TARGET_DESCRIPTION}"
            TARGET_VERSION "${arg_VERSION}")
    endif()

    if (WIN32 AND NOT arg_DELAY_RC)
        qt6_generate_win32_rc_file(${name})
    endif()

    qt_set_common_target_properties(${name})
    qt_autogen_tools_initial_setup(${name})
    qt_skip_warnings_are_errors_when_repo_unclean("${name}")

    set(extra_libraries "")
    if(NOT arg_BOOTSTRAP AND NOT arg_NO_QT)
        set(extra_libraries "Qt::Core")
    endif()

    set(private_includes
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}"
         ${arg_INCLUDE_DIRECTORIES}
    )

    qt_extend_target("${name}"
        SOURCES ${arg_SOURCES}
        INCLUDE_DIRECTORIES ${private_includes}
        DEFINES ${arg_DEFINES}
        LIBRARIES ${arg_LIBRARIES} Qt::PlatformCommonInternal
        PUBLIC_LIBRARIES ${extra_libraries} ${arg_PUBLIC_LIBRARIES}
        DBUS_ADAPTOR_SOURCES "${arg_DBUS_ADAPTOR_SOURCES}"
        DBUS_ADAPTOR_FLAGS "${arg_DBUS_ADAPTOR_FLAGS}"
        DBUS_INTERFACE_SOURCES "${arg_DBUS_INTERFACE_SOURCES}"
        DBUS_INTERFACE_FLAGS "${arg_DBUS_INTERFACE_FLAGS}"
        COMPILE_OPTIONS ${arg_COMPILE_OPTIONS}
        LINK_OPTIONS ${arg_LINK_OPTIONS}
        MOC_OPTIONS ${arg_MOC_OPTIONS}
        ENABLE_AUTOGEN_TOOLS ${arg_ENABLE_AUTOGEN_TOOLS}
        DISABLE_AUTOGEN_TOOLS ${arg_DISABLE_AUTOGEN_TOOLS}
    )
    set_target_properties("${name}" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${arg_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY "${arg_OUTPUT_DIRECTORY}"
        WIN32_EXECUTABLE "${arg_GUI}"
        MACOSX_BUNDLE "${arg_GUI}"
    )
    if(NOT ${arg_EXCEPTIONS})
        qt_internal_set_no_exceptions_flags("${name}")
    endif()

    # Check if target needs to be excluded from all target. Also affects qt_install.
    # Set by qt_exclude_tool_directories_from_default_target.
    set(exclude_from_all FALSE)
    if(__qt_exclude_tool_directories)
        foreach(absolute_dir ${__qt_exclude_tool_directories})
            string(FIND "${CMAKE_CURRENT_SOURCE_DIR}" "${absolute_dir}" dir_starting_pos)
            if(dir_starting_pos EQUAL 0)
                set(exclude_from_all TRUE)
                set_target_properties("${name}" PROPERTIES EXCLUDE_FROM_ALL TRUE)
                break()
            endif()
        endforeach()
    endif()

    if(NOT arg_NO_INSTALL)
        set(additional_install_args "")
        if(exclude_from_all)
            list(APPEND additional_install_args EXCLUDE_FROM_ALL COMPONENT "ExcludedExecutables")
        endif()

        qt_get_cmake_configurations(cmake_configs)
        foreach(cmake_config ${cmake_configs})
            qt_get_install_target_default_args(
                OUT_VAR install_targets_default_args
                CMAKE_CONFIG "${cmake_config}"
                ALL_CMAKE_CONFIGS "${cmake_configs}"
                RUNTIME "${arg_INSTALL_DIRECTORY}"
                LIBRARY "${arg_INSTALL_DIRECTORY}"
                BUNDLE "${arg_INSTALL_DIRECTORY}")
            qt_install(TARGETS "${name}"
                       ${additional_install_args} # Needs to be before the DESTINATIONS.
                       CONFIGURATIONS ${cmake_config}
                       ${install_targets_default_args})
        endforeach()
    endif()
endfunction()
