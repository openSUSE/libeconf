project(
  'libeconf',
  'c',
  meson_version : '>= 0.49.0',
  default_options : ['buildtype=debugoptimized',
  		  'default_library=shared',
		  'b_pie=true',
		  'warning_level=3',],
  license : 'MIT',
  version : '0.8.1',
)

cc = meson.get_compiler('c')
pkg = import('pkgconfig')
inc = include_directories('include')

add_project_arguments(['-D_GNU_SOURCE=1',
		       '-DXTSTRINGDEFINES',
		       '-D_FORTIFY_SOURCE=2'], language : 'c')

possible_cc_flags = [
		  '-Dwerror=true',
                  '-flto=auto',
                  '-ffat-lto-objects',
		  '-fstack-protector-strong',
		  '-funwind-tables',
		  '-fasynchronous-unwind-tables',
		  '-fstack-clash-protection',
		  '-Werror=return-type',
		  '-Wbad-function-cast',
		  '-Wcast-align',
		  '-Wcast-qual',
		  '-Winline',
		  '-Wmissing-declarations',
		  '-Wmissing-prototypes',
		  '-Wnested-externs',
		  '-Wshadow',
		  '-Wstrict-prototypes',
		  '-Wundef',
		  ]
add_project_arguments(cc.get_supported_arguments(possible_cc_flags), language : 'c')

libeconf_src = files(
  'lib/econf_error.c',
  'lib/get_value_def.c',
  'lib/getfilecontents.c',
  'lib/helpers.c',
  'lib/keyfile.c',
  'lib/libeconf.c',
  'lib/libeconf_ext.c',
  'lib/readconfig.c',
  'lib/mergefiles.c',
)
example_src = ['example/example.c']
econftool_src = ['util/econftool.c']

possible_link_args = [
      '-Wl,--no-undefined',
      '-Wl,--no-undefined-version',
      ]

mapfile = 'lib/libeconf.map'
version_flag = ['-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), mapfile)]

lib = library(
  'econf',
  libeconf_src,
  include_directories : inc,
  install : true,
  link_args : cc.get_supported_link_arguments(possible_link_args) + version_flag,
  link_depends : mapfile,
  version : meson.project_version(),
  soversion : '0',
)

install_headers('include/libeconf.h', 'include/libeconf_ext.h')

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

# Example
exampledir = join_paths(meson.source_root(), 'example/')
example_args = ['-DEXAMPLEDIR="' + exampledir + '"']
executable('example', example_src, c_args: example_args, dependencies : libeconf_dep)

# Commandline interface
executable('econftool', econftool_src, dependencies : libeconf_dep, install : true, )

# Unit tests
subdir('tests')

# documentation
subdir('doc')