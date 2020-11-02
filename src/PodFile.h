#pragma once
#include <string>
#include <vector>

namespace fyber {

using std::string;
using std::vector;

/// An abstraction over the `Podfile` file
class PodFile
{
 private:
  const string _pod_file_path;
  vector<string> _found_networks;

  vector<string> parseFile(const vector<string>& supported_networks);

  static void find_sk_ad_network(const vector<string>& supported_networks, const string& trimmed, vector<string>& pods);

 public:
  /// Parse the podfile in [pod_file_path], by matching the network name list of [supported_networks].
  explicit PodFile(string pod_file_path, const vector<string>& supported_networks);

  /// Get the list of networks used in the podfile.
  vector<string> get_used_networks() { return _found_networks; }
};

}  // namespace fyber
