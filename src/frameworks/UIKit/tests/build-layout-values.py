"""Build the UIKit value regression using pinned Darling headers and a runtime image."""
import argparse
import json
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--source-root', required=True, type=Path)
parser.add_argument('--runtime-root', required=True, type=Path)
parser.add_argument('--build-dir', required=True, type=Path)
parser.add_argument('--linker', required=True, type=Path)
args = parser.parse_args()
source = args.source_root.resolve()
runtime = args.runtime_root.resolve()
output = args.build_dir.resolve()
output.mkdir(parents=True, exist_ok=True)
component = source / 'src/frameworks/UIKit'
resource = subprocess.check_output(['clang', '-print-resource-dir'], text=True).strip()
compile_flags = ['clang', '-target', 'aarch64-apple-darwin20', '-nostdinc',
                 '-isystem', resource + '/include', '-D__APPLE__', '-D__MACH__',
                 '-D_DARWIN_C_SOURCE', '-DTARGET_OS_MAC=1', '-DDARWIN', '-DDARLING',
                 '-D_LIBC_NO_FEATURE_VERIFICATION', '-fblocks', '-fobjc-arc',
                 '-Wno-nullability-completeness']
for directory in ('basic-headers',
                  'Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include',
                  'framework-include', 'src/external/foundation/include',
                  'src/external/corefoundation/include',
                  'src/external/cocotron/CoreGraphics/include', 'src/frameworks/UIKit/include'):
    compile_flags += ['-I' + str(source / directory)]
linker = args.linker.resolve()
if not linker.is_file():
    raise SystemExit('a Darwin linker supporting -dylib_file is required')
link_flags = ['clang', '-target', 'aarch64-apple-darwin20', '-nostdlib',
              '-fuse-ld=' + str(linker), '-Wl,-platform_version,macos,11.0,11.0']
for path in sorted(runtime.rglob('*')):
    if path.is_file() and (path.suffix == '.dylib' or path.name + '.framework' in path.parts):
        guest = '/' + str(path.relative_to(runtime))
        link_flags += ['-Wl,-dylib_file,' + guest + ':' + str(path)]
libraries = [runtime / 'System/Library/Frameworks/Foundation.framework/Versions/C/Foundation',
             runtime / 'usr/lib/libobjc.A.dylib', runtime / 'usr/lib/libSystem.B.dylib']
commands = []

def run(command):
    command = [str(value) for value in command]
    commands.append(command)
    (output / 'commands.json').write_text(json.dumps(commands, indent=2) + '\n')
    result = subprocess.run(command, capture_output=True, text=True)
    with (output / 'build.log').open('a') as log:
        log.write(result.stdout + result.stderr)
    if result.returncode:
        print(result.stderr[-6000:])
        raise SystemExit('command failed; see build.log and commands.json')

run(compile_flags + ['-MD', '-MF', output / 'test.d', '-c', component / 'tests/layout-values.m', '-o', output / 'test.o'])
run(link_flags + ['-o', output / 'layout-values', output / 'test.o'] + libraries)
run(compile_flags + ['-MD', '-MF', output / 'layout.d', '-c', component / 'src/UICollectionViewCompositionalLayout.m',
                     '-o', output / 'layout.o'])
run(link_flags + ['-dynamiclib', '-install_name',
                  '/System/iOSSupport/System/Library/Frameworks/UIKit.framework/Versions/A/UIKit',
                  '-o', output / 'UIKit', output / 'layout.o'] + libraries)
