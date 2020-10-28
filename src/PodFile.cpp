#include "PodFile.h"

#include <fstream>
#include <utility>

#include "common.h"
#include "exit_message.h"
#include "spdlog/spdlog.h"
namespace fyber {

using std::string;
using std::vector;

fyber::PodFile::PodFile(string pod_path, const vector<string>& supported_networks)
    : _pod_path(std::move(pod_path)), _found_networks(vector<string>())
{
  _found_networks = parseFile(supported_networks);

  spdlog::debug("pod file contains these networks: [{}]", common::join(_found_networks, ","));
}

vector<string> PodFile::parseFile(const vector<string>& supported_networks)
{
  vector<string> pods;
  string line;
  std::ifstream podfile(_pod_path);
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

    throw ExitMessage::InvalidPodFile("Unable to open podfile '" + _pod_path + "'.");
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