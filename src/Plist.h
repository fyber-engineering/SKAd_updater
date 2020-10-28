#pragma once

#include <filesystem>
#include <map>
#include <pugixml.hpp>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include "exit_message.h"

namespace fyber {
using std::map;
using std::set;
using std::string;
using std::vector;

/// An abstraction over the `Info.Plist` file
class Plist
{
 private:
  const string _file_path;
  string _backup_name;
  set<string> _sk_ad_network_items;
  map<string, vector<string>> _network_items_mapping;
  set<string> _new_sk_ad_network_items;
  pugi::xml_document _doc;
  pugi::xml_document _new_doc;

  set<string> parseFile();
  static bool value_is(const pugi::xml_node& item, const char* text);
  static bool name_is(const pugi::xml_node& item, const char* text);
  static int get_next_backup_id(const std::filesystem::path& path, const string& file_name);
  static pugi::xml_node get_or_create_SKAdNetworkItems_xml(pugi::xml_node& plist_main_dictionary);
  static pugi::xml_node get_or_create_items_array_xml(pugi::xml_node& plist_main_dictionary,
                                                      const pugi::xml_node& skAdNetworkItems_key);
  void create_new_SKAdNetwork_items(pugi::xml_node& sk_items) const;

  inline static const char* plist_dict = "dict";
  inline static const char* plist_array = "array";
  inline static const char* plist_key = "key";
  inline static const char* plist_string = "string";
  inline static const char* plist_SKAdNetworkItems = "SKAdNetworkItems";
  inline static const char* plist_SKAdNetworkIdentifier = "SKAdNetworkIdentifier";

  inline static const char* backup_extension = ".bak.";

 public:
  /// Load and parse the Info.Plist file in [file_path]
  explicit Plist(string file_path);

  /// Setup new SKAdNetworkItems for update. <br/>
  /// Finds the difference with the existing SKAdNetworks and determines whether there are <b>new</b> network IDs.
  /// \param received_sk_ad_networks
  /// \return whether there's something to update in the actual file
  bool set_sk_ad_network_items_for_update(const map<string, vector<string>>& received_sk_ad_networks);

  /// Return whether there's something to update in the actual file
  bool should_update();

  /// Build a new Info.Plist XML based on the existing file, and the sk_ad_networks that needed to be added.
  /// \return Raw XML string
  string build_plist_SKAdNetworkItems();

  /// Perform an update of the actual Plist file.<br/>
  /// If [backup] is passed, create indexed backup files with the extension `bak.X` where `X` is the last number of
  /// update.
  /// \param backup - whether it should create a backup
  void update_file(bool backup);

  /// get a string of the currently existing SKAdNetworks items.
  string existing_sk_ad_network_items_str();

  /// get a string of the new SKAdNetworks items.
  string new_sk_ad_network_items_str();
};

}  // namespace fyber