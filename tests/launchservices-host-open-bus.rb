# Host test: the transport of the host desktop opener, against a stub sd-bus.
# usage: ruby tests/launchservices-host-open-bus.rb [REPO_ROOT]
# LSHostOpenBus.c and LSHostOpenURL.c are built unmodified with ASan and UBSan; every host call comes from
# a function table, so the stub records the bus traffic and the test checks it.
require 'tmpdir'
require 'open3'
root = ARGV.fetch(0, File.expand_path('..', __dir__))
ls = "#{root}/src/frameworks/CoreServices/src/LaunchServices"
Dir.mktmpdir('ls-bus-') do |dir|
  File.write("#{dir}/probe.c", <<~'C')
    #include <assert.h>
    #include <errno.h>
    #include <stdarg.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "LSHostOpenBus.h"

    static int bus_new, bus_closed, msg_new, msg_unref, portal_calls, systemd_calls, started;
    static int portal_result, systemd_result;
    static const char *portal_error_name;
    static char address_seen[1024], opener_seen[256], url_seen[8192], portal_url_seen[8192];
    static const char *env_address, *env_display;
    static int cur_is_portal;

    static int s_new(void **r) { bus_new++; *r = malloc(1); return 0; }
    static int s_set_address(void *b, const char *a) { snprintf(address_seen, sizeof address_seen, "%s", a); return 0; }
    static int s_client(void *b, int x) { return 0; }
    static int s_start(void *b) { started++; return 0; }
    static void *s_close(void *b) { bus_closed++; free(b); return NULL; }
    static int s_msg(void *b, void **m, const char *d, const char *p, const char *i, const char *member) {
      msg_new++; cur_is_portal = strcmp(d, "org.freedesktop.portal.Desktop") == 0; *m = malloc(1); return 0;
    }
    static int s_append(void *m, const char *types, ...) {
      va_list ap; va_start(ap, types);
      if (strcmp(types, "ss") == 0) {
        const char *a = va_arg(ap, const char *), *b = va_arg(ap, const char *);
        if (cur_is_portal) snprintf(portal_url_seen, sizeof portal_url_seen, "%s", b);
        (void) a;
      } else if (strcmp(types, "(sv)(sv)(sv)(sv)") == 0) {
        for (int i = 0; i < 3; i++) { va_arg(ap, const char *); va_arg(ap, const char *); va_arg(ap, const char *); }
        va_arg(ap, const char *); va_arg(ap, const char *); (void) va_arg(ap, int);
        const char *opener = va_arg(ap, const char *); (void) va_arg(ap, int);
        va_arg(ap, const char *); const char *url = va_arg(ap, const char *);
        snprintf(opener_seen, sizeof opener_seen, "%s", opener); snprintf(url_seen, sizeof url_seen, "%s", url);
      }
      va_end(ap); return 0;
    }
    static int s_strv(void *m, char **l) { return 0; }
    static int s_open(void *m, char t, const char *c) { return 0; }
    static int s_closec(void *m) { return 0; }
    static int s_call(void *b, void *m, uint64_t u, struct ls_host_bus_error *e, void **reply) {
      *reply = NULL;
      if (cur_is_portal) { portal_calls++; e->name = portal_result < 0 ? portal_error_name : NULL; return portal_result; }
      systemd_calls++; return systemd_result;
    }
    static void *s_unref(void *m) { msg_unref++; free(m); return NULL; }
    static void s_efree(struct ls_host_bus_error *e) { e->name = NULL; }
    static char *s_getenv(const char *n) {
      if (strcmp(n, "DBUS_SESSION_BUS_ADDRESS") == 0) return (char *) env_address;
      if (strcmp(n, "DISPLAY") == 0) return (char *) env_display;
      return NULL;
    }
    static unsigned s_uid(void) { return 1000; }
    static int s_access(const char *p, int m) { return strcmp(p, "/usr/bin/xdg-open") == 0 ? 0 : -1; }

    static struct ls_host_api api = { s_new, s_set_address, s_client, s_start, s_close, s_msg, s_append, s_strv,
      s_open, s_closec, s_call, s_unref, s_efree, s_getenv, s_uid, s_access, "/run/user", 0 };

    static void reset(void) {
      bus_new = bus_closed = msg_new = msg_unref = portal_calls = systemd_calls = started = 0;
      portal_result = systemd_result = 0; portal_error_name = NULL; env_address = NULL; env_display = ":0";
      address_seen[0] = opener_seen[0] = url_seen[0] = portal_url_seen[0] = 0;
    }
    static void balanced(void) { assert(bus_new == bus_closed); assert(msg_new == msg_unref); }

    int main(void) {
      int method = 0, r;
      /* portal success: one call, no systemd, default address from the uid */
      reset();
      r = _LSHostOpenURL(&api, "https://example.org/", &method);
      assert(r == 0 && method == LS_HOST_OPEN_VIA_PORTAL && portal_calls == 1 && systemd_calls == 0);
      assert(strcmp(address_seen, "unix:path=/run/user/1000/bus") == 0);
      assert(strcmp(portal_url_seen, "https://example.org/") == 0);
      balanced();
      /* portal not running: fall back to systemd with the found xdg-open and the URL */
      reset(); portal_result = -113; portal_error_name = "org.freedesktop.DBus.Error.ServiceUnknown";
      r = _LSHostOpenURL(&api, "https://example.org/", &method);
      assert(r == 0 && method == LS_HOST_OPEN_VIA_SYSTEMD && portal_calls == 1 && systemd_calls == 1);
      assert(strcmp(opener_seen, "/usr/bin/xdg-open") == 0 && strcmp(url_seen, "https://example.org/") == 0);
      balanced();
      reset(); portal_result = -6; portal_error_name = "org.freedesktop.DBus.Error.NameHasNoOwner";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == 0 && systemd_calls == 1);
      balanced();
      /* a timeout may have opened the URL: no second attempt */
      reset(); portal_result = -LS_HOST_LINUX_ETIMEDOUT; portal_error_name = "org.freedesktop.DBus.Error.NoReply";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_ETIMEDOUT && systemd_calls == 0);
      balanced();
      /* an invalid URL never touches the bus */
      reset();
      assert(_LSHostOpenURL(&api, "https://e.org/a b --new-window", &method) < 0 && bus_new == 0);
      assert(_LSHostOpenURL(&api, "file:///etc/passwd", &method) < 0 && bus_new == 0);
      /* an incomplete table gives ENOSYS and no bus */
      reset();
      { struct ls_host_api bad = api; bad.sd_bus_call = NULL;
        assert(_LSHostOpenURL(&bad, "https://example.org/", &method) == -LS_HOST_LINUX_ENOSYS && bus_new == 0); }
      /* the session address from the environment: a unix socket is used as given */
      reset(); env_address = "unix:path=/run/user/1000/bus,guid=abc";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == 0);
      assert(strcmp(address_seen, env_address) == 0);
      balanced();
      /* an abstract socket is a plain address too */
      reset(); env_address = "unix:abstract=/tmp/dbus-x";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == 0 && strcmp(address_seen, env_address) == 0);
      /* sd-bus takes ';' as an alternative address and "unixexec:" runs a host program */
      reset(); env_address = "unix:path=/nonexistent;unixexec:path=/bin/sh,argv1=-c,argv2=id";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "unix:path=/run/user/1000/bus;unix:path=/tmp/other";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "unix:path=/x\nunixexec:path=/bin/sh";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "unix:path=/x unixexec:path=/bin/sh";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "unix:path=/x --new-window";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "unix:path=/x,unixexec:path=/bin/sh";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "unix:path=/x\x01y";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "unix:path=/x\xc3\xa9";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "unix:guid=abc,path=/x";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      /* anything else is refused before a bus is created ("unixexec:" would run a host program) */
      reset(); env_address = "unixexec:path=/bin/sh,argv1=-c,argv2=id";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      reset(); env_address = "tcp:host=127.0.0.1,port=1";
      assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0);
      /* an overlong address is refused, not copied */
      { static char big[2048]; memset(big, 'a', sizeof big - 1); memcpy(big, "unix:", 5);
        reset(); env_address = big;
        assert(_LSHostOpenURL(&api, "https://example.org/", &method) == -LS_HOST_LINUX_EINVAL && bus_new == 0); }
      puts("PASS: host open transport");
      return 0;
    }
  C
  out, st = Open3.capture2e('clang', '-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=undefined', '-w',
    "-I#{ls}", "#{dir}/probe.c", "#{ls}/LSHostOpenBus.c", "#{ls}/LSHostOpenURL.c", '-o', "#{dir}/probe")
  abort out unless st.success?
  out, st = Open3.capture2e("#{dir}/probe", rlimit_core: 0)
  puts out
  abort 'host open transport regression' unless st.success?
end
