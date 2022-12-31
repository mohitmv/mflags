CCFLAGS="-I../ -std=c++17"
# g++ -std=c++17 $CCFLAGS -Wl,-rpath . -rdynamic -ldl mflags_test1.cpp -o /tmp/mflags_test1 &&
g++ $CCFLAGS -c ../mflags.cpp -o /tmp/mflags &&
g++ $CCFLAGS mflags_test2.cpp /tmp/mflags -o /tmp/mflags_test2 &&
g++ $CCFLAGS example_program.cpp /tmp/mflags -o /tmp/example_program &&
# /tmp/mflags_test2
/tmp/example_program
