#include "mflags.h"

#include <cassert>

DEFINE_MFLAG(int, x, 0, "Input value x for blah");

DEFINE_MFLAG(double, y, 0.0, "Input value x for blah");

DEFINE_MFLAG(const char*, z, "default_z", "Input value z for blah");

DECLARE_MFLAG(int, p);
DEFINE_MFLAG(int, p, 0, "Input value p for blah");

DECLARE_MFLAG(bool, r);
DEFINE_MFLAG(bool, r, false, "Input value r for blah");

// Mimic Dynamic Loaded Lib
// Note: Flags declared in dynamic libs will be resolved at static-init step
// at their load time. Hence they will behave like usual flags declared.
ALLOW_DYNAMIC_FLAG(q);
int MFLAGS_q = 0;
void LoadedDynamicLib() {
  ::mflags::AutoAssign<int> AutoAssignVar_q {"q", __FILE__, &MFLAGS_q, "", ""};
}

MFLAG_ODR_INIT();

int main() {
  {
    std::cout << "===== Test1 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x", "44"};
    mflags::ParseFlags(3, argv);
    assert(MFLAGS_x == 44);
    assert(MFLAGS_z == std::string("default_z"));
    std::cout << "===== All Good ===== " << std::endl;
  }
  {
    std::cout << "===== Test2 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x", "44", "--z", "value_of_z", "--p", "-39", "--x", "986"};
    mflags::ParseFlags(9, argv);
    assert(MFLAGS_x == 986);
    assert(MFLAGS_z == std::string("value_of_z"));
    assert(MFLAGS_p == -39);
    std::cout << "===== All Good ===== " << std::endl;
  }
  {
    std::cout << "===== Test3 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x", "4", "--y", "-45.68", "--r", "true"};
    mflags::ParseFlags(7, argv);
    assert(MFLAGS_x == 4);
    assert(MFLAGS_y == -45.68);
    assert(MFLAGS_r);
    std::cout << "===== All Good ===== " << std::endl;
  }
  {
    std::cout << "===== Test4 ===== " << std::endl;
    const char* argv[] = {"./a.out", "--x", "4", "--q", "88"};
    mflags::ParseFlags(5, argv);
    assert(MFLAGS_x == 4);
    LoadedDynamicLib();
    assert(MFLAGS_q == 88);
    std::cout << "===== All Good ===== " << std::endl;
  }
  if (false) {
    std::cout << "===== Manual Test ===== " << std::endl;
    const char* argv[] = {"./a.out", "--xyz", "4", "--help"};
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
    // Expected Crash : ERR: Command line option 'x' should start with `--`.
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
