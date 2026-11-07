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
      /* encoded more than once, encoded delimiters, case */
      bad("mailto:x@e.org?%2561ttach=/etc/passwd"); bad("mailto:x@e.org?%2526attach=/etc/passwd");
      bad("mailto:x@e.org?attach%253d/etc/passwd"); bad("mailto:x@e.org?attach%3d/etc/passwd");
      bad("mailto:x@e.org?attach%3D/etc/passwd"); bad("mailto:x@e.org?%3Battach=/etc/passwd");
      bad("mailto:x@e.org?subject=s%26attach=/etc/passwd"); bad("mailto:x@e.org?subject=s%3Battachment=/etc/passwd");
      bad("mailto:x@e.org?AtTaCh=/etc/passwd"); bad("mailto:x@e.org?Attachment=/etc/passwd");
      bad("mailto:x@e.org?%2525252561ttach=/etc/passwd");
      bad("mailto:x@e.org?att%00ach=/etc/passwd");
      ok("mailto:x@e.org?subject=Hi%20there&cc=y@e.org&body=a%26b%20c");
      ok("mailto:x@e.org?subject=please%20attach%20it&body=attachment%20follows");
      /* a client may decode before it finds the '?': the query can start inside an escape */
      bad("mailto:x@e.org%3Fattach=/etc/passwd"); bad("mailto:x@e.org%3fattach=/etc/passwd");
      bad("mailto:%3Fattach=/etc/passwd"); bad("mailto:x@e.org%3Fattach=/x?subject=hi");
      bad("mailto:x@e.org%253Fattach=/etc/passwd"); bad("mailto:x%2540e.org%253Fattach=/x");
      bad("mailto:x@e.org%25253Fattach=/etc/passwd"); bad("mailto:x@e.org%3Fsubject=s%26attach=/x");
      bad("mailto:x@e.org%3F%2561ttach=/x"); bad("MAILTO:x@e.org%3Fattach=/x");
      /* nested encoding up to the bound, refused beyond it */
      bad("mailto:x@e.org?%2561ttach=/x"); bad("mailto:x@e.org?%252561ttach=/x");
      bad("mailto:x@e.org?%25252561ttach=/x"); bad("mailto:x@e.org?%2525252561ttach=/x");
      bad("mailto:x@e.org?%25252525252561ttach=/x"); bad("mailto:x@e.org?%25252525252525252525252561ttach=/x");
      /* names padded with whitespace or '+' */
      bad("mailto:x@e.org?%20attach=/x"); bad("mailto:x@e.org?attach%20=/x"); bad("mailto:x@e.org?%09attach=/x");
      bad("mailto:x@e.org?%0Aattach=/x"); bad("mailto:x@e.org?+attach=/x"); bad("mailto:x@e.org?attach+=/x");
      bad("mailto:x@e.org?++attachment++=/x"); bad("mailto:x@e.org?subject=s&%20ATTACH%20=/x");
      bad("mailto:x@e.org?+%20+attach=/x"); bad("mailto:x@e.org?%2520attach=/x");
      /* a second '?' starts a name for a client that splits on every '?' */
      bad("mailto:x@e.org?subject=x?attach=/x"); bad("mailto:x@e.org??attach=/x");
      /* the decoded text is searched, so a NUL anywhere is refused */
      bad("mailto:x@e.org%00?attach=/x"); bad("mailto:%00x@e.org"); bad("mailto:x@e.org?subject=a%2500b");
      /* mixed case and the separators */
      bad("mailto:x@e.org?AtTaChMeNt=/x"); bad("mailto:x@e.org?subject=s&;attach=/x");
      bad("mailto:x@e.org?body=b%3Battach=/x"); bad("mailto:x@e.org?body=b%26ATTACHMENT=/x");
      /* only names count: a value, an address or a longer or shorter name is fine */
      ok("mailto:x@e.org?subject=attach"); ok("mailto:x@e.org?body=attachment"); ok("mailto:x@e.org?subject=a&body=attach");
      ok("mailto:attach@e.org"); ok("mailto:attach@e.org?subject=hi"); ok("mailto:x@e.org?attac=1&attachx=2&xattach=3");
      ok("mailto:x@e.org?subject=attach%3Dthis"); ok("mailto:x@e.org?subject=a%20attach%20b");
      ok("mailto:x@e.org?subject=hi&attacher=1&attach_=2"); ok("mailto:x@e.org?attach-me=1");
      /* benign mail URLs: several addresses, escapes inside values, no query, a literal percent */
      ok("mailto:a@e.org,b@e.org"); ok("mailto:a@e.org,b@e.org?cc=c@e.org&bcc=d@e.org&subject=s");
      ok("mailto:x@e.org?subject=a%20b%26c%0Ad&body=1%2B1%3D2"); ok("mailto:x@e.org?body=Tom%20%26%20Jerry");
      ok("mailto:x%40e.org"); ok("mailto:?to=a@e.org&subject=s"); ok("mailto:x@e.org?subject=100%25");
      ok("mailto:x@e.org?subject=50%25%20off"); ok("mailto:x@e.org#frag"); ok("mailto:x@e.org?");
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
