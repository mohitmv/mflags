#include "mflags.h"

#include <iostream>
#include <vector>

int main(int argc, const char* const* argv) {
  int f1 = 0;
  std::pair<std::string, int> f2 {};
  std::vector<std::pair<int, int>> f3;
  std::vector<int> f4;
  mflags::ArgsDescriptor args_desc{"help message about this program."};
  args_desc.AddArg({.names={"-f1"}, .help_text="For F1"}, &f1);
  args_desc.AddArg({.names={"-f2", "--field2"}, .help_text="For F2"}, &f2);
  args_desc.AddArg({.names={"-f3"}, .help_text="For F3"}, &f3);
  args_desc.AddArg({.names={"-f4", "--field4"}, .help_text="For F4"}, &f4);
  args_desc.ParseFlags(argc, argv);


  // mflags::ArgsDescriptor args_desc{"This is an example program"};
  // std::pair<int, int> f1 {};
  // int f2 = 0;
  // const char* f3 = nullptr;
  // std::vector<std::pair<int, int>> f4;
  // std::vector<int> f5;
  // args_desc.AddArg({.names={"-f1"}, .help_text="For F1"}, &f1);
  // args_desc.AddArg({.names={"-f2", "--field2"}, .help_text="For F2"}, &f2);
  // args_desc.AddArg({.names={"-f4", "--field4"}, .help_text="For F4"}, &f4);
  // args_desc.AddArg({.names={"-f3"}, .help_text="For F3"}, &f3);
  // args_desc.AddArg({.names={"-f5"}, .help_text="For F5"}, &f5);
  // args_desc.ParseFlags(argc, argv);
  // std::cout << "f2 = " << f2 << ", f1 = (" << f1.first << ", " << f1.second
  //           << ")" << std::endl;
}
