if(EXISTS "/mnt/c/Users/Usuario/CLionProjects/ac_p1/utest/cmake-build-debug/progargs_test[1]_tests.cmake")
  include("/mnt/c/Users/Usuario/CLionProjects/ac_p1/utest/cmake-build-debug/progargs_test[1]_tests.cmake")
else()
  add_test(progargs_test_NOT_BUILT progargs_test_NOT_BUILT)
endif()
