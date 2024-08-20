# Set make check target
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

# Set up macro for building and adding tests
macro(BuildAndAddTest TESTNAME)
  if (${TESTNAME} STREQUAL "tst-getconfdirs2")
    add_executable(${TESTNAME} EXCLUDE_FROM_ALL tst-getconfdirs1.c)
  else()
    add_executable(${TESTNAME} EXCLUDE_FROM_ALL ${TESTNAME}.c)
  endif()
  target_link_libraries(${TESTNAME} PRIVATE econf)
  if (${TESTNAME} STREQUAL "tst-getconfdirs1")
    target_compile_options(${TESTNAME} PRIVATE -DTESTSDIR=\"${PROJECT_SOURCE_DIR}/tests/\" -DSUFFIX=\".conf\")
  elseif (${TESTNAME} STREQUAL "tst-getconfdirs2")
    target_compile_options(${TESTNAME} PRIVATE -DTESTSDIR=\"${PROJECT_SOURCE_DIR}/tests/\" -DSUFFIX=\"conf\")
  elseif (${TESTNAME} STREQUAL "tst-getconfdirs8")
    target_compile_options(${TESTNAME} PRIVATE -DTESTSDIR=\"${PROJECT_SOURCE_DIR}/tests/\" -DSUFFIX=\"conf\")
  else()
    target_compile_options(${TESTNAME} PRIVATE -DTESTSDIR=\"${PROJECT_SOURCE_DIR}/tests/\")
  endif()
  add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
  add_dependencies(check ${TESTNAME})
endmacro()

set(TESTS tst-filedoesnotexit1
          tst-merge1
          tst-merge2
          tst-merge3
          tst-merge4
          tst-merge5
          tst-logindefs1
          tst-logindefs2
          tst-arguments1
          tst-arguments2
          tst-arguments3
          tst-arguments4
          tst-arguments5
	  tst-string
	  tst-string-append
	  tst-long-name
	  tst-security
	  tst-checkfiles
	  tst-delimiter-comment
	  tst-extvalue
	  tst-comments
	  tst-python
	  tst-options
	  tst-man-dir
          tst-getconfdirs1
          tst-getconfdirs2
          tst-getconfdirs3
          tst-getconfdirs4
          tst-getconfdirs5
          tst-getconfdirs6
          tst-getconfdirs7
          tst-getconfdirs8
	  tst-without-suffix
          tst-econf_errstring1
          tst-setgetvalues1
          tst-groups1
          tst-groups2
          tst-groups3
          tst-groups4
          tst-parseconfig1
          tst-quote1
	  tst-parse-error
	  tst-getpath
	  tst-write-string-data
	  tst-write-comments )

foreach (TESTCASE ${TESTS})
  BuildAndAddTest(${TESTCASE})
endforeach()

find_program (BASH_PROGRAM bash)

if (BASH_PROGRAM)
    add_test (econftool1 ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tst-econftool1.sh)
    add_test (econftool_show1 ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tst-econftool_show1.sh)
    add_test (econftool_cat ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tst-econftool_cat.sh)
    add_dependencies(check econftool)
endif (BASH_PROGRAM)
