#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include "gtest/gtest.h"

namespace fyber::test {

namespace fs = std::filesystem;
using std::string;

inline const fs::path base_path = fs::current_path().parent_path().parent_path();

inline const fs::path resources = base_path / "tests" / "resources";

inline const fs::path bin_path = skad_updater_BIN;

inline const string mockserver_addr = "localhost:5000";

inline const string WelcomeToSkadMsg = string("*** Welcome to SKAd Updater ( version ") + skad_updater_VERSION + " )\n";

std::string exec(const string& command)
{
  const char* cmd = command.c_str();
  std::array<char, 128> buffer{};
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

std::string read_file(const fs::path& file)
{
  string content;
  string line;
  std::ifstream podfile(file);
  if (podfile.is_open()) {
    while (getline(podfile, line)) {
      content += line + "\n";
    }
  } else {
    throw std::invalid_argument("Unable to open " + file.string());
  }
  return content;
}

std::string ltrim(std::string s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  return s;
}

std::string rtrim(std::string s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
  return s;
}

bool starts_with(const string& str, const string& term)
{
  return str.rfind(term, 0) == 0;
}

bool log_starts_with(const string& str, const string& term)
{
  return starts_with(str, WelcomeToSkadMsg + term);
}

string set_mock_data(const string& data)
{
  auto cmd = "curl -X POST \"http://" + mockserver_addr + "/set_data\" -d '" + data +
             "' -H \"Content-Type: application/json\"";
  std::cout << "Modifing data with : " << cmd << std::endl;
  auto res = ltrim(rtrim(exec(cmd)));

  return res;
}

void with_mock_data(const string& data, const std::function<void(const string&)>& block)
{
  auto cmd = "curl -X GET \"http://" + mockserver_addr + "/get_data\"";
  auto original_data = ltrim(rtrim(exec(cmd)));
  set_mock_data(data);
  block(data);
  set_mock_data(original_data);
}

string run_skad_updater(const string& param)
{
  auto cmd = "export FYBER_SKAD_NETWORKS_SERVER_HOST=http://" + mockserver_addr + ";" +
             (bin_path / "skad_updater").string() + " " + param;
  std::cout << "Running: " << cmd << std::endl;
  return exec(cmd);
}

class End2End : public ::testing::Test
{
 protected:
  // Per-test-suite set-up.
  // Called before the first test in this test suite.
  // Can be omitted if not needed.
  static void SetUpTestSuite()
  {
    std::system((base_path / "tests" / "servermock" / "run_mock_server.sh &").c_str());
    for (int i = 0; i < 5; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      const string& check_server_status =
          rtrim(ltrim(exec("curl -LI http://" + mockserver_addr + "/networks -o /dev/null -w '%{http_code}\\n' -s")));

      std::cout << "Mock status = [" << check_server_status << "]" << std::endl;

      if (check_server_status == "200") {
        std::cout << "Mock Server Ready" << std::endl;
        return;
      }
    }
  }

  // Per-test-suite tear-down.
  // Called after the last test in this test suite.
  // Can be omitted if not needed.
  static void TearDownTestSuite()
  {
    auto shutdown = exec("curl -X GET \"" + mockserver_addr + "/shutdown\"");
    std::cout << shutdown << std::endl;
  }

  //  // You can define per-test set-up logic as usual.
  //  virtual void SetUp() { ... }
  //
  //  // You can define per-test tear-down logic as usual.
  //  virtual void TearDown() { ... }

  // Some expensive resource shared by all tests.
  //  static T* shared_resource_;
};

TEST_F(End2End, Help)
{
  auto result = run_skad_updater("--help");
  EXPECT_PRED2(log_starts_with, result, "Automatically update your SKAdNetwork Items");

  result = run_skad_updater("-h");
  EXPECT_PRED2(log_starts_with, result, "Automatically update your SKAdNetwork Items");

  result = run_skad_updater("--show_networks -h");
  EXPECT_PRED2(log_starts_with, result, "Automatically update your SKAdNetwork Items");

  result = run_skad_updater("--unknown_param 323 -fl -h --more_unknown_flag");
  EXPECT_PRED2(log_starts_with, result, "*** Option ‘unknown_param’ does not exist");

  result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() + " -h --network_list=Facebook");
  EXPECT_PRED2(log_starts_with, result, "Automatically update your SKAdNetwork Items");
}

TEST_F(End2End, ShowNetworks)
{
  auto result = run_skad_updater("--show_networks");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "Supported network names: AdColony,Google-Mobile-Ads-SDK,ChartboostSDK,Applovin,Unknown_network")
                   .c_str());

  result = run_skad_updater("--show_networks --plist_file_path " + (resources / "Info.plist").string() +
                            " --network_list=Facebook");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "Supported network names: AdColony,Google-Mobile-Ads-SDK,ChartboostSDK,Applovin,Unknown_network")
                   .c_str());
}

TEST_F(End2End, NetworkListUnchanged)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                 " --network_list=Facebook --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
                "*** Fetching SKAdNetworks for: Facebook\n"
                "*** New SKAdNetworks: \n"
                "*** Nothing to update. `" +
                resources.string() + "/Info.plist` unchanged.\n")
                   .c_str());
}

TEST_F(End2End, NetworkListNewNetworks)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                 " --network_list=Applovin --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
                "*** Fetching SKAdNetworks for: Applovin\n"
                "*** New SKAdNetworks: ludvb6z3bs.skadnetwork\n"
                "*** Updating `" +
                resources.string() +
                "/Info.plist`\n"
                "*** These network IDs will be added: ludvb6z3bs.skadnetwork\n")
                   .c_str());
}

TEST_F(End2End, PodFileNewNetworks)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
                "*** Fetching SKAdNetworks for: AdColony, ChartboostSDK, Google-Mobile-Ads-SDK\n"
                "*** New SKAdNetworks: blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n"
                "*** Updating `" +
                resources.string() +
                "/Info.plist`\n"
                "*** These network IDs will be added: blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n")
                   .c_str());
}

TEST_F(End2End, PodFileNewNetworksWithExplicitNetworkList)
{
  with_mock_data(
      R"({"AdColony": ["4PFYVQ9L8R.skadnetwork", "YCLNXRL5PM.skadnetwork"],)"
      R"("Google-Mobile-Ads-SDK": ["cstr6suwn9.skadnetwork"],)"
      R"("ChartboostSDK": ["blskdfjl2e3.skadnetwork"],)"
      R"("Applovin": ["ludvb6z3bs.skadnetwork"],)"
      R"("Inmobi":["inmobi.skad.id","inmobi.more.skad.id"],)"
      R"("AnotherNetwork":["other.id"],)"
      R"("Facebook": ["facebook.stuff"]})",
      [](auto data) {
        auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                       " --pod_file_path=" + (resources / "Podfile").string() +
                                       " --network_list=Inmobi,Facebook"
                                       " --dry_run");
        ASSERT_STREQ(
            result.c_str(),
            (WelcomeToSkadMsg +
             "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
             "*** Fetching SKAdNetworks for: AdColony, ChartboostSDK, Google-Mobile-Ads-SDK, Inmobi, Facebook\n"
             "*** New SKAdNetworks: blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork, facebook.stuff, inmobi.more.skad.id, inmobi.skad.id\n"
             "*** Updating `" +
             resources.string() +
             "/Info.plist`\n"
             "*** These network IDs will be added: blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork, facebook.stuff, inmobi.more.skad.id, inmobi.skad.id\n")
                .c_str());
      });
}

TEST_F(End2End, PodFileNoNetworks)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                 " --pod_file_path=" + (resources / "NoNetworksPodfile").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
                "*** No supported networks found in your Podfile\n")
                   .c_str());
}

TEST_F(End2End, PodFileNewIDsOverridingExistingNetworksAreJustAdded)
{

  with_mock_data(
      R"({"AdColony":["blskdfjl2e3.skadnetwork"],"AppLovinSDK":["cstr6suwn9.skadnetwork"]})", [](const string& data) {
        auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                       " --pod_file_path=" + (resources / "Podfile").string() + " --dry_run");
        ASSERT_STREQ(
            result.c_str(),
            (WelcomeToSkadMsg +
             "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
             "*** Fetching SKAdNetworks for: AdColony, AppLovinSDK\n"
             "*** New SKAdNetworks: blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n"
             "*** Updating `" +
             resources.string() +
             "/Info.plist`\n"
             "*** These network IDs will be added: blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n")
                .c_str());
      });
}

TEST_F(End2End, PlistIsEmpty)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "empty.Info.plist").string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: \n"
                "*** Fetching SKAdNetworks for: AdColony, ChartboostSDK, Google-Mobile-Ads-SDK\n"
                "*** New SKAdNetworks: 4PFYVQ9L8R.skadnetwork, YCLNXRL5PM.skadnetwork, blskdfjl2e3.skadnetwork, "
                "cstr6suwn9.skadnetwork\n"
                "*** Updating `" +
                resources.string() +
                "/empty.Info.plist`\n"
                "*** These network IDs will be added: 4PFYVQ9L8R.skadnetwork, YCLNXRL5PM.skadnetwork, "
                "blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n")
                   .c_str());
}

TEST_F(End2End, PlistIsFull)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "full.Info.plist").string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: 22mmun2rn5.skadnetwork, 238da6jt44.skadnetwork, 23zd986j2c.skadnetwork, "
                "24t9a8vw3c.skadnetwork, 252b5q8x7y.skadnetwork, 2U9PT9HC89.skadnetwork, 3RD42EKR43.skadnetwork, "
                "3qy4746246.skadnetwork, 3sh42y64q3.skadnetwork, 424M5254LK.skadnetwork, 4468KM3ULZ.skadnetwork, "
                "4468km3ulz.skadnetwork, 44JX6755AQ.skadnetwork, 44jx6755aq.skadnetwork, 44n7hlldy6.skadnetwork, "
                "488r3q3dtq.skadnetwork, 4DZT52R2T5.skadnetwork, 4FZDC2EVR5.skadnetwork, 4PFYVQ9L8R.skadnetwork, "
                "5LM9LJ6JB7.skadnetwork, 5a6flpkh64.skadnetwork, 5l3tpt7t6e.skadnetwork, 5lm9lj6jb7.skadnetwork, "
                "6xzpu9s2p8.skadnetwork, 7RZ58N8NTL.skadnetwork, 7UG5ZH24HU.skadnetwork, 7rz58n8ntl.skadnetwork, "
                "8S468MFL3Y.skadnetwork, 8s468mfl3y.skadnetwork, 9G2AGGBJ52.skadnetwork, 9RD848Q2BZ.skadnetwork, "
                "9T245VHMPL.skadnetwork, C6K4G5QG8M.skadnetwork, DZG6XY7PWJ.skadnetwork, ECPZ2SRF59.skadnetwork, "
                "EJVT5QM6AK.skadnetwork, F38H382JLK.skadnetwork, GLQZH8VGBY.skadnetwork, GTA9LK7P23.skadnetwork, "
                "HS6BDUKANM.skadnetwork, KBD757YWX3.skadnetwork, KLF5C3L5U5.skadnetwork, M8DBW4SV7C.skadnetwork, "
                "MLMMFZH3R3.skadnetwork, MTKV5XTK9E.skadnetwork, PPXM28T8AP.skadnetwork, PRCB7NJMU6.skadnetwork, "
                "T38B2KH725.skadnetwork, TL55SBB4FM.skadnetwork, V72QYCH5UU.skadnetwork, W9Q455WK68.skadnetwork, "
                "WZMMZ9FP6W.skadnetwork, YCLNXRL5PM.skadnetwork, YDX93A7ASS.skadnetwork, av6w8kgt66.skadnetwork, "
                "bvpn9ufa9b.skadnetwork, c6k4g5qg8m.skadnetwork, cstr6suwn9.skadnetwork, ejvt5qm6ak.skadnetwork, "
                "f73kdq92p3.skadnetwork, hdw39hrw9y.skadnetwork, hs6bdukanm.skadnetwork, lr83yxwka7.skadnetwork, "
                "ludvb6z3bs.skadnetwork, mlmmfzh3r3.skadnetwork, ppxm28t8ap.skadnetwork, prcb7njmu6.skadnetwork, "
                "t38b2kh725.skadnetwork, uw77j35x4d.skadnetwork, v79kvwwj4g.skadnetwork, wg4vff78zm.skadnetwork, "
                "y45688jllp.skadnetwork, ydx93a7ass.skadnetwork, zmvfpc5aq8.skadnetwork\n"
                "*** Fetching SKAdNetworks for: AdColony, ChartboostSDK, Google-Mobile-Ads-SDK\n"
                "*** New SKAdNetworks: blskdfjl2e3.skadnetwork\n"
                "*** Updating `" +
                resources.string() +
                "/full.Info.plist`\n"
                "*** These network IDs will be added: blskdfjl2e3.skadnetwork\n")
                   .c_str());
}

TEST_F(End2End, PlistHasNoSKAdNetworksKey)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "nosk.Info.plist").string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(), (WelcomeToSkadMsg +
                                "*** Existing SKAdNetworks: \n"
                                "*** Fetching SKAdNetworks for: AdColony, ChartboostSDK, Google-Mobile-Ads-SDK\n"
                                "*** New SKAdNetworks: 4PFYVQ9L8R.skadnetwork, YCLNXRL5PM.skadnetwork, "
                                "blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n"
                                "*** Updating `" +
                                resources.string() +
                                "/nosk.Info.plist`\n"
                                "*** These network IDs will be added: 4PFYVQ9L8R.skadnetwork, YCLNXRL5PM.skadnetwork, "
                                "blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n")
                                   .c_str());
}

TEST_F(End2End, PlistIsSimple)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "simple.Info.plist").string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(), (WelcomeToSkadMsg +
                                "*** Existing SKAdNetworks: 4PFYVQ9L8R, YCLNXRL5PM, cstr6suwn9\n"
                                "*** Fetching SKAdNetworks for: AdColony, ChartboostSDK, Google-Mobile-Ads-SDK\n"
                                "*** New SKAdNetworks: 4PFYVQ9L8R.skadnetwork, YCLNXRL5PM.skadnetwork, "
                                "blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n"
                                "*** Updating `" +
                                resources.string() +
                                "/simple.Info.plist`\n"
                                "*** These network IDs will be added: 4PFYVQ9L8R.skadnetwork, YCLNXRL5PM.skadnetwork, "
                                "blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n")
                                   .c_str());
}

TEST_F(End2End, InvalidPathPList)
{
  auto result = run_skad_updater("--plist_file_path " + resources.string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(), (WelcomeToSkadMsg + "*** Provided plist_file_path is invalid : -directory-\n").c_str());
}

TEST_F(End2End, InvalidPathPodfile)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                 " --pod_file_path=" + resources.string() + " --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
                "*** Provided pod_file_path is invalid : -directory-\n")
                   .c_str());
}

TEST_F(End2End, PathPodfileNotExits)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                 " --pod_file_path=" + (resources / "ImNotExisting").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
                "*** Provided pod_file_path is invalid : -does not exist-\n")
                   .c_str());
}

TEST_F(End2End, PListNotExists)
{
  auto result = run_skad_updater("--plist_file_path " + (resources / "NotExisting.Info.plist").string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + " --dry_run");
  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg + "*** Provided plist_file_path is invalid : -does not exist-\n").c_str());
}
TEST_F(End2End, NoDryRun)
{
  string plist_old = read_file(resources / "Info.plist");

  ASSERT_NE(plist_old.find("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"), string::npos);
  ASSERT_NE(
      plist_old.find(
          "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"),
      string::npos);

  ASSERT_NE(plist_old.find("4PFYVQ9L8R.skadnetwork"), string::npos);   // found
  ASSERT_NE(plist_old.find("YCLNXRL5PM.skadnetwork"), string::npos);   // found
  ASSERT_NE(plist_old.find("V72QYCH5UU.skadnetwork"), string::npos);   // found
  ASSERT_EQ(plist_old.find("blskdfjl2e3.skadnetwork"), string::npos);  // added
  ASSERT_EQ(plist_old.find("cstr6suwn9.skadnetwork"), string::npos);   // added

  auto result = run_skad_updater("--plist_file_path " + (resources / "Info.plist").string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + "");

  string plist_new = read_file(resources / "Info.plist");
  string plist_bak = read_file(resources / "Info.plist.bak.1");
  fs::rename(resources / "Info.plist.bak.1", resources / "Info.plist");

  ASSERT_NE(plist_new.find("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"), string::npos);  // unchanged
  ASSERT_NE(
      plist_new.find(
          "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"),
      string::npos);  // unchanged

  ASSERT_NE(plist_new.find("4PFYVQ9L8R.skadnetwork"), string::npos);   // kept
  ASSERT_NE(plist_new.find("YCLNXRL5PM.skadnetwork"), string::npos);   // kept
  ASSERT_NE(plist_old.find("V72QYCH5UU.skadnetwork"), string::npos);   // kept
  ASSERT_NE(plist_new.find("blskdfjl2e3.skadnetwork"), string::npos);  // added
  ASSERT_NE(plist_new.find("cstr6suwn9.skadnetwork"), string::npos);   // added

  ASSERT_STREQ(plist_old.c_str(), plist_bak.c_str());

  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: 4PFYVQ9L8R.skadnetwork, V72QYCH5UU.skadnetwork, YCLNXRL5PM.skadnetwork\n"
                "*** Fetching SKAdNetworks for: AdColony, ChartboostSDK, Google-Mobile-Ads-SDK\n"
                "*** New SKAdNetworks: blskdfjl2e3.skadnetwork, cstr6suwn9.skadnetwork\n"
                "*** Updating `" +
                resources.string() +
                "/Info.plist`\n"
                "*** Backup `" +
                resources.string() + "/Info.plist` created at `" + resources.string() +
                "/Info.plist.bak.1`\n"
                "*** Saving new `" +
                resources.string() + "/Info.plist` = true\n")
                   .c_str());
}

TEST_F(End2End, NoDryRunEmptyPlist)
{
  string plist_old = read_file(resources / "empty.Info.plist");

  ASSERT_NE(plist_old.find("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"), string::npos);
  ASSERT_NE(
      plist_old.find(
          "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"),
      string::npos);

  ASSERT_EQ(plist_old.find("4PFYVQ9L8R.skadnetwork"), string::npos);   // not found
  ASSERT_EQ(plist_old.find("YCLNXRL5PM.skadnetwork"), string::npos);   // not found
  ASSERT_EQ(plist_old.find("blskdfjl2e3.skadnetwork"), string::npos);  // not found
  ASSERT_EQ(plist_old.find("cstr6suwn9.skadnetwork"), string::npos);   // not found

  auto result = run_skad_updater("--plist_file_path " + (resources / "empty.Info.plist").string() +
                                 " --pod_file_path=" + (resources / "Podfile").string() + "");

  string plist_new = read_file(resources / "empty.Info.plist");
  string plist_bak = read_file(resources / "empty.Info.plist.bak.1");
  fs::rename(resources / "empty.Info.plist.bak.1", resources / "empty.Info.plist");

  ASSERT_NE(plist_new.find("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"), string::npos);  // unchanged
  ASSERT_NE(
      plist_new.find(
          "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"),
      string::npos);  // unchanged

  ASSERT_NE(plist_new.find("4PFYVQ9L8R.skadnetwork"), string::npos);   // added
  ASSERT_NE(plist_new.find("YCLNXRL5PM.skadnetwork"), string::npos);   // added
  ASSERT_NE(plist_new.find("blskdfjl2e3.skadnetwork"), string::npos);  // added
  ASSERT_NE(plist_new.find("cstr6suwn9.skadnetwork"), string::npos);   // added

  ASSERT_STREQ(plist_old.c_str(), plist_bak.c_str());

  ASSERT_STREQ(result.c_str(),
               (WelcomeToSkadMsg +
                "*** Existing SKAdNetworks: \n"
                "*** Fetching SKAdNetworks for: AdColony, ChartboostSDK, Google-Mobile-Ads-SDK\n"
                "*** New SKAdNetworks: 4PFYVQ9L8R.skadnetwork, YCLNXRL5PM.skadnetwork, blskdfjl2e3.skadnetwork, "
                "cstr6suwn9.skadnetwork\n"
                "*** Updating `" +
                resources.string() +
                "/empty.Info.plist`\n"
                "*** Backup `" +
                resources.string() +
                "/empty.Info.plist` created at "
                "`" +
                resources.string() +
                "/empty.Info.plist.bak.1`\n"
                "*** Saving new `" +
                resources.string() + "/empty.Info.plist` = true\n")
                   .c_str());
}

TEST_F(End2End, NoDryRunMultiBackup)
{
  with_mock_data(R"({"AdColony":["one_1"],"Facebook":["two_2"],"Admob":["three_3","four_4","five_5"]})", [](auto data) {
    string plist_old = read_file(resources / "empty.Info.plist");

    ASSERT_EQ(plist_old.find("one_1"), string::npos);    // not found
    ASSERT_EQ(plist_old.find("two_2"), string::npos);    // not found
    ASSERT_EQ(plist_old.find("three_3"), string::npos);  // not found
    ASSERT_EQ(plist_old.find("four_4"), string::npos);   // not found
    ASSERT_EQ(plist_old.find("five_5"), string::npos);   // not found

    auto result1 =
        run_skad_updater("--plist_file_path " + (resources / "empty.Info.plist").string() + " --network_list=AdColony");
    string plist_new1 = read_file(resources / "empty.Info.plist");
    string plist_bak1 = read_file(resources / "empty.Info.plist.bak.1");

    auto result2 =
        run_skad_updater("--plist_file_path " + (resources / "empty.Info.plist").string() + " --network_list=Admob");
    string plist_new2 = read_file(resources / "empty.Info.plist");
    string plist_bak2 = read_file(resources / "empty.Info.plist.bak.2");

    auto result3 =
        run_skad_updater("--plist_file_path " + (resources / "empty.Info.plist").string() + " --network_list=Facebook");
    string plist_new3 = read_file(resources / "empty.Info.plist");
    string plist_bak3 = read_file(resources / "empty.Info.plist.bak.3");

    fs::rename(resources / "empty.Info.plist.bak.1", resources / "empty.Info.plist");
    fs::remove(resources / "empty.Info.plist.bak.2");
    fs::remove(resources / "empty.Info.plist.bak.3");

    ASSERT_STREQ(plist_old.c_str(), plist_bak1.c_str());   // original backed up to bak.1
    ASSERT_STREQ(plist_new1.c_str(), plist_bak2.c_str());  // second run backed up to bak.2
    ASSERT_STREQ(plist_new2.c_str(), plist_bak3.c_str());  // third run backed up to bak.3

    ASSERT_NE(plist_new1.find("one_1"), string::npos);    // added
    ASSERT_EQ(plist_new1.find("two_2"), string::npos);    // not found
    ASSERT_EQ(plist_new1.find("three_3"), string::npos);  // not found
    ASSERT_EQ(plist_new1.find("four_4"), string::npos);   // not found
    ASSERT_EQ(plist_new1.find("five_5"), string::npos);   // not found

    ASSERT_NE(plist_new2.find("one_1"), string::npos);    // kept
    ASSERT_EQ(plist_new2.find("two_2"), string::npos);    // not found
    ASSERT_NE(plist_new2.find("three_3"), string::npos);  // added
    ASSERT_NE(plist_new2.find("four_4"), string::npos);   // added
    ASSERT_NE(plist_new2.find("five_5"), string::npos);   // added

    ASSERT_NE(plist_new3.find("one_1"), string::npos);    // kept
    ASSERT_NE(plist_new3.find("two_2"), string::npos);    // added
    ASSERT_NE(plist_new3.find("three_3"), string::npos);  // kept
    ASSERT_NE(plist_new3.find("four_4"), string::npos);   // kept
    ASSERT_NE(plist_new3.find("five_5"), string::npos);   // kept
  });
}

}  // namespace fyber::test

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
