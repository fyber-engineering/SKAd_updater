#pragma once

#include <cxxopts.hpp>
#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

#include "exit_message.h"

namespace fyber {

using std::optional;
using std::string;
using std::vector;

struct Options
{
  const optional<string> show_help;
  const optional<string> plist_file_path;
  const optional<string> pod_file_path;
  const optional<vector<string>> network_list;
  const bool dry_run = false;
  const bool show_networks = false;

  Options(optional<string> showHelp, optional<string> plistPath, optional<string> podPath,
          optional<vector<string>> networkList, bool dryRun, bool showNetworks);

  [[nodiscard]] string to_string() const;
};

/// Facilitate CLI options
class cli
{
 private:
  static inline const char* plist_file_path_Id = "plist_file_path";
  static inline const char* network_list_Id = "network_list";
  static inline const char* pod_file_path_Id = "pod_file_path";
  static inline const char* dry_run_Id = "dry_run";
  static inline const char* show_networks_Id = "show_networks";
  static inline const char* help_Id = "help";

  static Options buildOptions(const cxxopts::ParseResult& result, const cxxopts::Options& options);

 public:
  /// Reads valid arguments into an `Options` object
  static Options read_args(int argc, char** argv);
};

}  // namespace fyber
