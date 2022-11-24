CCFLAGS="-I../"
g++ -shared -fPIC $CCFLAGS shared_lib.cpp -o shared_lib.so &&
g++ -std=c++17 $CCFLAGS -Wl,-rpath . -rdynamic -ldl mflags_test.cpp -o mflags_test &&
./mflags_test
