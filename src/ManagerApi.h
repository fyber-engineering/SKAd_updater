#pragma once
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace fyber {
using std::map;
using std::optional;
using std::string;
using std::tuple;
using std::vector;

/// The API with the SKAdNetwork manager service in Fyber
class ManagerApi
{
 private:
  const string API_URL;
  static string GET_request(const string& endpoint, optional<tuple<string, string>> param);

  static vector<string> parse_networks_response(const char* body);
  static map<string, vector<string>> parse_plist_response(const char* body);

  static void log_sk_ad_networks(const map<string, vector<string>>& sk_ad_networks);

 public:
  explicit ManagerApi(string url);

  /// Get a list of network names. <br/>
  /// Using the api call: https://network-setup.fyber.com/networks
  /// \return list of network names
  [[nodiscard]] vector<string> get_networks() const;

  /// Get a Mapping from 'Network Name' (as used in the podfile) to a list of SKAdNetwork IDs.<br/>
  /// Using this api call: https://network-setup.fyber.com/plist?network_list=<comma-separated-networks>
  /// \param networks list of network names
  /// \return map of network names to IDs
  [[nodiscard]] map<string, vector<string>> get_sk_ad_networks(const vector<string>& networks) const;
};

}  // namespace fyber
