doxygen = find_program('doxygen', required: false)
if doxygen.found()
  conf_data = configuration_data()
  conf_data.set('PROJECT_SOURCE_DIR', meson.source_root())
  conf_data.set('CMAKE_PROJECT_NAME', meson.project_name())
  conf_data.set('PROJECT_VERSION', meson.project_version())
  conf_data.set('HAVE_DOT', 'NO')
  conf_data.set('PACKAGE', meson.project_name())
  conf_data.set('PROJECT_BINARY_DIR', meson.source_root()+'/build')
  conf_data.set('doxy_main_page', '')  

  doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: conf_data
  )

  custom_target('doc',
    build_by_default: true,
    command: [doxygen, 'doc/Doxyfile'],
    output: ['html']
  )
endif
