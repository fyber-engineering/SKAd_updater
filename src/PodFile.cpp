#include "PodFile.h"

#include <filesystem>
#include <fstream>
#include <utility>

#include "common.h"
#include "exit_message.h"
#include "spdlog/spdlog.h"

namespace fyber {

using std::string;
using std::vector;
namespace fs = std::filesystem;

fyber::PodFile::PodFile(string pod_file_path, const vector<string>& supported_networks)
    : _pod_file_path(std::move(pod_file_path)), _found_networks(vector<string>())
{
  if (!fs::is_regular_file(_pod_file_path)) {
    throw ExitMessage::NotAFile("Provided pod_file_path is invalid : " +
                                common::file_status_to_string(fs::status(_pod_file_path)));
  }

  _found_networks = parseFile(supported_networks);

  spdlog::debug("pod file contains these networks: [{}]", common::join(_found_networks, ","));
}

vector<string> PodFile::parseFile(const vector<string>& supported_networks)
{
  vector<string> pods;
  string line;
  std::ifstream podfile(_pod_file_path);
  if (podfile.is_open()) {
    while (getline(podfile, line)) {
      const string& trimmed = common::ltrim(line);
      if (common::starts_with(trimmed, "pod '")) {

        find_sk_ad_network(supported_networks, trimmed, pods);
      }
    }
    podfile.close();

    return pods;
  } else {

    throw ExitMessage::InvalidPodFile("Unable to open podfile '" + _pod_file_path + "'.");
  }
}

void PodFile::find_sk_ad_network(const vector<string>& supported_networks, const string& trimmed, vector<string>& pods)
{
  for (auto& supported_network : supported_networks) {
    if (common::starts_with(trimmed, "pod '" + supported_network)) {
      pods.emplace_back(supported_network);
      break;
    }
  }
}

}  // namespace fyber