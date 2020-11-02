#include <iostream>
#include <string>
#include <variant>

#include "ManagerApi.h"
#include "Plist.h"
#include "PodFile.h"
#include "cli.h"
#include "common.h"
#include "exit_message.h"
#include "spdlog/spdlog.h"

void set_log_level();
std::vector<std::string> networks_list_by_podfile(const fyber::ManagerApi& manager_api, const fyber::Options& options);
std::vector<std::string> networks_list_by_options(const fyber::Options& options);
void update_network_IDs(const fyber::Options& options, fyber::Plist& plist);

int main(int argc, char** argv)
{
  set_log_level();

  spdlog::set_pattern("%^*** %v%$");

  const char* server_host_override = std::getenv("FYBER_SKAD_NETWORKS_SERVER_HOST");
  auto manager_api =
      fyber::ManagerApi((server_host_override != nullptr) ? server_host_override : "https://network-setup.fyber.com");

  spdlog::info("Welcome to SKAd Updater ( version {} )", skad_updater_VERSION);

  try {

    auto options = fyber::cli::read_args(argc, argv);

    spdlog::debug("options = {}", options.to_string());

    if (options.show_help.has_value()) {
      std::cout << options.show_help.value() << std::endl;
      return 0;
    }

    if (options.show_networks) {
      auto networks = manager_api.get_networks();
      std::cout << "Supported network names: " << fyber::common::join(networks, ",");
      return 0;
    }

    auto plist = fyber::Plist(options.plist_file_path.value());

    spdlog::info("Existing SKAdNetworks: {}", plist.existing_sk_ad_network_items_str());

    std::vector<std::string> network_list;

    if (options.pod_file_path.has_value()) {
      network_list = networks_list_by_podfile(manager_api, options);
    } else {
      network_list = networks_list_by_options(options);
    }

    plist.set_sk_ad_network_items_for_update(manager_api.get_sk_ad_networks(network_list));

    spdlog::info("New SKAdNetworks: {}", plist.new_sk_ad_network_items_str());

    if (plist.should_update()) {
      update_network_IDs(options, plist);
    } else {
      spdlog::info("Nothing to update. `{}` unchanged.", options.plist_file_path.value());
    }

  } catch (fyber::ExitMessage& err) {
    if (err.code == 0) {
      std::cout << err.what() << std::endl;
    } else {
      spdlog::error(err.what());
    }
    return err.code;
  } catch (const std::exception& e) {
    spdlog::error(e.what());
    return -1;
  }

  return 0;
}

/// Get the list of networks to fetch, as defined in the podfile
/// \param manager_api - the API to the SKAdNetwork listing server
/// \param options - cli options
/// \return a list of network names
/// \throws EmptyPodFile if the pod file doesn't contain supported network names.
std::vector<std::string> networks_list_by_podfile(const fyber::ManagerApi& manager_api, const fyber::Options& options)
{
  auto supported_networks = manager_api.get_networks();

  auto podfile = fyber::PodFile(options.pod_file_path.value(), supported_networks);

  auto networks = podfile.get_used_networks();

  if (networks.empty()) {
    throw fyber::ExitMessage::EmptyPodFile("No supported networks found in your Podfile");
  }

  return networks;
}

/// Get the list of networks to fetch provided by the cli options
/// \param options - cli options
/// \return a list of network names
/// \throws EmptyNetworkList if the provided list is empty
std::vector<std::string> networks_list_by_options(const fyber::Options& options)
{
  auto networks = options.network_list.value();

  if (networks.empty()) {
    throw fyber::ExitMessage::EmptyNetworkList("A non-empty list of networks must be provided");
  }
  return networks;
}

/// Update or Print (on `dry_run`) the Info.Plist file
/// \param options - cli options
/// \param plist
void update_network_IDs(const fyber::Options& options, fyber::Plist& plist)
{
  spdlog::info("Updating `{}`", options.plist_file_path.value());

  std::string raw_new_file = plist.build_plist_SKAdNetworkItems();

  if (options.dry_run) {
    spdlog::info("These network IDs will be added: {}", plist.new_sk_ad_network_items_str());
    spdlog::debug("Printing modified `{}`", options.plist_file_path.value());
    spdlog::debug(raw_new_file);
  } else {
    plist.update_file(true);
  }
}

/// Set the log level as DEBUG if the environment variable 'FYBER_SKAD_DEBUG_LOG' exists
void set_log_level()
{
  if (getenv("FYBER_SKAD_DEBUG_LOG")) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }
}
