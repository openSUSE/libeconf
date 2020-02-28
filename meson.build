project(
  'libeconf',
  'c',
  meson_version : '>= 0.46.0',
  default_options : ['default_library=both'],
  license : 'MIT',
  version : '0.3.5',
)

pkg = import('pkgconfig')
inc = include_directories('include')
src = files(
  'lib/econf_errString.c',
  'lib/get_value_def.c',
  'lib/getfilecontents.c',
  'lib/helpers.c',
  'lib/keyfile.c',
  'lib/libeconf.c',
  'lib/mergefiles.c',
)

mapfile = 'lib/libeconf.map'
version_flag = '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), mapfile)

lib = library(
  'libeconf',
  src,
  include_directories : inc,
  install : true,
  link_args : version_flag,
  link_depends : mapfile,
  version : meson.project_version(),
  soversion : '0',
)

install_headers('include/libeconf.h')

pkg.generate(
  lib,
  name : 'libeconf',
  description : 'highly flexible library to manage key=value configuration files',
  version : meson.project_version(),
)

libeconf_dep = declare_dependency(
  link_with : lib,
  include_directories : inc,
)
