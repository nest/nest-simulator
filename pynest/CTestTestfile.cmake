# CMake generated Testfile for 
# Source directory: /home/yansong/programs/justforgit/nest-simulator/pynest
# Build directory: /home/yansong/programs/justforgit/nest-simulator/pynest
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(PyNEST "/usr/local/bin/nosetests" "-v" "--with-xunit" "--xunit-file=/home/yansong/programs/justforgit/nest-simulator/reports/pynest_tests.xml" "/home/yansong/programs/justforgit/nest-git/lib/python2.7/site-packages/nest/tests")
