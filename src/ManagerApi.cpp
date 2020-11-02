#include "ManagerApi.h"

#include <cpr/cpr.h>

#include <optional>
#include <tuple>

#include "common.h"
#include "exit_message.h"
#include "rapidjson/document.h"
#include "spdlog/spdlog.h"

namespace fyber {
using std::optional;
using std::tuple;

ManagerApi::ManagerApi(string url) : API_URL(std::move(url))
{
  spdlog::debug("Remote set to {}", API_URL);
}

vector<string> ManagerApi::get_networks() const
{
  auto response = GET_request(API_URL + "/networks", std::nullopt);
  vector<string> networks = parse_networks_response(response.c_str());

  spdlog::debug("Returned networks: {} ", common::join(networks, ","));

  return networks;
}

/// Parses a response with this format:
///\code  {"networks": [AdColony, Google-Mobile-Ads-SDK, AppLovinSDK, ... ]}
vector<string> ManagerApi::parse_networks_response(const char* body)
{
  vector<string> networks;

  rapidjson::Document doc;
  doc.Parse(body);

  if (doc.HasParseError()) {
    throw ExitMessage::InvalidNetworks("Invalid Networks returned from server: " + std::to_string(doc.GetParseError()));
  }

  try {
    for (auto& network : doc["networks"].GetArray()) {
      networks.emplace_back(network.GetString());
    }
  } catch (std::exception& ex) {
    throw ExitMessage::InvalidNetworks("Networks parsing error: " + string(ex.what()));
  }

  return networks;
}

map<string, vector<string>> ManagerApi::get_sk_ad_networks(const vector<string>& networks) const
{
  const string& req_networks_str = common::join(networks, ",");
  auto response = GET_request(API_URL + "/plist", std::make_tuple("network_list", req_networks_str));

  map<string, vector<string>> sk_ad_networks = parse_plist_response(response.c_str());

  log_sk_ad_networks(sk_ad_networks);

  return sk_ad_networks;
}

/// Parses a response with this format:
///\code
/// {
///         "Adcolony": ["4PFYVQ9L8R", "YCLNXRL5PM",..],
///         "Google-Mobile-Ads-SDK": ["cstr6suwn9"], "Applovin":
///         ["ludvb6z3bs"], "Unknown_network": []
/// }
map<string, vector<string>> ManagerApi::parse_plist_response(const char* body)
{

  map<string, vector<string>> sdk_ad_networks;

  rapidjson::Document doc;
  doc.Parse(body);

  if (doc.HasParseError()) {
    throw ExitMessage::InvalidNetworks("Invalid Networks returned from server: " + std::to_string(doc.GetParseError()));
  }

  try {
    for (auto& network : doc.GetObject()) {
      const char* network_name = network.name.GetString();

      vector<string> network_values;
      for (auto& v : network.value.GetArray()) {
        network_values.emplace_back(v.GetString());
      }

      sdk_ad_networks.emplace(network_name, network_values);
    }
  } catch (std::exception& ex) {
    throw ExitMessage::InvalidNetworks("SKAdNetworks parsing error: " + string(ex.what()));
  }

  return sdk_ad_networks;
}

/// Perform a GET request to [[endpoint]] with a single optional parameter [[param]]
/// \param endpoint
/// \param param
/// \return response body
/// \throws
string ManagerApi::GET_request(const string& endpoint, optional<tuple<string, string>> param)
{
  cpr::Parameters parameters;
  if (param.has_value()) {
    auto [key, value] = param.value();
    parameters = cpr::Parameters{{key.c_str(), value.c_str()}};
  }

  cpr::Response r = cpr::Get(cpr::Url{endpoint}, parameters);

  spdlog::debug("{} Returned ({}) : {}", endpoint, r.status_code, r.text);

  if (r.error) {
    throw ExitMessage::RemoteAPIFailure("API failure for '" + endpoint + "': " + r.error.message);
  }

  if (r.status_code != 200) {
    throw ExitMessage::ServerUnavailable("Connection to '" + endpoint + "' failed (" + r.status_line + ") : " + r.text);
  }

  return r.text;
}

void ManagerApi::log_sk_ad_networks(const map<string, vector<string>>& sk_ad_networks)
{
  string sk_ad_networks_str = "{";
  for (auto& [network_name, values] : sk_ad_networks) {
    sk_ad_networks_str += network_name + ": [" + common::join(values, ",") + "]\n";
  }
  sk_ad_networks_str += "}";

  spdlog::debug("returned sk_ad_networks: {} ", sk_ad_networks_str);
}

}  // namespace fyber