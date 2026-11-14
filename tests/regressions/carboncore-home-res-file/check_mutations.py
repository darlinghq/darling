# SPDX-License-Identifier: GPL-3.0-or-later
"""Check that incorrect ownership implementations fail the assertions."""
import os
import pathlib
import subprocess
import sys
import tempfile

suite = pathlib.Path(__file__).resolve().parent
root = suite.parents[2]
source = root / "src/frameworks/CoreServices/src/CarbonCore/Resources.cpp"
text = source.read_text()
start = text.index("ResFileRefNum HomeResFile(Handle resource)\n")
end = text.index("ResFileRefNum CurResFile(void)\n", start)
body = text[start:end]
mutations = {
    "current-file-instead-of-owner": ("return refNum;", "return g_currentResFile;"),
    "stale-error-on-success": ("g_lastError = noErr;", "/* leave previous error */"),
    "null-owner-map-entry": ("if (resource)", "if (true)"),
    "changes-current-file": ("return refNum;", "g_currentResFile = refNum; return refNum;"),
}
compiler = sys.argv[1] if len(sys.argv) > 1 else "g++"
with tempfile.TemporaryDirectory(prefix="darling-home-res-file-mutations.") as tmp:
    for name, (before, after) in mutations.items():
        if body.count(before) != 1:
            raise SystemExit("unsupported source shape for " + name)
        variant = pathlib.Path(tmp) / (name + ".cpp")
        variant.write_text(text[:start] + body.replace(before, after, 1) + text[end:])
        env = os.environ.copy()
        env["DARLING_HOME_RES_FILE_SOURCE"] = str(variant)
        result = subprocess.run(["bash", str(suite / "run.sh"), compiler],
                                cwd=root, env=env, text=True,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                timeout=90)
        if result.returncode == 0 or "FAIL line " not in result.stdout:
            print(result.stdout)
            raise SystemExit("FAIL: mutation not detected by an assertion: " + name)
        print("PASS: detected " + name)
