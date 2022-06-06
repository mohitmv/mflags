#include "mflags.h"

#include <iostream>
#include <cassert>

ADD_GLOBAL_MFLAG(int, g_x, 0,
    {.names={"--x"}, .help_text="Input value x for blah"});

ADD_GLOBAL_MFLAG(double, g_y, 0.0,
    {.names={"--y"}, .help_text="Input value x for blah"});

ADD_GLOBAL_MFLAG(const char*, g_z, "default_z",
    {.names={"--z"}, .help_text="Input value z for blah"});

ADD_GLOBAL_MFLAG(int, g_p, 0,
    {.names={"--p"}, .help_text="Input value p for blah"});

ADD_GLOBAL_MFLAG(bool, g_r, false,
    {.names={"--r"}, .help_text="Input value r for blah"});

int main() {
  {
    std::cout << "===== Test1 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x", "44"};
    mflags::ParseFlags(3, argv);
    assert(g_x == 44);
    assert(g_z == std::string("default_z"));
    std::cout << "===== All Good ===== " << std::endl;
  }
  {
    std::cout << "===== Test2 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x", "44", "--z", "value_of_z", "--p",
                          "-39", "--x", "986"};
    mflags::ParseFlags(9, argv);
    assert(g_x == 986);
    assert(g_z == std::string("value_of_z"));
    assert(g_p == -39);
    std::cout << "===== All Good ===== " << std::endl;
  }
  {
    std::cout << "===== Test3 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x", "4", "--y", "-45.68", "--r", "true"};
    mflags::ParseFlags(7, argv);
    assert(g_x == 4);
    assert(g_y == -45.68);
    assert(g_r);
    std::cout << "===== All Good ===== " << std::endl;
  }
  if (false) {
    std::cout << "===== Manual Test ===== " << std::endl;
    const char* argv[] = {"./a.out", "--help", "--xyz", "4"};
    // Expect: should print help text and exit(0).
    mflags::ParseFlags(4, argv);
    std::cout << "===== Program should not reach here ===== " << std::endl;
  }
  if (false) {
    std::cout << "===== Manual Failure Test 1 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x", "a4", "--y", "-45.68"};
    // Expected Crash : ERR: Failed to parse value "'a4' for the flag 'x'.
    mflags::ParseFlags(5, argv);
    std::cout << "===== Program should have crashed by now ===== " << std::endl;
  }
  if (false) {
    std::cout << "===== Manual Failure Test 2 ===== " << std::endl;
    const char* argv[] = {"./a.out", "x", "a4"};
    // Expected Crash : Unrecognized param: x a4
    mflags::ParseFlags(3, argv);
    std::cout << "===== Program should have crashed by now ===== " << std::endl;
  }
  if (false) {
    std::cout << "===== Manual Failure Test 3 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x"};
    // Expected Crash : No value for the mflag command line flag '--x'
    mflags::ParseFlags(2, argv);
    std::cout << "===== Program should have crashed by now ===== " << std::endl;
  }
  if (false) {
    std::cout << "===== Manual Failure Test 4 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x1", "33"};
    // Expected Crash : Unknown command line option '--x1'.
    mflags::ParseFlags(3, argv);
    std::cout << "===== Program should have crashed by now ===== " << std::endl;
  }
}
