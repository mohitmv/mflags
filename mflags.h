// Author: mohitsaini1196@gmail.com
// Github: https://github.com/mohitmv/mflags
// Feel free to copy it as long as you don't remove the Author part.

// MFLAGS : A header only, super simple, command line flags library ;
// Learn more @ README.md

#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <functional>
#include <cstdlib>
#include <sstream>

namespace mflags {

struct GlobalState {
  // vector(pair(flag_name_string, function_to_run_to_set_its_value))
  // In case of `DECLARE_MFLAG(int, price)`, the @flag_name_string is "price"
  // and function = [](const char* value) { MFLAGS_price = str2int(value); }
  std::vector<std::pair<
      std::string, std::function<void(const char*)>>> unresolved_flags;

  // map(name -> (is_resolved, value))
  // In case of `--price 110`, the "price" is @name and "110" is @value.
  // The @is_resolved is false by default ; When the C++ variable
  // `int MFLAGS_price` is updated to 110, the @is_resolved becomes true.
  std::map<std::string, const char*> command_line_options;

  std::set<std::string> expected_command_line_option;

  std::ostringstream help_text;
  GlobalState() {
    help_text << "Command line Options:\n\n --help / -h   : Display help text\n\n";
  }
};

GlobalState& GlobalStateInstance();

struct AllowDynamicFlag {
  AllowDynamicFlag(const char* flag) {
    GlobalStateInstance().expected_command_line_option.insert(flag);
  }
};

inline void parsingFailure(const char* name, const char* value,
                           const char* filename) {
  std::cerr << "ERR: Failed to parse command line value '" << value
            << "' for the flag '" << name << "', defined in file " << filename
            << "." << std::endl;
  std::abort();
}

template<typename T>
inline void StrValueToVariable(const char* name, const char* value,
                               const char* filename, T* variable) {
}

inline void StrValueToVariable(const char* name, const char* value,
                               const char* filename, const char** variable) {
  *variable = value;
}

template<typename Lambda>
inline void crashOnFailedToParseFlag(const char* name, const char* value,
                                     const char* filename, Lambda func) {
  try {
    func();
  } catch (const std::exception& e) {
    parsingFailure(name, value, filename);
  }
}

inline void StrValueToVariable(const char* name, const char* value,
                               const char* filename, int* variable) {
  crashOnFailedToParseFlag(name, value, filename, [&](){
    *variable = std::stoi(value);
  });
}

inline void StrValueToVariable(const char* name, const char* value,
                               const char* filename, double* variable) {
  crashOnFailedToParseFlag(name, value, filename, [&](){
    *variable = std::stod(value);
  });
}

inline void StrValueToVariable(const char* name, const char* value,
                               const char* filename, bool* variable) {
  std::string value_str = value;
  if (value_str == "true") {
    *variable = true;
    return;
  }
  if (value_str == "false") {
    *variable = false;
    return;
  }
  parsingFailure(name, value, filename);
}

template<typename T>
class AutoAssign {
 public:
  static_assert(
      std::is_same<T, int>::value || std::is_same<T, bool>::value ||
        std::is_same<T, double>::value || std::is_same<T, const char*>::value,
      "mflags supports flags of type int, bool, double and const char* only.");
  AutoAssign(const char* name, const char* filename, T* variable,
             const char* help_text, const char* type_string) {
    auto& g_state = GlobalStateInstance();
    g_state.help_text << " --" << name << " VALUE  : (" << type_string << ") "
                      << help_text << "\n\n";
    auto it = g_state.command_line_options.find(name);
    if (it != g_state.command_line_options.end()) {
      StrValueToVariable(name, it->second, filename, variable);
    } else {
      g_state.expected_command_line_option.insert(name);
      g_state.unresolved_flags.push_back({name,
        [name, variable, filename](const char* value) {
          StrValueToVariable(name, value, filename, variable);
        }});
    }
  }
};

inline void ParseFlags(int argc, const char** argv) {
  auto& g_state = GlobalStateInstance();
  for (int i = 1; i < argc; ++i) {
    std::string name = argv[i];
    if (name == "--help" or name == "-h") {
      std::cout << g_state.help_text.str() << std::endl;
      std::exit(0);
    }
  }
  for (int i = 1; i < argc; ++i) {
    std::string name = argv[i];
    if (!(name.size() > 2 && name.substr(0, 2) == "--")) {
      std::cerr << "ERR: mflag command line option '" << name << "' should "
        "start with `--`" << std::endl;
      std::abort();
    }
    if (!(i+1 < argc)) {
      std::cerr << "ERR: No value for the mflag command line flag '" << name
                << "'" << std::endl;
      std::abort();
    }
    name = name.substr(2);
    if (g_state.expected_command_line_option.count(name) == 0) {
      std::cerr << "Unknown command line option '--" << name << "'."
                << std::endl;
      std::abort();
    }
    g_state.command_line_options[name] = argv[i+1];
    i++;
  }

  for (auto& it : g_state.unresolved_flags) {
    auto it2 = g_state.command_line_options.find(it.first);
    if (it2 != g_state.command_line_options.end()) {
      it.second(it2->second);
    }
  }
}

}

#define MFLAG_ODR_INIT() ::mflags::GlobalState&                         \
  ::mflags::GlobalStateInstance() { static GlobalState s; return s; }
#define DECLARE_MFLAG(type, var) extern type MFLAGS_ ## var;
#define DEFINE_MFLAG(type, var, default_value, help_text)               \
  type MFLAGS_ ## var = default_value;                                  \
  ::mflags::AutoAssign<type> AutoAssignVar_ ## var {                    \
      #var, __FILE__, &MFLAGS_ ## var, help_text, #type};
#define ALLOW_DYNAMIC_FLAG(var) ::mflags::AllowDynamicFlag AllowDynamicFlagVar_ ## var {#var};
