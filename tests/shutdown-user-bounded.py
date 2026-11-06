#!/usr/bin/env python3
"""
Host test: tools/shutdown-user.sh gives up instead of looping forever, and
only counts a container's launchd.
usage: python3 tests/shutdown-user-bounded.py [REPO_ROOT]
The script runs under /bin/sh with whoami, ps, pgrep, su, timeout and sleep
replaced by stubs that read a fake process table ("pid stat user args" per
line). su records each `darling shutdown` and, in the "stops" case, removes
the launchd from the table. Cases:
  stuck      a launchd that `darling shutdown` does not stop (another prefix):
             must exit non-zero after exactly five attempts
  unrelated  only commands that mention launchd (as an argument, or inside
             another first word) and a zombie launchd: must exit 0 without
             calling `darling shutdown`
  flap       a stuck launchd that returns under a new PID after every
             shutdown: must still exit 1 after five attempts
  mixed      the lowest-PID container is stuck, another stops: the other must
             still be shut down, then exit 1
Every su call must run under `timeout 15s`.
  stops      a launchd that `darling shutdown` stops: must exit 0 after one
             attempt
  many       seven users, each with a container that `darling shutdown` stops:
             must exit 0 after shutting down all seven (the attempt bound is
             per container, not global)
  uninstall  tools/uninstall with a shutdown-user.sh that fails: must exit
             non-zero before removing anything
"""

import os
import stat
import subprocess
import sys
import tempfile

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
script = os.path.join(root, "tools", "shutdown-user.sh")

STUBS = {
    "whoami": "echo root\n",
    "sleep": ":\n",
    "timeout": 'echo "$1" >> "$TIMEOUTS"\nshift\nexec "$@"\n',
    # ps -eo pid=,stat=,args=   |   ps -o uname=|args=|ppid= -p PID
    # (parents come from "$TABLE.parents", "child parent" per line)
    "ps": r'''
if [ "$1" = "-eo" ]; then
	while read -r pid st user args; do echo "$pid $st $args"; done < "$TABLE"
elif [ "$1" = "-o" ]; then
	case "$2" in
	uname=) while read -r pid st user args; do [ "$pid" = "$4" ] && echo "$user"; done < "$TABLE";;
	args=) while read -r pid st user args; do [ "$pid" = "$4" ] && echo "$args"; done < "$TABLE";;
	ppid=) [ -f "$TABLE.parents" ] && awk -v p="$4" '$1 == p { print $2 }' "$TABLE.parents";;
	esac
fi
exit 0
''',
    # pgrep -f PATTERN: every entry whose command line contains PATTERN
    "pgrep": r'''
while read -r pid st user args; do
	case "$args" in *"$2"*) echo "$pid";; esac
done < "$TABLE"
''',
    "su": r'''
echo "$*" >> "$CALLS"
if [ "$MODE" = stops ]; then grep -v ' /sbin/launchd$' "$TABLE" > "$TABLE.new"; mv "$TABLE.new" "$TABLE"; fi
if [ "$MODE" = many ]; then awk -v u="$2" '$3 != u' "$TABLE" > "$TABLE.new"; mv "$TABLE.new" "$TABLE"; fi
if [ "$MODE" = mixed ] && [ "$2" = bob ]; then awk -v u="$2" '$3 != u' "$TABLE" > "$TABLE.new"; mv "$TABLE.new" "$TABLE"; fi
if [ "$MODE" = prefix ] && [ "$4" = "DPREFIX='/srv/dp' darling shutdown" ]; then grep -v ' /sbin/launchd$' "$TABLE" > "$TABLE.new"; mv "$TABLE.new" "$TABLE"; fi
if [ "$MODE" = flap ]; then
	n=$(wc -l < "$CALLS")
	if [ "$n" -gt 50 ]; then : > "$TABLE"; else awk -v n="$n" '{ $1 = 9000 + n } 1' "$TABLE" > "$TABLE.new"; mv "$TABLE.new" "$TABLE"; fi
fi
exit 0
''',
}

CASES = [
    # name, table, expected exit is zero, expected number of shutdown calls,
    # users whose shutdown must have been called
    ("stuck", ["4242 Ssl alice /sbin/launchd"], False, 5, []),
    ("unrelated", ["100 S+ bob less src/launchd/src/core.c", "4243 Z alice /sbin/launchd",
                   "101 S root /usr/bin/not-launchd", "102 S root launchdctl list"], True, 0, []),
    ("stops", ["4244 Ssl alice /sbin/launchd"], True, 1, ["alice"]),
    ("many", [f"{5000 + i} Ssl user{i} /sbin/launchd" for i in range(7)], True, 7,
     [f"user{i}" for i in range(7)]),
    # the stuck launchd reappears under a new PID after every shutdown; the
    # stub empties the table after 50 calls so a regression cannot run away
    ("flap", ["4300 Ssl alice /sbin/launchd"], False, 5, []),
    # the lowest PID is stuck; the other container must still be shut down
    ("mixed", ["100 Ssl alice /sbin/launchd", "200 Ssl bob /sbin/launchd"], False, 6, ["bob"]),
    # a container in a non-default prefix: DPREFIX is taken from its darlingserver
    # parent, since su --login drops it; plain `darling shutdown` would not stop it
    ("prefix", ["4259 Ssl alice darlingserver /srv/dp 0 0 4 0", "4260 Ssl alice /sbin/launchd"], True, 1, ["alice"]),
    # a prefix that isn't a plain absolute path is not passed into su -c
    ("badprefix", ["4269 Ssl alice darlingserver /srv/x';id;' 0 0 4 0", "4270 Ssl alice /sbin/launchd"], False, 5, []),
]

# "child parent" pairs for the stub ps, and the su -c command each case must use
PARENTS = {"prefix": ["4260 4259"], "badprefix": ["4270 4269"]}
SU_COMMAND = {"prefix": "DPREFIX='/srv/dp' darling shutdown"}

failed = False
with tempfile.TemporaryDirectory(prefix="shutdown-user-") as d:
    bindir = os.path.join(d, "bin")
    os.mkdir(bindir)
    for name, body in STUBS.items():
        path = os.path.join(bindir, name)
        with open(path, "w", encoding="utf-8") as f:
            f.write("#!/bin/sh\n" + body)
        os.chmod(path, stat.S_IRWXU)

    for name, table, want_ok, want_calls, want_users in CASES:
        table_path = os.path.join(d, name + ".table")
        calls_path = os.path.join(d, name + ".calls")
        with open(table_path, "w", encoding="utf-8") as f:
            f.write("\n".join(table) + "\n")
        if name in PARENTS:
            with open(table_path + ".parents", "w", encoding="utf-8") as f:
                f.write("\n".join(PARENTS[name]) + "\n")
        su_command = SU_COMMAND.get(name, "darling shutdown")
        timeouts_path = os.path.join(d, name + ".timeouts")
        open(calls_path, "w").close()
        open(timeouts_path, "w").close()
        env = dict(os.environ, PATH=bindir + ":" + os.environ.get("PATH", "/usr/bin:/bin"),
                   TABLE=table_path, CALLS=calls_path, MODE=name, TIMEOUTS=timeouts_path)
        try:
            res = subprocess.run(["/bin/sh", script], env=env, capture_output=True, text=True, timeout=20)
        except subprocess.TimeoutExpired:
            with open(calls_path, encoding="utf-8") as f:
                n = sum(1 for _ in f)
            print(f"FAIL [{name}]: still looping after 20 s ({n} shutdown attempts)")
            failed = True
            continue
        with open(calls_path, encoding="utf-8") as f:
            calls = f.read().splitlines()
        with open(timeouts_path, encoding="utf-8") as f:
            timeouts = f.read().splitlines()
        problems = []
        if timeouts != ["15s"] * len(calls):
            problems.append(f"su run under timeouts {timeouts}, want 15s for each of {len(calls)} calls")
        if (res.returncode == 0) != want_ok:
            problems.append(f"exit status {res.returncode}")
        if want_calls is not None and len(calls) != want_calls:
            problems.append(f"{len(calls)} shutdown attempts, want {want_calls}")
        for u in want_users:
            if f"--login {u} -c {su_command}" not in calls:
                problems.append(f"no shutdown call for {u}")
        if calls and any(not c.startswith("--login ") or not c.endswith(" -c " + su_command) for c in calls):
            problems.append(f"unexpected su call {calls[0]!r}")
        if problems:
            print(f"FAIL [{name}]: " + "; ".join(problems))
            sys.stdout.write(res.stdout + res.stderr)
            failed = True
        else:
            print(f"PASS [{name}]: exit {res.returncode}, {len(calls)} shutdown attempts")

# tools/uninstall must not delete anything when shutdown-user.sh fails. It is
# copied next to a stub shutdown-user.sh that exits 1; darling, rm, rmdir and
# find are stubs that record what would have been removed.
uninstall = os.path.join(root, "tools", "uninstall")
with tempfile.TemporaryDirectory(prefix="uninstall-") as d:
    tools = os.path.join(d, "tools")
    bindir = os.path.join(d, "bin")
    os.mkdir(tools)
    os.mkdir(bindir)
    log = os.path.join(d, "removed")
    open(log, "w").close()
    stubs = {
        os.path.join(tools, "uninstall"): open(uninstall, encoding="utf-8").read(),
        os.path.join(tools, "shutdown-user.sh"): "#!/bin/sh\nexit 1\n",
        os.path.join(bindir, "whoami"): "#!/bin/sh\necho root\n",
        os.path.join(bindir, "darling"): "#!/bin/sh\nexit 0\n",
    }
    for cmd in ("rm", "rmdir", "find"):
        stubs[os.path.join(bindir, cmd)] = f'#!/bin/sh\necho "{cmd} $*" >> "{log}"\n'
    for path, body in stubs.items():
        with open(path, "w", encoding="utf-8") as f:
            f.write(body)
        os.chmod(path, stat.S_IRWXU)
    env = dict(os.environ, PATH=bindir + ":" + os.environ.get("PATH", "/usr/bin:/bin"))
    res = subprocess.run(["/bin/sh", os.path.join(tools, "uninstall")], env=env,
                         capture_output=True, text=True, timeout=20)
    removed = open(log, encoding="utf-8").read().splitlines()
    if res.returncode == 0 or removed:
        print(f"FAIL [uninstall]: exit {res.returncode} after a failed shutdown, {len(removed)} removals"
              + (f" ({removed[0]!r} ...)" if removed else ""))
        failed = True
    else:
        print(f"PASS [uninstall]: exit {res.returncode}, nothing removed after a failed shutdown")

if failed:
    print("FAIL: shutdown-user.sh regression")
    sys.exit(1)
print("PASS: shutdown-user.sh stops after bounded attempts and only counts a container's launchd")
