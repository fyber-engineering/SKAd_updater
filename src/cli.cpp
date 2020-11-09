#include "cli.h"

#include <utility>

#include "common.h"
#include "cxxopts.hpp"
#include "exit_message.h"
#include "spdlog/spdlog.h"

namespace fyber {

using std::move;
using std::optional;
using std::string;
using std::vector;

//------------------- Options -----------------------------------------------

Options::Options(optional<string> showHelp, optional<string> plistPath, optional<string> podPath,
                 optional<vector<string>> networkList, bool dryRun, bool showNetworks)
    : show_help(std::move(showHelp)),
      plist_file_path(move(plistPath)),
      pod_file_path(move(podPath)),
      network_list(move(networkList)),
      dry_run(dryRun),
      show_networks(showNetworks)
{}

string Options::to_string() const
{
  std::stringstream stream;
  stream << "{\nhelp: " << show_help.has_value();
  stream << "\n plist_file_path: " << plist_file_path.value_or("");
  stream << "\n pod_file_path: " << pod_file_path.value_or("");
  stream << "\n network_list: " << (network_list.has_value() ? common::join(network_list.value(), ",") : "");
  stream << "\n dry_run: " << dry_run;
  stream << "\n show_networks: " << show_networks;
  stream << "}\n";
  return stream.str();
}

//------------------- CLI -------------------------------- //

Options cli::read_args(int argc, char **argv)
{
  try {
    cxxopts::Options options("skad_updater", "Automatically update your SKAdNetwork Items");
    // clang-format off
    options.add_options()
        (plist_file_path_Id, "The plist file path", cxxopts::value<string>())
        (network_list_Id, "Request for a specific list of networks to update. "
                          "The argument is a comma separated list of network names", cxxopts::value<string>())
        (pod_file_path_Id, "Update all the networks according to a pod file. "
                      "The argument is the path to the pod file.",cxxopts::value<string>())
        (dry_run_Id, "Perform a dry-run. Prints out the new `plist` file instead of overwriting.")
        (show_networks_Id, "Show the list of supported network names.")
        ("h," + string(help_Id),"Print usage");
    // clang-format on

    auto result = options.parse(argc, argv);

    if (!result.count(help_Id) && !result.count(show_networks_Id)) {

      if (result.count(plist_file_path_Id) == 0) {
        throw ExitMessage::InvalidArguments("Missing required parameter `plist_file_path`.\n" + options.help());
      }

      if (result.count(network_list_Id) == 0 and result.count(pod_file_path_Id) == 0) {
        throw ExitMessage::InvalidArguments(
            "At least one of the parameters `network_list` or `pod_file_path` is required.\n\n" + options.help());
      }
    }

    return buildOptions(result, options);

  } catch (ExitMessage &exitMessage) {
    throw exitMessage;
  } catch (const std::exception &e) {
    throw ExitMessage::InvalidArguments(e.what());
  }
}

Options cli::buildOptions(const cxxopts::ParseResult &result, const cxxopts::Options &options)
{
  optional<vector<string>> maybe_networks = std::nullopt;
  optional<string> maybe_pod_file_path = std::nullopt;
  optional<string> maybe_plist_file_path = std::nullopt;
  optional<string> maybe_show_help = std::nullopt;

  if (result.count(network_list_Id) == 1) {
    maybe_networks = fyber::common::split(result[network_list_Id].as<string>(), ',');
  }

  if (result.count(pod_file_path_Id) == 1) {
    maybe_pod_file_path = result[pod_file_path_Id].as<string>();
  }

  if (result.count(plist_file_path_Id) == 1) {
    maybe_plist_file_path = result[plist_file_path_Id].as<string>();
  }

  if (result.count(help_Id) == 1) {
    maybe_show_help = options.help();
  }

  return Options(maybe_show_help, maybe_plist_file_path, maybe_pod_file_path, maybe_networks,
                 result[dry_run_Id].as<bool>(), result[show_networks_Id].as<bool>());
}

}  // namespace fyber