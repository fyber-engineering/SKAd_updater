#pragma once
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace fyber {

struct ExitMessage : std::runtime_error
{
  const int code = 0;

  ExitMessage(int code, const std::string &message) : std::runtime_error(message), code(code) {}

  static ExitMessage ShowHelp(const std::string &message) { return ExitMessage(0, message); };

  static ExitMessage InvalidArguments(const std::string &message) { return ExitMessage(1, message); };
  static ExitMessage InvalidPlist(const std::string &message) { return ExitMessage(2, message); };
  static ExitMessage InvalidPodFile(const std::string &message) { return ExitMessage(3, message); };
  static ExitMessage EmptyPodFile(const std::string &message) { return ExitMessage(4, message); };
  static ExitMessage EmptyNetworkList(const std::string &message) { return ExitMessage(5, message); };
  static ExitMessage InvalidNetworks(const std::string &message) { return ExitMessage(6, message); };
  static ExitMessage ServerUnavailable(const std::string &message) { return ExitMessage(7, message); };
  static ExitMessage RemoteAPIFailure(const std::string &message) { return ExitMessage(8, message); };
  static ExitMessage NotAFile(const std::string &message) { return ExitMessage(9, message); };

  static ExitMessage Oops(const std::string &message) { return ExitMessage(13, message); };
};

}  // namespace fyber