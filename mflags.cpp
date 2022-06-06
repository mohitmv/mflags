#include <limits>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <iostream>
#include <functional>
#include <cstdlib>
#include <sstream>
#include <string_view>
#include <optional>

#include "mflags.h"

namespace mflags {
namespace {

bool SplitOnEqual(std::string_view sv, std::string_view& first,
                  std::string_view& second) {
  for (size_t i = 0; i < sv.size(); i++) {
    if (sv[i] == '=') {
      first = std::string_view(sv.data(), i);
      second = std::string_view(sv.data() + i + 1, sv.size() - i - 1);
      return true;
    }
  }
  return false;
}

class Parser {
 public:
  Parser() = default;

  Status ParseFlags(int argc, const char* const* argv,
                    std::vector<OneArgDesc> arg_desc_list);

 private:

  Status PreprocessArgDescList();

  void CreateFieldValues(int argc, const char* const* argv);

  Status ParsePositionalArgs();

 private:
  std::vector<OneArgDesc> arg_desc_list_;

  // Generally field_names are of form "--flag"
  std::map<std::string_view, const OneArgDesc*> field_names_map_;
  
  std::vector<FieldArgs> field_values_;
  std::vector<const char*> positional_args_;
};


Status Parser::PreprocessArgDescList() {
  for (auto& desc : arg_desc_list_) {
    for (auto& name : desc.opts.names) {
      std::string_view name_sv{name};
      if (field_names_map_.count(name_sv)) {
        return Status::Error("Field name `") << name_sv
          << "` declared twice across in "
          << "argument descriptions. One at " << field_names_map_[name_sv]->filename
          << " and other one at " << desc.filename;
      }
      field_names_map_[name_sv] = &desc;
    }
  }
  return Status::OK;
}

template<typename T>
auto SliceVector(const std::vector<T>& v, size_t start, size_t len) {
  return std::vector<T>(v.begin() + std::min(v.size(), start), v.begin() + std::min(v.size(), start + len));
}

template<typename T>
auto SliceVector(const std::vector<T>& v, size_t start) {
  return std::vector<T>(v.begin() + std::min(v.size(), start), v.end());
}

Status Parser::ParsePositionalArgs() {
  size_t positional_args_offset = 0;
  for (auto& arg : arg_desc_list_) {
    if (!arg.opts.positional) continue;
    auto name = (arg.opts.names.size() > 0 ? arg.opts.names[0]: "");
    if (arg.variable_num_args) {
      auto status = arg.parse_func({"Positional " + name, SliceVector(positional_args_, positional_args_offset)});
      if (!status.ok()) return status;
      positional_args_offset = positional_args_.size();
    } else {
      auto args = SliceVector(positional_args_, positional_args_offset, arg.num_needed_args);
      if (args.size() == 0 && arg.opts.required) {
        return Status::Error("Required positional arg ") << name << " not found.";
      }
      if (args.size() > 0) {
        auto status = arg.parse_func({"Positional " + name, args});
        if (!status.ok()) return status;
        positional_args_offset += arg.num_needed_args;
      }
    }
  }
  auto remaining_args = SliceVector(positional_args_, positional_args_offset);
  if (remaining_args.size() > 0) {
    return Status::Error("Unrecognized param: ") << mflags_impl::StrJoin(remaining_args, " ");
  }
  return Status::OK;
}


Status Parser::ParseFlags(int argc, const char* const* argv,
                          std::vector<OneArgDesc> arg_desc_list) {
  arg_desc_list_ = std::move(arg_desc_list);
  auto result = PreprocessArgDescList();
  if (!result.ok()) return result;
  CreateFieldValues(argc, argv);
  for (auto& item: field_values_) {
    auto& arg_desc = field_names_map_.at(item.field_name);
    result = arg_desc->parse_func(item);
    if (!result.ok()) return result;
  }
  result = ParsePositionalArgs();
  if (!result.ok()) return result;
  return Status::OK;
}


void Parser::CreateFieldValues(int argc, const char* const* argv) {
  int num_needed_optional_args = 0;
  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    std::string_view first, second;
    if (SplitOnEqual(arg, first, second) && field_names_map_.count(first)) {
      field_values_.push_back({first, {second.data()}});
      num_needed_optional_args = 0;
      continue;
    }
    if (field_names_map_.count(arg)) {
      auto& arg_desc = field_names_map_.at(arg);
      field_values_.push_back({arg, {}});
      if (arg_desc->variable_num_args) {
        num_needed_optional_args = std::numeric_limits<int>::max();
        continue;
      };
      if (arg_desc->is_bool) {
        num_needed_optional_args = 0;
        if (i + 1 < argc && mflags_impl::IsBoolString(argv[i+1])) {
          field_values_.back().args.push_back(argv[i+1]);
          i++;
        }
      } else {
        num_needed_optional_args = arg_desc->num_needed_args;
      }
      continue;
    }
    if (num_needed_optional_args > 0) {
      field_values_.back().args.push_back(argv[i]);
      num_needed_optional_args--;
    } else {
      positional_args_.push_back(argv[i]);
    }
  }
}

} // namespace

std::string mflags_impl::ValueString(int num_needed_args) {
  std::ostringstream oss;
  for (int i = 0; i < num_needed_args; i++) {
    oss << " ";
    oss << "VALUE" << (i+1);
  }
  return oss.str();
}

ArgsDescriptor::ArgsDescriptor(std::string help_text)
    : help_text_(help_text) {
  AddArg({
    .names={"-h", "--help"},
    .help_text="Show this help message and exit"}, &help_opt_);
  AddArgList(mflags_impl::GlobalArgDescList());
}

void ArgsDescriptor::ParseFlags(int argc, const char* const* argv) const {
  auto status = ParseFlagsInternal(argc, argv);
  if (help_opt_) {
    std::cerr << FullHelpText() << std::endl;
    std::exit(0);
  }
  if (!status.ok()) {
    std::cerr << status.str() << std::endl;
    std::exit(0);
  }
}


std::string ArgsDescriptor::FullHelpText() const {
  constexpr size_t left_size_max_size = 24;
  auto lArgHelpString = [&](const OneArgDesc& arg_desc, size_t index) {
    std::ostringstream oss;
    oss << "  ";
    if (arg_desc.opts.positional) {
      auto name = arg_desc.opts.names.size() > 0 ? arg_desc.opts.names[0] : "POSITIONAL";
      oss << name;
    } else {
      oss << arg_desc.help_text_left;
    }
    auto left_side = oss.str();
    if (left_side.size() < left_size_max_size) {
      left_side += std::string(left_size_max_size - left_side.size(), ' ');
    } else {
      left_side += "\n" + std::string(left_size_max_size, ' ');
    }
    std::ostringstream oss2;
    oss2 << left_side << "  " << arg_desc.opts.help_text;
    if (index > 0) {
      if (!(arg_desc.type_string == "string" && arg_desc.default_value_str == "\"\"")) {
        oss2 << (arg_desc.opts.help_text.size() > 0 ? ". " : "");
        oss2 << "Type: " << arg_desc.type_string;
        if (!arg_desc.default_value_str.empty()) {
          oss2 << " ; default: " << arg_desc.default_value_str;
        }
      }
    }
    oss2 << "\n";
    return oss2.str();
  };

  std::ostringstream oss;
  oss << "\n";
  oss << help_text_ << "\n\n";
  oss << "Positional Arguments:\n\n";
  for (size_t i = 0 ; i < arg_desc_list_.size(); ++i) {
    if (!arg_desc_list_[i].opts.positional) continue;
    oss << lArgHelpString(arg_desc_list_[i], i);
  }
  oss << "\nOptional Arguments:\n\n";
  for (size_t i = 0 ; i < arg_desc_list_.size(); ++i) {
    if (arg_desc_list_[i].opts.positional) continue;
    if (!arg_desc_list_[i].opts.include_in_help_text) continue;
    oss << lArgHelpString(arg_desc_list_[i], i);
  }
  return oss.str();
}

void ParseFlags(int argc, const char* const* argv) {
  ArgsDescriptor().ParseFlags(argc, argv);
}

Status ArgsDescriptor::ParseFlagsInternal(
      int argc, const char* const* argv) const {
  return Parser().ParseFlags(argc, argv, DescList());
}

Status ArgsDescriptor::ParseFlagsInternal(
      const std::vector<const char*>& argv) const {
  return ParseFlagsInternal(static_cast<int>(argv.size()), argv.data());
}

std::vector<OneArgDesc>& mflags_impl::GlobalArgDescList() {
  static std::vector<OneArgDesc> s;
  return s;
}

}  // namespace mflags
