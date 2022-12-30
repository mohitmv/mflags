// Author: Mohit <mohitsaini1196@gmail.com>
// Github: https://github.com/mohitmv/mflags

// MFLAGS : A super simple yet powerful command line flags library.
// Learn more @ README.md

#ifndef MFLAGS_H
#define MFLAGS_H

#include <vector>
#include <string_view>
#include <functional>
#include <cstdlib>
#include <sstream>
#include <optional>
#include <cassert>

namespace mflags {

struct Status {
  static constexpr struct TypeOk {} OK = {};
  static constexpr struct TypeError {} ERROR = {};
  Status(TypeOk) { } // Implicitly convertiable from Status::OK
  Status(const Status&) = delete;
  Status(Status& other): Status(std::move(other)) { }
  Status(Status&&) = default;
  Status& operator=(const Status&) = delete;
  Status& operator=(Status&&) = default;
  Status& operator=(Status& other) { return *this = std::move(other); }
  static Status Error(std::string e="") { return Status(Status::ERROR, e); }
  constexpr bool ok() const { return !error.has_value(); }
  constexpr operator bool() const { return ok(); }
  std::string str() const { return ok() ? "" : error->str(); }
  template<typename T>
  Status& operator<<(const T& x) { assert(!ok()); *error << x; return *this; }

 private:
  Status(TypeError, std::string e): error{std::in_place} { *error << e; }
  std::optional<std::ostringstream> error;
};

struct ArgDescOpts {
  std::vector<std::string> names;
  bool required = false;
  bool positional = false;
  std::string help_text;
};

struct FieldArgs {
  std::string_view field_name;
  std::vector<const char*> args;
};

// Descriptor for one argument field.
struct OneArgDesc {
  ArgDescOpts opts;
  std::function<Status(const FieldArgs& field_args)> parse_func;
  const char* filename = "";
  int num_needed_args = 1;
  bool is_bool = false;
  bool variable_num_args = false;
};

namespace mflags_impl {

template<typename...> struct Typechain {};

template<typename...> struct RankOf;

template<typename T, typename... Ts>
struct RankOf<T, Typechain<Ts...>> : std::integral_constant<size_t, 0> { };

template<typename S, typename H, typename... T>
struct RankOf<S, Typechain<H, T...>> {
  static constexpr size_t value = std::is_same<S, H>::value ?
    sizeof...(T) + 1 : RankOf<S, Typechain<T...>>::value;
};

template<typename TypechainT, typename T>
struct Contains {
  static constexpr bool value = (RankOf<T, TypechainT>::value > 0);
};

using CoreTypes = Typechain<int, char, bool, std::string, const char*, double>;

template<typename T>
using IsCoreType = Contains<CoreTypes, T>;

template<typename T> struct IsTupleOfCoreTypes : std::false_type { };

template<typename T1, typename T2>
struct IsTupleOfCoreTypes<std::pair<T1, T2>> : std::bool_constant<
  IsCoreType<T1>::value && IsCoreType<T2>::value> { };

template<typename T> struct IsVectorOfCoreTypes : std::false_type { };

template<typename T> 
struct IsVectorOfCoreTypes<std::vector<T>> : IsCoreType<T> { };

template<typename T> struct IsVectorOfTupleOfCoreTypes : std::false_type { };

template<typename T> 
struct IsVectorOfTupleOfCoreTypes<std::vector<T>> : IsTupleOfCoreTypes<T> { };

template<typename T>
using IsSupportedType = std::bool_constant<
  IsCoreType<T>::value || IsVectorOfCoreTypes<T>::value ||
  IsTupleOfCoreTypes<T>::value || IsVectorOfTupleOfCoreTypes<T>::value>;


template< class T >
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

inline std::string TypeStrImpl(int) { return "int"; }
inline std::string TypeStrImpl(double) { return "double"; }
inline std::string TypeStrImpl(bool) { return "bool"; }
inline std::string TypeStrImpl(char) { return "char"; }
inline std::string TypeStrImpl(std::string) { return "string"; }
inline std::string TypeStrImpl(const char*) { return "const char*"; }

template<typename T1, typename T2>
inline std::string TypeStrImpl(std::pair<T1, T2>) {
  return "pair<" + TypeStrImpl(T1{}) + ", " + TypeStrImpl(T2{}) + ">";
}

template<typename T>
inline std::string TypeStrImpl(std::vector<T>) {
  return "vector<" + TypeStrImpl(T{}) + ">";
}

template<typename T>
inline std::string TypeStr() { return TypeStrImpl(T{}); }

// GlobalState& GlobalStateInstance();

inline bool IsBoolString(const char* str) {
  return (std::string_view(str) == "true" || std::string_view(str) == "false");
}

inline bool CstrToCoreTypes(const char* str, const char*& output) {
  output = str;
  return true;
}

inline bool CstrToCoreTypes(const char* str, bool& output) {
  if (!IsBoolString(str)) return false;
  output = (std::string_view(str) == "true");
  return true;
}

inline bool CstrToCoreTypes(const char* str, char& output) {
  std::string_view sv(str);
  if (sv.size() != 1) return false;
  output = sv[0];
  return true;
}

template<typename T>
inline bool CstrToCoreTypes(const char* str, T& output) {
  std::stringstream ss(str);
  T tmp;
  ss >> tmp;
  bool ok = (!ss.fail()) && ss.eof();
  if (ok) output = tmp;
  return ok;
}

template<typename T>
inline Status ParseCoreTypes(const FieldArgs& field_args, T& output) {
  if constexpr (std::is_same<remove_cvref_t<T>, bool>::value) {
    if (field_args.args.size() == 0) {
      output = true;
      return Status::OK;
    }
  }
  if (field_args.args.size() != 1) {
    return Status::Error("Invalid number of args for `") << field_args.field_name
      << "`. Expected " << 1 << " found " << field_args.args.size() << ". Should be "
      << "of type " << TypeStr<T>();
  }
  if (CstrToCoreTypes(field_args.args.at(0), output)) return Status::OK;
  return Status::Error("Failed to parse `") << field_args.args.at(0) << "` as type "
    << TypeStr<T>() << " for field " << field_args.field_name;
}

template<typename T>
inline Status ParseCoreTypesVector(const FieldArgs& field_args, std::vector<T>& output) {
  for (auto& arg: field_args.args) {
    T tmp;
    if (!CstrToCoreTypes(arg, tmp)) {
      return Status::Error("Failed to parse `") << arg << "` as type "
        << TypeStr<T>() << " for field " << field_args.field_name;
    }
    output.push_back(tmp);
  }
  return Status::OK;
}

template<typename T1, typename T2>
inline Status ParseCoreTypesTuple(const FieldArgs& field_args, std::pair<T1, T2>& output) {
  auto&& args = field_args.args;
  auto&& field_name = field_args.field_name;
  if (args.size() != 2) {
    return Status::Error("Invalid number of args for `") << field_name
      << "`. Expected " << 2 << " found " << args.size() << ". Should be "
      << "parsable for " << TypeStr<std::pair<T1, T2>>();
  }
  if (!CstrToCoreTypes(args[0], output.first)) {
    return Status::Error("Failed to parse `") << args[0] << "` as type "
      << TypeStr<T1>() << " for field " << field_name
      << ". Expected args of " << field_name << " to be parsable for "
      << TypeStr<std::pair<T1, T2>>();
  }
  if (!CstrToCoreTypes(args[1], output.second)) {
    return Status::Error("Failed to parse `") << args[1] << "` as type "
      << TypeStr<T2>() << " for field " << field_name
      << ". Expected args of " << field_name << " to be parsable for "
      << TypeStr<std::pair<T1, T2>>();
  }
  return Status::OK;
}


// Parse a vector of tuple/pair of primitive types.
template<typename T>
inline Status ParseCoreTypesTuplesVector(const FieldArgs& field_args,
                                   std::vector<T>& output) {
  T tmp;
  auto status = ParseCoreTypesTuple(field_args, tmp);
  if (status.ok()) output.push_back(tmp);
  return status;
}

template<typename T>
inline OneArgDesc MakeArgDesc(ArgDescOpts opts, T& bound_variable) {
  using Type = remove_cvref_t<T>;
  static_assert(IsSupportedType<Type>::value, "Unsupported arg data type");
  OneArgDesc output{.opts=opts};
  if constexpr (IsCoreType<Type>::value) {
    output.parse_func = [&bound_variable](const FieldArgs& field_args) {
      return ParseCoreTypes(field_args, bound_variable);
    };
  } else if constexpr (IsTupleOfCoreTypes<Type>::value) {
    output.num_needed_args = 2;  // TODO: handle tuple as well.
    output.parse_func = [&bound_variable](const FieldArgs& field_args) {
      return ParseCoreTypesTuple(field_args, bound_variable);
    };
  } else if constexpr (IsVectorOfCoreTypes<Type>::value) {
    output.variable_num_args = true;
    output.parse_func = [&bound_variable](const FieldArgs& field_args) {
      return ParseCoreTypesVector(field_args, bound_variable);
    };
  } else if constexpr (IsVectorOfTupleOfCoreTypes<Type>::value) {
    output.num_needed_args = 2;  // TODO: handle tuple as well.
    output.parse_func = [&bound_variable](const FieldArgs& field_args) {
      return ParseCoreTypesTuplesVector(field_args, bound_variable);
    };
  }
  output.is_bool = std::is_same<Type, bool>::value;
  return output;
}

}  // namespace mflags_impl

// Overall arguments descriptor.
class ArgsDescriptor {
 public:
  ArgsDescriptor(): ArgsDescriptor("") { }
  ArgsDescriptor(std::string help_text);
  ArgsDescriptor(const ArgsDescriptor&) = delete;
  ArgsDescriptor& operator=(const ArgsDescriptor&) = delete;
  template<typename T>
  void AddArg(ArgDescOpts opts, T* bound_variable);
  void AddArgList(std::vector<OneArgDesc> list) {
    arg_desc_list_.insert(arg_desc_list_.end(), list.begin(), list.end());
  }
  void ParseFlags(int argc, const char* const* argv) const;
  Status ParseFlagsInternal(int argc, const char* const* argv) const;
  Status ParseFlagsInternal(const std::vector<const char*>& argv) const;
  const auto& DescList() const { return arg_desc_list_;}
  std::string FullHelpText() const { return "TODO"; };
  void AddGlobalDesc() {}

 private:
  std::string help_text_;
  bool help_opt_ = false;
  std::vector<OneArgDesc> arg_desc_list_;
};


template<typename T>
inline void ArgsDescriptor::AddArg(ArgDescOpts opts, T* bound_variable) {
  arg_desc_list_.push_back(mflags_impl::MakeArgDesc(opts, *bound_variable));
}

void ParseFlags(int argc, const char* const* argv);

}  // namespace mflags

// #define MFLAG_ODR_INIT() ::mflags::GlobalState&                         \
//   ::mflags::GlobalStateInstance() { static GlobalState s; return s; }
// #define DECLARE_MFLAG(type, var) extern type MFLAGS_ ## var;
// #define DEFINE_MFLAG(type, var, default_value, help_text)               \
//   type MFLAGS_ ## var = default_value;                                  \
//   ::mflags::AutoAssign<type> AutoAssignVar_ ## var {                    \
//       #var, __FILE__, &MFLAGS_ ## var, help_text, #type};

#endif  // MFLAGS_H
