doxygen = find_program('doxygen', required: false)
if doxygen.found()
  conf_data = configuration_data()
  conf_data.set('PROJECT_SOURCE_DIR', meson.source_root())
  conf_data.set('CMAKE_PROJECT_NAME', meson.project_name())
  conf_data.set('PROJECT_VERSION', meson.project_version())
  conf_data.set('HAVE_DOT', 'NO')
  conf_data.set('PACKAGE', meson.project_name())
  conf_data.set('PROJECT_BINARY_DIR', meson.current_build_dir())
  conf_data.set('doxy_main_page', '')  

  doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: conf_data
  )

  custom_target('doc',
    build_by_default: true,
    command: [doxygen, meson.current_build_dir()+'/Doxyfile'],
    output: ['html']
  )
endif

install_man('man/econftool.8',
	'man/econf_comment_tag.3',
	'man/econf_delimiter_tag.3',
	'man/econf_errLocation.3',
	'man/econf_errString.3',
	'man/econf_freeArray.3',
	'man/econf_freeExtValue.3',
	'man/econf_freeFile.3',
	'man/econf_getBoolValue.3',
	'man/econf_getExtValue.3',
	'man/econf_getFloatValue.3',
	'man/econf_getGroups.3',
	'man/econf_getIntValue.3',
	'man/econf_getKeys.3',
	'man/econf_getPath.3',
	'man/econf_getStringValue.3',
	'man/econf_mergeFiles.3',
	'man/econf_newIniFile.3',
	'man/econf_newKeyFile.3',
	'man/econf_readConfig.3',
	'man/econf_readConfigWithCallback.3',
	'man/econf_readFile.3',
	'man/econf_readFileWithCallback.3',
	'man/econf_setBoolValue.3',
	'man/econf_set_comment_tag.3',
	'man/econf_set_conf_dirs.3',
	'man/econf_set_delimiter_tag.3',
	'man/econf_setFloatValue.3',
	'man/econf_setIntValue.3',
	'man/econf_setStringValue.3',
	'man/econf_writeFile.3',
	'man/libeconf.3')