function(qt_add_docs)
    if(${ARGC} EQUAL 1)
        # Function called from old generated CMakeLists.txt that was missing the target parameter
        return()
    endif()
    if(NOT ${ARGC} EQUAL 2)
        message(FATAL_ERROR "qt_add_docs called with the wrong number of arguments. Should be qt_add_docs(target path_to_project.qdocconf).")
        return()
    endif()
    set(target ${ARGV0})
    set(doc_project ${ARGV1})

    # If a target is not built (which can happen for tools when crosscompiling, we shouldn't try
    # to generate docs.
    if(NOT TARGET "${target}")
        return()
    endif()

    if (NOT QT_SUPERBUILD OR QT_WILL_INSTALL)
        set(doc_tools_dir "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/${INSTALL_BINDIR}")
    else()
        set(doc_tools_dir "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/qtbase/${INSTALL_BINDIR}")
    endif()

    set(qdoc_bin "${doc_tools_dir}/qdoc${CMAKE_EXECUTABLE_SUFFIX}")
    set(qtattributionsscanner_bin "${doc_tools_dir}/qtattributionsscanner${CMAKE_EXECUTABLE_SUFFIX}")
    set(qhelpgenerator_bin "${doc_tools_dir}/qhelpgenerator${CMAKE_EXECUTABLE_SUFFIX}")

    get_target_property(target_type ${target} TYPE)
    if (NOT target_type STREQUAL "INTERFACE_LIBRARY")
        get_target_property(target_bin_dir ${target} BINARY_DIR)
        get_target_property(target_source_dir ${target} SOURCE_DIR)
    else()
        set(target_bin_dir ${CMAKE_CURRENT_BINARY_DIR})
        set(target_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    set(doc_ouput_dir "${target_bin_dir}/.doc")


    # Generate include dir list
    set(target_include_dirs_file "${doc_ouput_dir}/$<CONFIG>/includes.txt")

    set(include_paths_property "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>")
    if (NOT target_type STREQUAL "INTERFACE_LIBRARY")
        file(GENERATE
            OUTPUT ${target_include_dirs_file}
            CONTENT "-I$<JOIN:${include_paths_property},\n-I>"
        )
        set(include_path_args "@${target_include_dirs_file}")
    else()
        set(include_path_args "")
    endif()

    get_filename_component(doc_target "${doc_project}" NAME_WLE)
    if (QT_WILL_INSTALL)
        set(qdoc_output_dir "${CMAKE_BINARY_DIR}/${INSTALL_DOCDIR}/${doc_target}")
        set(index_dir "${CMAKE_BINARY_DIR}/${INSTALL_DOCDIR}")
    elseif (QT_SUPERBUILD)
        set(qdoc_output_dir "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/qtbase/${INSTALL_DOCDIR}/${doc_target}")
        set(index_dir "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/qtbase/${INSTALL_DOCDIR}")
    else()
        set(qdoc_output_dir "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/${INSTALL_DOCDIR}/${doc_target}")
        set(index_dir "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/${INSTALL_DOCDIR}")
    endif()

    # qtattributionsscanner
    add_custom_target(qattributionsscanner_${target}
        DEPENDS ${qattributionsscanner_bin}
        COMMAND ${qtattributionsscanner_bin}
        ${PROJECT_SOURCE_DIR}
        --filter "QDocModule=${doc_target}"
        -o "${target_bin_dir}/codeattributions.qdoc"
    )

    # prepare docs target
    set(prepare_qdoc_args
        -outputdir "${qdoc_output_dir}"
        -installdir "${QT_INSTALL_DIR}/${INSTALL_DOCDIR}"
        "${target_source_dir}/${doc_project}"
        -prepare
        -indexdir "${index_dir}"
        -no-link-errors
        "${include_path_args}"
    )

    if (QT_WILL_INSTALL)
        set(qt_install_docs_env "${CMAKE_INSTALL_PREFIX}/${INSTALL_DOCDIR}")
    elseif (QT_SUPERBUILD)
        set(qt_install_docs_env "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/qtbase/${INSTALL_DOCDIR}")
    else()
        set(qt_install_docs_env "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/${INSTALL_DOCDIR}")
    endif()

    set(qdoc_env_args
        "QT_INSTALL_DOCS=\"${qt_install_docs_env}\""
        "QT_VERSION=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        "QT_VER=${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        "QT_VERSION_TAG=${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}"
        "BUILDDIR=${target_bin_dir}"
    )

    add_custom_target(prepare_docs_${target}
        DEPENDS ${qdoc_bin}
        COMMAND ${CMAKE_COMMAND} -E env ${qdoc_env_args}
        ${qdoc_bin}
        ${prepare_qdoc_args}
    )

    add_dependencies(prepare_docs_${target} qattributionsscanner_${target})

    # generate docs target
    set(generate_qdocs_args
        -outputdir "${qdoc_output_dir}"
        -installdir "${INSTALL_DOCDIR}"
        "${target_source_dir}/${doc_project}"
        -generate
        -indexdir "${index_dir}"
        "${include_path_args}"
    )

    add_custom_target(generate_docs_${target}
        DEPENDS ${qdoc_bin}
        COMMAND ${CMAKE_COMMAND} -E env ${qdoc_env_args}
        ${qdoc_bin}
        ${generate_qdocs_args}
    )

    add_dependencies(generate_docs_${target} prepare_docs_${target})

    # generate html
    set(html_qdocs_args
        -outputdir "${qdoc_output_dir}"
        -installdir "${INSTALL_DOCDIR}"
        "${target_source_dir}/${doc_project}"
        -indexdir "${index_dir}"
        "${include_path_args}"
    )

    add_custom_target(html_docs_${target}
        DEPENDS ${qdoc_bin}
        COMMAND ${CMAKE_COMMAND} -E env ${qdoc_env_args}
        ${qdoc_bin}
        ${html_qdocs_args}
    )

    add_dependencies(html_docs_${target} generate_docs_${target})

    # generate .qch
    set(qch_file_name ${doc_target}.qch)
    set(qch_file_path ${qdoc_output_dir}/${qch_file_name})

    add_custom_target(qch_docs_${target}
        DEPENDS ${qhelpgenerator_bin}
        COMMAND ${qhelpgenerator_bin}
           "${qdoc_output_dir}/${doc_target}.qhp"
           -o "${qch_file_path}"
    )
    add_dependencies(qch_docs_${target} generate_docs_${target})

    if (QT_WILL_INSTALL)
        add_custom_target(install_html_docs_${target}
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${qdoc_output_dir}"
                "${CMAKE_INSTALL_PREFIX}/${INSTALL_DOCDIR}/${doc_target}"
            COMMENT "Installing html docs for target ${target}"
        )

        add_custom_target(install_qch_docs_${target}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${qch_file_path}"
                "${CMAKE_INSTALL_PREFIX}/${INSTALL_DOCDIR}/${qch_file_name}"
            COMMENT "Installing qch docs for target ${target}"
       )
    else()
        # Don't need to do anything when not installing
        add_custom_target(install_html_docs_${target})
        add_custom_target(install_qch_docs_${target})
    endif()

    add_dependencies(install_html_docs_${target} html_docs_${target})
    add_dependencies(install_qch_docs_${target} qch_docs_${target})

    add_custom_target(install_docs_${target})
    add_dependencies(install_docs_${target} install_html_docs_${target} install_qch_docs_${target})

    add_custom_target(docs_${target})
    add_dependencies(docs_${target} html_docs_${target})
    add_dependencies(docs_${target} qch_docs_${target})

    add_dependencies(${qt_docs_prepare_target_name} prepare_docs_${target})
    add_dependencies(${qt_docs_generate_target_name} generate_docs_${target})
    add_dependencies(${qt_docs_html_target_name} html_docs_${target})
    add_dependencies(${qt_docs_qch_target_name} qch_docs_${target})
    add_dependencies(${qt_docs_target_name} docs_${target})
    add_dependencies(${qt_docs_install_html_target_name} install_html_docs_${target})
    add_dependencies(${qt_docs_install_qch_target_name} install_qch_docs_${target})
    add_dependencies(${qt_docs_install_target_name} install_docs_${target})
endfunction()
