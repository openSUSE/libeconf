# This file builds and runs the unit tests

testdir = join_paths(meson.source_root(), 'tests/')
test_args = ['-DTESTSDIR="' + testdir + '"']

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


tst_econf_errstring1_exe = executable('tst-econf_errstring1', 'tst-econf_errstring1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-econf_errstring1', tst_econf_errstring1_exe)


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


tst_parseconfig1_exe = executable('tst-parseconfig1', 'tst-parseconfig1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-parseconfig1', tst_parseconfig1_exe)


tst_quote1_exe = executable('tst-quote1', 'tst-quote1.c', c_args: test_args, dependencies : libeconf_dep)
test('tst-quote1', tst_quote1_exe)

test('tst_econftool1', find_program('tst-econftool1.sh'))
test('tst_econftool_show1', find_program('tst-econftool_show1.sh'))
