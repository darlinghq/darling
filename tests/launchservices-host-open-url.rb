# Host test: the URL check in front of the host desktop opener.
# usage: ruby tests/launchservices-host-open-url.rb [REPO_ROOT]
# LSHostOpenURL.c has no Darling dependency,
# so the validator is built unmodified with ASan and UBSan.
require 'tmpdir'
require 'open3'
root = ARGV.fetch(0, File.expand_path('..', __dir__))
ls = "#{root}/src/frameworks/CoreServices/src/LaunchServices"
Dir.mktmpdir('ls-url-') do |dir|
  File.write("#{dir}/probe.c", <<~'C')
    #include <assert.h>
    #include <stdio.h>
    #include <string.h>
    #include "LSHostOpenURL.h"
    static void ok(const char *u)  { if (_LSHostOpenValidateURL(u) != 0)  { printf("should accept: %s\n", u); assert(0); } }
    static void bad(const char *u) { if (_LSHostOpenValidateURL(u) == 0) { printf("should reject: %s\n", u); assert(0); } }
    int main(void) {
      ok("https://example.org/");
      ok("HTTP://example.org:8080/a/b?x=1&y=%20z#frag");
      ok("https://user@example.org/p;q=(r)*+,!~-._[x]");
      ok("mailto:someone@example.org");
      ok("mailto:someone@example.org?subject=Hi%20there&body=line%0Aline");
      ok("mailto:a@example.org?attacher=1");      /* a different parameter name */
      /* scheme allowlist */
      bad(""); bad("file:///etc/passwd"); bad("javascript:alert(1)"); bad("httpx://e.org"); bad(" https://e.org");
      bad("ftp://e.org/"); bad("file://localhost/etc/passwd"); bad("-https://e.org"); bad("--new-window"); bad("http:foo"); bad("https:"); bad("http:///x");
      /* characters that are never legal in a URL */
      bad("https://e.org/a b"); bad("https://e.org/a b --new-window"); bad("https://e.org/a\tb");
      bad("https://e.org/a\nb"); bad("https://e.org/a\rb"); bad("https://e.org/\x01"); bad("https://e.org/\x7f");
      bad("https://e.org/\""); bad("https://e.org/<x>"); bad("https://e.org/\\x"); bad("https://e.org/^"); bad("https://e.org/`x`");
      /* xdg-open is a shell script: no '$' and no single quote */
      bad("https://e.org/$(id)"); bad("https://e.org/a$b"); bad("https://e.org/'x'"); bad("mailto:a@e.org?subject=it's");
      ok("https://e.org/%24%27");
      bad("https://e.org/{x}"); bad("https://e.org/|x"); bad("https://e.org/\xc3\xa9"); bad("https://e.org/\xff");
      bad("https://e.org/%"); bad("https://e.org/%4"); bad("https://e.org/%zz"); bad("mailto:a@e.org?subject=a b");
      /* mailto options that attach a host file */
      bad("mailto:x@e.org?attach=/home/u/.ssh/id_rsa"); bad("mailto:x@e.org?ATTACH=/etc/passwd");
      bad("mailto:x@e.org?subject=s&attach=/etc/passwd"); bad("mailto:x@e.org?subject=s;attachment=/etc/passwd");
      bad("mailto:x@e.org?%61ttach=/etc/passwd"); bad("mailto:x@e.org?attachment");
      /* length */
      char big[LS_HOST_OPEN_MAX_URL_LENGTH + 16];
      memset(big, 'a', sizeof big); memcpy(big, "https://e.org/", 14);
      big[LS_HOST_OPEN_MAX_URL_LENGTH] = 0; ok(big);
      big[LS_HOST_OPEN_MAX_URL_LENGTH + 1] = 0; big[LS_HOST_OPEN_MAX_URL_LENGTH] = 'a'; bad(big);
      bad(NULL);
      puts("PASS: URL validation");
      return 0;
    }
  C
  out, st = Open3.capture2e('clang', '-fsanitize=address,undefined', '-fno-sanitize-recover=undefined', '-Wall', "-I#{ls}",
    "#{dir}/probe.c", "#{ls}/LSHostOpenURL.c", '-o', "#{dir}/probe")
  abort out unless st.success?
  out, st = Open3.capture2e("#{dir}/probe", rlimit_core: 0)
  puts out
  abort 'host open URL validation regression' unless st.success?
end
