#pragma once
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace fyber {

struct common
{
  /// join a collection of strings into a single string using a delimiter
  /// \param col
  /// \param delimiter
  /// \tparam C an iterable type
  /// \return joined string
  template <typename C>
  static std::string join(const C& col, const std::string& delimiter)
  {
    std::string s;

    int i = 0;
    for (const auto& piece : col) {
      i++;
      s += piece + (i < col.size() ? delimiter : "");
    }

    return s;
  }

  /// Split a string to a vector of strings using a delimiter.
  /// \param text
  /// \param delimiter
  /// \return vector of the original strings parts
  static std::vector<string> split(const std::string& text, char delimiter)
  {
    std::vector<string> v;
    std::string token;
    for (char c : text) {
      if (c == delimiter) {
        v.push_back(token);
        token = "";
      } else {
        token.append(1, c);
      }
    }

    if (!token.empty()) v.push_back(token);

    return v;
  }

  /// Flatten a map to get all its values in a single vector.
  /// \return
  static std::set<std::string> get_value_set(const std::map<string, std::vector<std::string>>& keys_values)
  {
    std::set<std::string> map_values;

    for (auto [k, vs] : keys_values) {
      for (auto& v : vs) {
        map_values.emplace(v);
      }
    }

    return map_values;
  }

  /// Determine if a string [str] starts with another string [term]
  static bool starts_with(const string& str, const string& term) { return str.rfind(term, 0) == 0; }

  /// Determine if a string [str] is a positive integer
  static bool is_integer(const std::string& str)
  {
    return !str.empty() and str.find_first_not_of("0123456789") == string::npos;
  }

  /// trim spaces from start
  static inline std::string ltrim(std::string s)
  {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    return s;
  }

  ///  trim spaces from end
  static inline std::string rtrim(std::string s)
  {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
  }
};

}  // namespace fyber