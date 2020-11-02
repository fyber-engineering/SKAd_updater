class SkadUpdater < Formula
  desc "SKAdNetworks Updater"
  homepage "https://github.com/fyber-engineering/SKAd_updater/"
  url "https://github.com/fyber-engineering/SKAd_updater/releases/download/${PROJECT_VERSION}/${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz"
  sha256 "${SHA}"
  license "MIT"

  bottle :unneeded

  def install
    bin.install "bin/skad_updater"
    man1.install "docs/skad_updater.1"
    bash_completion.install "completions/completions.bash" => "skad_updater"
  end

  test do
    puts "Testing SKAD updater"
    help_out =  shell_output("skad_updater -h")
    puts "skad_updater -h" + help_out
    assert_true help_out.start_with?("*** Welcome to SKAd Updater")
    puts  shell_output("man skad_updater")
    assert_true shell_output("man skad_updater")
  end
end
