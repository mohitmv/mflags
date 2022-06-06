// TODOs:
// 1. Test out positional params. (var-number of args, fixed args, tuple), and
//    handle help text for it.
// 2. Add support for std::tuple and test it out.
// 3. Add support for required params and test them out.
// 4. Test out issues with ArgsDescriptor example - field declared twice.

#include "mflags.h"

#include <cassert>
#include <iostream>
#include <set>

void BasicTest() {
  mflags::ArgsDescriptor args_desc{};
  int flag1 = 0;
  std::string flag2;
  args_desc.AddArg({.names={"--flag1", "-f"}, .help_text="help f1"}, &flag1);
  args_desc.AddArg({.names={"-flag2", "-g"}, .help_text="help f2"}, &flag2);
  auto status = args_desc.ParseFlagsInternal(
    {"", "--flag1", "44", "-flag2", "ABC"});
  assert(status.ok());
  assert(flag1 == 44);
  assert(flag2 == "ABC");

  std::cout << "Passed BasicTest" << std::endl;
}

void InvalidInputTest_Basic() {
  int flag1 = 0;
  mflags::ArgsDescriptor args_desc{};
  args_desc.AddArg({.names={"--flag1"}}, &flag1);
  auto status = args_desc.ParseFlagsInternal(
      {"", "--flag1", "44", "-flag2", "ABC"});
  assert(!status.ok());
  assert(status.str() == "Unrecognized param: -flag2 ABC");

  std::cout << "Passed InvalidInputTest_Basic" << std::endl;
}

void Basic_TypedParsing() {
  int flag1 = 0;
  bool flag2 = false;
  char flag3 = 0;
  double flag4 = 0.0;
  std::string flag5;
  const char* flag6 = nullptr;
  mflags::ArgsDescriptor args_desc{};
  args_desc.AddArg({.names={"--flag1"}}, &flag1);
  args_desc.AddArg({.names={"--flag2"}}, &flag2);
  args_desc.AddArg({.names={"--flag3"}}, &flag3);
  args_desc.AddArg({.names={"--flag4"}}, &flag4);
  args_desc.AddArg({.names={"--flag5"}}, &flag5);
  args_desc.AddArg({.names={"--flag6"}}, &flag6);

  assert(args_desc.ParseFlagsInternal({"", "--flag1", "-22"}).ok());
  assert(flag1 == -22);
  assert(flag4 == 0.0);
  assert(flag6 == nullptr);

  assert(args_desc.ParseFlagsInternal({"", "--flag1", "+22"}).ok());
  assert(flag1 == 22);

  assert(args_desc.ParseFlagsInternal({"", "--flag2"}).ok());
  assert(flag2 == true);

  assert(args_desc.ParseFlagsInternal({"", "--flag3", "+"}).ok());
  assert(flag3 == '+');

  assert(args_desc.ParseFlagsInternal({"", "--flag4", "+2.3"}).ok());
  assert(flag4 == 2.3);

  assert(args_desc.ParseFlagsInternal({"", "--flag5", "+2.3"}).ok());
  assert(flag5 == "+2.3");

  assert(args_desc.ParseFlagsInternal({"", "--flag6", "+2.3"}).ok());
  assert(flag6 == std::string_view("+2.3"));

  std::cout << "Passed Basic_TypedParsing" << std::endl;
}

void InvalidInputTest_TypedParsing() {
  int flag1 = 0;
  bool flag2 = false;
  char flag3 = 0;
  double flag4 = 0.0;
  mflags::ArgsDescriptor args_desc{};
  args_desc.AddArg({.names={"--flag1"}}, &flag1);
  args_desc.AddArg({.names={"--flag2"}}, &flag2);
  args_desc.AddArg({.names={"--flag3"}}, &flag3);
  args_desc.AddArg({.names={"--flag4"}}, &flag4);

  auto status = args_desc.ParseFlagsInternal({"", "--flag1", "4.4"});
  assert(status.str() == "Failed to parse `4.4` as type int for field --flag1");

  status = args_desc.ParseFlagsInternal({"", "--flag1", "ABC"});
  assert(status.str() == "Failed to parse `ABC` as type int for field --flag1");

  status = args_desc.ParseFlagsInternal({"", "--flag3", "AB"});
  assert(status.str() == "Failed to parse `AB` as type char for field --flag3");

  assert(args_desc.ParseFlagsInternal({"", "--flag2", "--flag3", "A"}).ok());
  assert(flag2 == true);
  assert(flag3 == 'A');

  status = args_desc.ParseFlagsInternal({"", "--flag4", "2.3.5"});
  assert(status.str() == "Failed to parse `2.3.5` as type double for field --flag4");

  std::cout << "Passed InvalidInputTest_TypedParsing" << std::endl;
}


void InvalidInputTest_NumArgs() {
  int flag1 = 0;
  bool flag2 = false;
  char flag3 = 0;
  double flag4 = 0.0;
  mflags::ArgsDescriptor args_desc{};
  args_desc.AddArg({.names={"--flag1"}}, &flag1);
  args_desc.AddArg({.names={"--flag2"}}, &flag2);
  args_desc.AddArg({.names={"--flag3"}}, &flag3);
  args_desc.AddArg({.names={"--flag4"}}, &flag4);

  auto status = args_desc.ParseFlagsInternal({"", "--flag2", "--flag1", "--flag3=A"});
  assert(status.str() == "Invalid number of args for `--flag1`. "
                         "Expected 1 found 0. Should be of type int");
  std::cout << "Passed InvalidInputTest_NumArgs" << std::endl;
}

// Test the case when command line args are passed using `=`
// Example: See flag1 in `--flag1=44 -flag2 ABC`
void TestEqualsToAfterName() {
  mflags::ArgsDescriptor args_desc{};
  int flag1 = 0;
  std::string flag2;
  const char* flag3 = nullptr;
  char flag4 = 0;
  args_desc.AddArg({.names={"--flag1", "-f"}}, &flag1);
  args_desc.AddArg({.names={"-flag2", "-g"}}, &flag2);
  args_desc.AddArg({.names={"--flag3"}}, &flag3);
  args_desc.AddArg({.names={"--flag4"}}, &flag4);
  auto status = args_desc.ParseFlagsInternal(
    {"", "--flag1=44", "-flag2", "ABC", "--flag3=XY", "--flag4=A"});
  assert(status.ok());
  assert(flag1 == 44);
  assert(flag2 == "ABC");
  assert(flag3 == std::string_view("XY"));
  assert(flag4 == 'A');

  std::cout << "Passed TestEqualsToAfterName" << std::endl;
}

void TestTupleParams() {
  mflags::ArgsDescriptor args_desc{};
  std::pair<int, std::string> field1;
  std::pair<double, bool> field2;
  std::pair<int, const char*> field3;
  args_desc.AddArg({.names={"-f1"}}, &field1);
  args_desc.AddArg({.names={"-f2"}}, &field2);
  args_desc.AddArg({.names={"-f3"}}, &field3);
  auto status = args_desc.ParseFlagsInternal(
      {"", "-f1", "22", "ABC", "-f2", "4.5", "true", "-f3", "45", "XYZ"});
  assert(status.ok());
  assert(field1.first == 22);
  assert(field1.second == "ABC");
  assert(field2.first == 4.5);
  assert(field2.second == true);
  assert(field3.first == 45);
  assert(std::string_view(field3.second) == "XYZ");

  status = args_desc.ParseFlagsInternal({"", "-f1", "2.2", "ABC", "-f2"});
  assert(status.str() == "Failed to parse `2.2` as type int for field -f1. "
        "Expected args of -f1 to be parsable for pair<int, string>");

  status = args_desc.ParseFlagsInternal({"", "-f1", "2.2", "-f2"});
  assert(status.str() == "Invalid number of args for `-f1`. Expected 2 found 1."
                         " Should be parsable for pair<int, string>");

  std::cout << "Passed TestTupleParams" << std::endl;
}

void TestVectorOfCoreTypes() {
  mflags::ArgsDescriptor args_desc{};
  std::vector<int> f1;
  std::vector<std::string> field2;
  std::vector<const char*> field3;
  std::vector<bool> field4;
  std::vector<double> field5;
  int field6 = 0;
  args_desc.AddArg({.names={"-f1"}}, &f1);
  args_desc.AddArg({.names={"-f2"}}, &field2);
  args_desc.AddArg({.names={"-f3"}}, &field3);
  args_desc.AddArg({.names={"-f4"}}, &field4);
  args_desc.AddArg({.names={"-f5"}}, &field5);
  args_desc.AddArg({.names={"-f6"}}, &field6);

  auto status = args_desc.ParseFlagsInternal(
      {"", "-f1", "22", "23", "-f2", "4.5", "ABC", "true", "-f1", "4", "2", "7",
       "-f6", "45", "-f4", "true", "false"});
  assert(status.ok());
  assert(f1.size() == 5 && f1[0] == 22 && f1[1] == 23 && f1[2] == 4);
  assert(f1[3] == 2 && f1[4] == 7);
  assert(field2.size() == 3 && field2[0] == "4.5" && field2[1] == "ABC");
  assert(field2[2] == "true");
  assert(field4.size() == 2 && field4[0] == true && field4[1] == false);
  assert(field6 == 45);

  status = args_desc.ParseFlagsInternal(
      {"", "-f3", "4.5", "true", "-f5", "4.5"});
  assert(status.ok());
  assert(field3.size() == 2);
  assert(field3[0] == std::string_view("4.5"));
  assert(field3[1] == std::string_view("true"));
  assert(field5.size() == 1 && field5[0] == 4.5);

  status = args_desc.ParseFlagsInternal({"", "-f1", "-954545", "4.5", "true"});
  assert(status.str() == "Failed to parse `4.5` as type int for field -f1");

  std::cout << "Passed TestVectorOfCoreTypes" << std::endl;
}

void TestVectorOfTupleOfCoreTypes() {
  mflags::ArgsDescriptor args_desc{};
  std::vector<std::pair<int, std::string>> f1;
  std::pair<int, int> field2 {};
  int field3 = 0;
  std::vector<double> field4;
  args_desc.AddArg({.names={"-f1"}}, &f1);
  args_desc.AddArg({.names={"-f2"}}, &field2);
  args_desc.AddArg({.names={"-f3"}}, &field3);
  args_desc.AddArg({.names={"-f4"}}, &field4);
  auto status = args_desc.ParseFlagsInternal(
      {"", "-f1", "22", "ABC", "-f3", "4", "-f4", "5.6", "-f2", "4", "7",
       "-f1", "33", "45"});
  assert(status.ok());
  assert(f1.size() == 2);
  assert(f1[0].first == 22 && f1[0].second == "ABC");
  assert(f1[1].first == 33 && f1[1].second == "45");
  assert(field3 == 4);
  assert(field2.first == 4 && field2.second == 7);
  assert(field4.size() == 1);
  assert(field4[0] == 5.6);

  status = args_desc.ParseFlagsInternal({"", "-f1", "2.2", "ABC", "-f2"});
  assert(status.str() == "Failed to parse `2.2` as type int for field -f1. "
        "Expected args of -f1 to be parsable for pair<int, string>");

  std::cout << "Passed TestVectorOfTupleOfCoreTypes" << std::endl;
}

void TestOverwriteValue() {
  mflags::ArgsDescriptor args_desc{};
  std::pair<int, int> f1 {};
  int f2 = 0;
  const char* f3 = nullptr;
  args_desc.AddArg({.names={"-f1"}}, &f1);
  args_desc.AddArg({.names={"-f2"}}, &f2);
  args_desc.AddArg({.names={"-f3"}}, &f3);
  assert(args_desc.ParseFlagsInternal(
      {"", "-f1", "22", "8", "-f2=4", "-f1", "11", "62", "-f2", "45",
       "-f3=YYY", "-f2", "89", "-f3=AX"}).ok());
  assert(f1.first == 11 && f1.second == 62);
  assert(f2 == 89);
  assert(f3 == std::string_view("AX"));

  std::cout << "Passed TestOverwriteValue" << std::endl;
}

void TestPositionalArgs() {
  mflags::ArgsDescriptor args_desc{};
  std::pair<int, int> f1 {};
  int positional = 0;
  const char* f2 = nullptr;
  args_desc.AddArg({.names={"-f1"}}, &f1);
  args_desc.AddArg({.positional=true}, &positional);
  args_desc.AddArg({.names={"-f2"}}, &f2);
  assert(args_desc.ParseFlagsInternal(
      {"", "-f1", "22", "8", "111000", "-f2=AX"}).ok());
  assert(f1.first == 22 && f1.second == 8);
  assert(positional == 111000);
  assert(f2 == std::string_view("AX"));

  std::cout << "Passed TestPositionalArgs" << std::endl;
}

void TestPositionalArgs2() {
  mflags::ArgsDescriptor args_desc{};
  std::pair<int, int> f1 {};
  int positional = 0;
  std::vector<const char*> positionals;
  const char* f2 = nullptr;
  args_desc.AddArg({.names={"-f1"}}, &f1);
  args_desc.AddArg({.names={"fp"}, .positional=true, .required=true}, &positional);
  args_desc.AddArg({.positional=true}, &positionals);
  args_desc.AddArg({.names={"-f2"}}, &f2);

  auto status = args_desc.ParseFlagsInternal(
      {"", "-f1", "22", "8", "111000", "pp1", "-f2=AX", "qq1"});
  assert(status.ok());
  assert(f1.first == 22 && f1.second == 8);
  assert(positional == 111000);
  assert(f2 == std::string_view("AX"));
  assert(positionals.size() == 2);
  assert(positionals[0] == std::string_view("pp1"));
  assert(positionals[1] == std::string_view("qq1"));
  positionals.clear();

  status = args_desc.ParseFlagsInternal(
      {"", "-f1", "22", "8", "-f2=AX"});
  assert(!status.ok());
  assert(status.str() == "Required positional arg fp not found.");

  status = args_desc.ParseFlagsInternal(
      {"", "-f1", "22", "8", "111000", "-f2=AX"});
  assert(status.ok());
  assert(positionals.size() == 0);

  std::cout << "Passed TestPositionalArgs2" << std::endl;
}

std::string g_expected_help_text = R"(
This is an example program

Positional Arguments:

  p1                      For P1. Type: int ; default: 0
  p2                      For P2. Type: int ; default: 0

Optional Arguments:

  -h, --help              Show this help message and exit. 
  -f1 VALUE1 VALUE2       For F1. Type: pair<int, int> ; default: (0, 0)
  -f2, --field2=VALUE     For F2. Type: int ; default: 0
  ( -f4, --field4 VALUE1 VALUE2 )*
                          For F4. Type: vector<pair<int, int>>
  -f3=VALUE               For F3. Type: const char* ; default: nullptr
  -f5 VALUES...           For F5. Type: vector<int>
)";

void TestHelpText() {
  mflags::ArgsDescriptor args_desc{"This is an example program"};
  int p1 = 0, p2 = 0;
  std::pair<int, int> f1 {};
  int f2 = 0;
  const char* f3 = nullptr;
  std::vector<std::pair<int, int>> f4;
  std::vector<int> f5;
  args_desc.AddArg({.names={"p1"}, .positional=true, .help_text="For P1"}, &p1);
  args_desc.AddArg({.names={"p2"}, .positional=true, .help_text="For P2"}, &p2);
  args_desc.AddArg({.names={"-f1"}, .help_text="For F1"}, &f1);
  args_desc.AddArg({.names={"-f2", "--field2"}, .help_text="For F2"}, &f2);
  args_desc.AddArg({.names={"-f4", "--field4"}, .help_text="For F4"}, &f4);
  args_desc.AddArg({.names={"-f3"}, .help_text="For F3"}, &f3);
  args_desc.AddArg({.names={"-f5"}, .help_text="For F5"}, &f5);
  auto help_text = args_desc.FullHelpText();
  assert(g_expected_help_text.size() == help_text.size());
  assert(g_expected_help_text == help_text);
  std::cout << "Passed TestHelpText" << std::endl;
}

void TestMisc() {
  mflags::ArgsDescriptor args_desc{};
  std::set<int> f1;
  std::cout << "Passed TestMisc" << std::endl;
}

int main() {
  BasicTest();
  InvalidInputTest_Basic();
  Basic_TypedParsing();
  InvalidInputTest_TypedParsing();
  InvalidInputTest_NumArgs();
  TestEqualsToAfterName();
  TestTupleParams();
  TestVectorOfCoreTypes();
  TestVectorOfTupleOfCoreTypes();
  TestOverwriteValue();
  TestPositionalArgs();
  TestPositionalArgs2();
  TestHelpText();
}
