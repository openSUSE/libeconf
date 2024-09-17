# This file builds and runs the unit tests

testdir = join_paths(meson.source_root(), 'tests/')
test_args = ['-DTESTSDIR="' + testdir + '"']
inc = include_directories('../include')

staticlibeconf_dep = declare_dependency(
  include_directories : inc,
)

tst_filedoesnotexit1_exe = executable('tst-filedoesnotexit1', 'tst-filedoesnotexit1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-filedoesnotexit1', tst_filedoesnotexit1_exe)


tst_merge1_exe = executable('tst-merge1', 'tst-merge1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-merge1', tst_merge1_exe)
tst_merge2_exe = executable('tst-merge2', 'tst-merge2.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-merge2', tst_merge2_exe)
tst_merge3_exe = executable('tst-merge3', 'tst-merge3.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-merge3', tst_merge3_exe)
tst_merge4_exe = executable('tst-merge4', 'tst-merge4.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-merge4', tst_merge4_exe)
tst_merge5_exe = executable('tst-merge5', 'tst-merge5.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-merge5', tst_merge5_exe)

tst_write_string_data_exe = executable('tst-write-string-data', 'tst-write-string-data.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-write-string-data', tst_write_string_data_exe)
tst_write_comments_exe = executable('tst-write-comments', 'tst-write-comments.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-write-comments', tst_write_comments_exe)

tst_logindefs1_exe = executable('tst-logindefs1', 'tst-logindefs1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-logindefs1', tst_logindefs1_exe)
tst_logindefs2_exe = executable('tst-logindefs2', 'tst-logindefs2.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-logindefs2', tst_logindefs2_exe)

tst_arguments1_exe = executable('tst-arguments1', 'tst-arguments1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-arguments1', tst_arguments1_exe)
tst_arguments2_exe = executable('tst-arguments2', 'tst-arguments2.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-arguments2', tst_arguments2_exe)
tst_arguments3_exe = executable('tst-arguments3', 'tst-arguments3.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-arguments3', tst_arguments3_exe)
tst_arguments4_exe = executable('tst-arguments4', 'tst-arguments4.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-arguments4', tst_arguments4_exe)
tst_arguments5_exe = executable('tst-arguments5', 'tst-arguments5.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-arguments5', tst_arguments5_exe)

tst_string_exe = executable('tst-string', 'tst-string.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-string', tst_string_exe)
tst_string_append_exe = executable('tst-string-append', 'tst-string-append.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-string-append', tst_string_append_exe)
tst_long_name_exe = executable('tst-long-name', 'tst-long-name.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-long-name', tst_long_name_exe)
tst_security_exe = executable('tst-security', 'tst-security.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-security', tst_security_exe)
tst_checkfiles_exe = executable('tst-checkfiles', 'tst-checkfiles.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-checkfiles', tst_checkfiles_exe)
tst_delimiter_comment_exe = executable('tst-delimiter-comment', 'tst-delimiter-comment.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-delimiter-comment', tst_delimiter_comment_exe)
tst_extvalue_exe = executable('tst-extvalue', 'tst-extvalue.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-extvalue', tst_extvalue_exe)
tst_comments_exe = executable('tst-comments', 'tst-comments.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-comments', tst_comments_exe)
tst_comments2_exe = executable('tst-comments2',
  sources: ['tst-comments2.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-comments2-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-comments2', tst_comments2_exe)
tst_python_exe = executable('tst-python', 'tst-python.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-python', tst_python_exe)

tst_getconfdirs1_exe = executable('tst-getconfdirs1', 'tst-getconfdirs1.c', c_args: ['-DSUFFIX=".conf"', test_args], dependencies : libeconf_dep)
test('tst-getconfdirs1', tst_getconfdirs1_exe)
tst_getconfdirs2_exe = executable('tst-getconfdirs2', 'tst-getconfdirs1.c', c_args: ['-DSUFFIX="conf"', test_args], dependencies : libeconf_dep)
test('tst-getconfdirs2', tst_getconfdirs2_exe)
tst_getconfdirs3_exe = executable('tst-getconfdirs3', 'tst-getconfdirs3.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-getconfdirs3', tst_getconfdirs3_exe)
tst_getconfdirs4_exe = executable('tst-getconfdirs4', 'tst-getconfdirs4.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-getconfdirs4', tst_getconfdirs4_exe)
tst_getconfdirs5_exe = executable('tst-getconfdirs5', 'tst-getconfdirs5.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-getconfdirs5', tst_getconfdirs5_exe)
tst_getconfdirs6_exe = executable('tst-getconfdirs6', 'tst-getconfdirs6.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-getconfdirs6', tst_getconfdirs6_exe)
tst_getconfdirs7_exe = executable('tst-getconfdirs7', 'tst-getconfdirs7.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-getconfdirs7', tst_getconfdirs7_exe)
tst_getconfdirs8_exe = executable('tst-getconfdirs8', 'tst-getconfdirs8.c', c_args: ['-DSUFFIX="conf"', test_args], dependencies : libeconf_dep)
test('tst-getconfdirs8', tst_getconfdirs8_exe)
tst_getconfdirs9_exe = executable('tst-getconfdirs9', 'tst-getconfdirs9.c', c_args: ['-DSUFFIX="conf"', test_args], dependencies : libeconf_dep)
test('tst-getconfdirs9', tst_getconfdirs9_exe)


tst_man_dir_exe = executable('tst-man-dir', 'tst-man-dir.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-man-dir', tst_man_dir_exe)

tst_options_exe = executable('tst-options', 'tst-options.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-options', tst_options_exe)

tst_uapi_1_exe = executable('tst-uapi-1',
  sources: ['tst-uapi-1.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-1-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-1', tst_uapi_1_exe)

tst_uapi_2_exe = executable('tst-uapi-2',
  sources: ['tst-uapi-2.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-2-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-2', tst_uapi_2_exe)

tst_uapi_3_exe = executable('tst-uapi-3',
  sources: ['tst-uapi-3.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-3-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-3', tst_uapi_3_exe)

tst_uapi_4_exe = executable('tst-uapi-4',
  sources: ['tst-uapi-4.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-4-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-4', tst_uapi_4_exe)

tst_uapi_5_exe = executable('tst-uapi-5',
  sources: ['tst-uapi-5.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-5-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-5', tst_uapi_5_exe)

tst_uapi_6_exe = executable('tst-uapi-6',
  sources: ['tst-uapi-6.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-6-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-6', tst_uapi_6_exe)

tst_uapi_7_exe = executable('tst-uapi-7',
  sources: ['tst-uapi-7.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-7-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-7', tst_uapi_7_exe)

tst_uapi_8_exe = executable('tst-uapi-8',
  sources: ['tst-uapi-8.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-8-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-8', tst_uapi_8_exe)

tst_uapi_9_exe = executable('tst-uapi-9',
  sources: ['tst-uapi-9.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-9-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-9', tst_uapi_9_exe)

tst_uapi_10_exe = executable('tst-uapi-10',
  sources: ['tst-uapi-10.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-10-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-10', tst_uapi_10_exe)

tst_uapi_11_exe = executable('tst-uapi-11',
  sources: ['tst-uapi-11.c', '../lib/helpers.c', '../lib/readconfig.c', '../lib/libeconf.c', '../lib/getfilecontents.c',
            '../lib/mergefiles.c', '../lib/econf_error.c', '../lib/keyfile.c'],
  c_args: ['-DTESTSDIR="' + testdir + 'tst-uapi-11-data' + '"'], dependencies : staticlibeconf_dep)
test('tst-uapi-11', tst_uapi_11_exe)

tst_parse_error_exe = executable('tst-parse-error', 'tst-parse-error.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-parse-error', tst_parse_error_exe)

tst_econf_errstring1_exe = executable('tst-econf_errstring1', 'tst-econf_errstring1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-econf_errstring1', tst_econf_errstring1_exe)

tst_getpath_exe = executable('tst-getpath', 'tst-getpath.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-getpath', tst_getpath_exe)

tst_without_suffix_exe = executable('tst-without-suffix', 'tst-without-suffix.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-without-suffix', tst_without_suffix_exe)

tst_setgetvalues1_exe = executable('tst-setgetvalues1', 'tst-setgetvalues1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-setgetvalues1', tst_setgetvalues1_exe)


tst_groups1_exe = executable('tst-groups1', 'tst-groups1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-groups1', tst_groups1_exe)
tst_groups2_exe = executable('tst-groups2', 'tst-groups2.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-groups2', tst_groups2_exe)
tst_groups3_exe = executable('tst-groups3', 'tst-groups3.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-groups3', tst_groups3_exe)
tst_groups4_exe = executable('tst-groups4', 'tst-groups4.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-groups4', tst_groups4_exe)
tst_groups5_exe = executable('tst-groups5', 'tst-groups5.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-groups5', tst_groups5_exe)
tst_groups6_exe = executable('tst-groups6', 'tst-groups6.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-groups6', tst_groups6_exe)

tst_parseconfig1_exe = executable('tst-parseconfig1', 'tst-parseconfig1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-parseconfig1', tst_parseconfig1_exe)


tst_quote1_exe = executable('tst-quote1', 'tst-quote1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-quote1', tst_quote1_exe)


tst_shells1_exe = executable('tst-shells1', 'tst-shells1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-shells1', tst_shells1_exe)
tst_shells2_exe = executable('tst-shells2', 'tst-shells2.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-shells2', tst_shells2_exe)


test('tst_econftool1', find_program('tst-econftool1.sh'))
test('tst_econftool_show1', find_program('tst-econftool_show1.sh'))
test('tst_econftool_cat', find_program('tst-econftool_cat.sh'))
