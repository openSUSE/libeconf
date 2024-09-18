# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../html DESTINATION share/doc)
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/libeconf.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_comment_tag.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_delimiter_tag.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_errLocation.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_errString.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_freeArray.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_freeExtValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_freeFile.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_getBoolValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_getExtValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_getFloatValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_getGroups.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_getIntValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_getKeys.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_getPath.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_getStringValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_mergeFiles.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_newIniFile.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_newKeyFile.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_readConfig.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_readConfigWithCallback.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_readFile.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_readFileWithCallback.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_setBoolValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_set_comment_tag.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_set_conf_dirs.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_set_delimiter_tag.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_setFloatValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_setIntValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_setStringValue.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econf_writeFile.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/econftool.8 DESTINATION ${CMAKE_INSTALL_MANDIR}/man8)
