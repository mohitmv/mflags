// Try `./mflags_example` with following command line args:
// -f1 7 -f2 AB 39 -f3 2 ABC -f3 20 DEF -f4 5 4 2 -f3 8 GHI -f4 20 25
// Also try --help

#include "mflags.h"

#include <iostream>
#include <vector>

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2> &p) {
  os << "(" << p.first << ", " << p.second << ")";
  return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> &v) {
  os << "[";
  for (auto& x : v) {
    os << x << ", ";
  }
  os << "]";
  return os;
}

int main(int argc, const char* const* argv) {
  int f1 = 0;
  std::pair<std::string, int> f2 {};
  std::vector<std::pair<int, std::string>> f3;
  std::vector<int> f4;
  mflags::ArgsDescriptor args_desc{"Help message about this program."};
  args_desc.AddArg({.names={"-f1"}, .help_text="For F1"}, &f1);
  args_desc.AddArg({.names={"-f2", "--field2"}, .help_text="For F2"}, &f2);
  args_desc.AddArg({.names={"-f3"}, .help_text="For F3"}, &f3);
  args_desc.AddArg({.names={"-f4", "--field4"}, .help_text="For F4"}, &f4);
  args_desc.ParseFlags(argc, argv);
  std::cout << "f1 = " << f1 << "\n";
  std::cout << "f2 = " << f2 << "\n";
  std::cout << "f3 = " << f3 << "\n";
  std::cout << "f4 = " << f4 << "\n";
}
