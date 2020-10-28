#include "Plist.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <filesystem>
#include <pugixml.hpp>
#include <utility>

#include "common.h"

namespace fyber {

using std::map;
using std::set;
using std::string;
using std::vector;
namespace fs = std::filesystem;

struct xml_string_writer : pugi::xml_writer
{
  std::string result;

  void write(const void* data, size_t size) override { result.append(static_cast<const char*>(data), size); }
};

Plist::Plist(string file_path) : _file_path(std::move(file_path)), _sk_ad_network_items(set<string>())
{
  _sk_ad_network_items = parseFile();
}

set<string> Plist::parseFile()
{
  pugi::xml_parse_result result = _doc.load_file(_file_path.c_str(), pugi::parse_full);

  if (result) {
    set<string> collected_items = set<string>();

    auto skAdNetworkItems_key = _doc.child("plist").child(plist_dict).find_child([](pugi::xml_node node) {
      return name_is(node, plist_key) and value_is(node, plist_SKAdNetworkItems);
    });

    if (!skAdNetworkItems_key.empty()) {

      auto sk_items = skAdNetworkItems_key.next_sibling(plist_array).children();

      for (auto& item : sk_items) {
        if (name_is(item, plist_dict) and value_is(item.child(plist_key), plist_SKAdNetworkIdentifier)) {
          collected_items.emplace(item.child_value(plist_string));
        }
      }

      spdlog::debug("Extracted from [" + _file_path + "] : [" + fyber::common::join(collected_items, ",") + "]");
    }

    return collected_items;
  } else {
    spdlog::error("XML [" + _file_path + "] parsed with errors");
    spdlog::error("error at [" + (_file_path + ":" + std::to_string(result.offset)) + "]");

    throw ExitMessage::InvalidPlist("Plist XML: " + string(result.description()));
  }
}

bool Plist::value_is(const pugi::xml_node& item, const char* text)
{
  return std::strcmp(item.child_value(), text) == 0;
}

bool Plist::name_is(const pugi::xml_node& item, const char* text)
{
  return std::strcmp(item.name(), text) == 0;
}

string Plist::existing_sk_ad_network_items_str()
{
  return fyber::common::join(_sk_ad_network_items, ", ");
}

string Plist::new_sk_ad_network_items_str()
{
  return fyber::common::join(_new_sk_ad_network_items, ", ");
}

bool Plist::set_sk_ad_network_items_for_update(const std::map<string, vector<string>>& received_sk_ad_networks)
{
  set<string> new_items;

  const set<std::string>& received_items = fyber::common::get_value_set(received_sk_ad_networks);

  for (const auto& received_item : received_items) {
    if (_sk_ad_network_items.count(received_item) == 0) {
      new_items.emplace(received_item);
    }
  }

  spdlog::debug("Set New SKAdNetworkItems: {}", fyber::common::join(new_items, ", "));

  _network_items_mapping = received_sk_ad_networks;
  _new_sk_ad_network_items = new_items;

  return should_update();
}

bool Plist::should_update()
{
  return !_new_sk_ad_network_items.empty();
}

string Plist::build_plist_SKAdNetworkItems()
{
  pugi::xml_document new_doc;
  new_doc.reset(_doc);

  pugi::xml_node plist_main_dictionary = new_doc.child("plist").child(plist_dict);

  pugi::xml_node skAdNetworkItems_key = get_or_create_SKAdNetworkItems_xml(plist_main_dictionary);

  pugi::xml_node sk_items = get_or_create_items_array_xml(plist_main_dictionary, skAdNetworkItems_key);

  create_new_SKAdNetwork_items(sk_items);

  xml_string_writer writer_new;
  new_doc.save(writer_new);

  spdlog::debug("New Info.plist: \n{}", writer_new.result);

  _new_doc.reset(new_doc);

  return writer_new.result;
}

void Plist::create_new_SKAdNetwork_items(pugi::xml_node& sk_items) const
{
  for (const auto& new_item : _new_sk_ad_network_items) {
    pugi::xml_node new_identifier = sk_items.append_child(plist_dict);

    // clang-format off
      new_identifier
          .append_child(plist_key)
          .append_child(pugi::node_pcdata)
          .set_value(plist_SKAdNetworkIdentifier);

      new_identifier
          .append_child(plist_string)
          .append_child(pugi::node_pcdata)
          .set_value(new_item.c_str());
    // clang-format on
  }
}

pugi::xml_node Plist::get_or_create_items_array_xml(pugi::xml_node& plist_main_dictionary,
                                                    const pugi::xml_node& skAdNetworkItems_key)
{
  pugi::xml_node sk_items = skAdNetworkItems_key.next_sibling(plist_array);

  if (sk_items.empty()) {
    sk_items = plist_main_dictionary.append_child(plist_array);
  }
  return sk_items;
}

pugi::xml_node Plist::get_or_create_SKAdNetworkItems_xml(pugi::xml_node& plist_main_dictionary)
{
  pugi::xml_node skAdNetworkItems_key = plist_main_dictionary.find_child(
      [](pugi::xml_node node) { return name_is(node, plist_key) and value_is(node, plist_SKAdNetworkItems); });

  if (skAdNetworkItems_key.empty()) {
    skAdNetworkItems_key = plist_main_dictionary.append_child(plist_key);
    skAdNetworkItems_key.append_child(pugi::node_pcdata).set_value(plist_SKAdNetworkItems);
  }
  return skAdNetworkItems_key;
}

void Plist::update_file(bool backup)
{
  std::filesystem::path path(_file_path);
  std::string file_name = path.filename();
  if (backup) {
    auto next_backup_id = std::to_string(get_next_backup_id(path, file_name));

    _backup_name = path.parent_path() / (file_name + backup_extension + next_backup_id);

    fs::copy(path, _backup_name);
    spdlog::info("Backup `{}` created at `{}`", _file_path, _backup_name);
  }

  auto saved = _new_doc.save_file(_file_path.c_str());
  spdlog::info("Saving new `{}` = {}", _file_path, saved);
}

int Plist::get_next_backup_id(const std::filesystem::path& path, const string& file_name)
{
  int max = 0;
  int curr;
  for (const auto& entry : fs::directory_iterator(path.parent_path())) {
    if (common::starts_with(entry.path().filename(), file_name + backup_extension)) {
      auto ext = entry.path().filename().extension().string().erase(0, 1);
      if (common::is_integer(ext)) {
        curr = std::stoi(ext);
        max = (max < curr ? curr : max);
      }
    }
  }
  return max + 1;
}

}  // namespace fyber
