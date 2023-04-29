#!/usr/bin/env python3

import os
from typing import List, Dict
import subprocess
import re
import pathlib
from enum import Enum
import shutil
import datetime

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

#
# USING THIS SCRIPT:
#   * edit `DYLDINFO`, `OTOOL`, and `NM` to refer to the corresponding programs to analyze x86_64 Mach-Os.
#   * edit `XCODE_PATH` to point to the Xcode application.
#   * edit `SYSTEM_ROOT` to point to the root of a macOS or stock Darling installation.
#   * if desired, edit `OUT_DIR` to point to where the stub frameworks will be generated; the default is the best in most cases.
#

###
# EDIT THESE TO USE SCRIPT
###

DYLDINFO = 'x86_64-apple-darwin11-dyldinfo'
OTOOL = 'x86_64-apple-darwin11-otool'
NM = 'x86_64-apple-darwin11-nm'

XCODE_PATH = '/data/darling/Applications/Xcode.app'
SYSTEM_ROOT = '/usr/local/libexec/darling'
OUT_DIR = os.path.join(SCRIPT_DIR, '..', 'src', 'frameworks', 'dev-stubs')

###
# DON'T EDIT ANYTHING ELSE
###

#
# most of this was just trial-and-errored until i got something that worked.
# in particular, the RPATH computations are iffy, but they get the job done.
#

MACH_BINARY_MIME_TYPE = 'application/x-mach-binary'
XCODE_MAIN_BINARY = os.path.join(XCODE_PATH, 'Contents', 'MacOS', 'Xcode')

# extra RPATHs added to the RPATHs of plugins (not the main Xcode binary) and their dependencies.
EXTRA_PLUGIN_RPATHS = [
	os.path.join(XCODE_PATH, 'Contents', 'SharedFrameworks'),
	os.path.join(XCODE_PATH, 'Contents', 'OtherFrameworks'),
	os.path.join(XCODE_PATH, 'Contents', 'Frameworks'),
	os.path.join(XCODE_PATH, 'Contents', 'PlugIns'),
	os.path.join(XCODE_PATH, 'Contents', 'Developer', 'Platforms', 'MacOSX.platform', 'Developer', 'Library', 'Frameworks'),
	os.path.join(XCODE_PATH, 'Contents', 'Developer', 'Platforms', 'MacOSX.platform', 'Developer', 'usr', 'lib'),
	os.path.join(XCODE_PATH, 'Contents', 'Developer', 'Library', 'Frameworks'),
	os.path.join(XCODE_PATH, 'Contents', 'Developer', 'Platforms', 'MacOSX.platform', 'Developer', 'Library', 'PrivateFrameworks'),

	os.path.join(XCODE_PATH, 'Contents', 'PlugIns', 'Xcode3Core.ideplugin', 'Contents', 'Frameworks'),
	os.path.join(XCODE_PATH, 'Contents', 'SharedFrameworks', 'XCBuild.framework', 'Versions', 'A', 'Frameworks'),
]

# RPATH templates to try to add for each binary, if they exist.
# `{}` is substituted with the full path to the binary.
TRY_PATH_TEMPLATES = [
	'{}/../Frameworks',
	'{}/Frameworks',
	'{}/../SharedFrameworks',
	'{}/SharedFrameworks',
	'{}/../OtherFrameworks',
	'{}/OtherFrameworks',
	'{}/../SystemFrameworks',
	'{}/SystemFrameworks',
	'{}/../PlugIns',
	'{}/PlugIns',
]

# these are binaries that are repeated often throughout the source and we don't really care about them.
# it's mainly just lots of copies of the Swift system libraries.
EXCLUDED_BINARIES = [
	'libswiftAccelerate.dylib',
	'libswiftAppKit.dylib',
	'libswiftAVFoundation.dylib',
	'libswiftCloudKit.dylib',
	'libswiftCompression.dylib',
	'libswiftContacts.dylib',
	'libswiftCoreAudio.dylib',
	'libswiftCoreData.dylib',
	'libswiftCore.dylib',
	'libswiftCoreFoundation.dylib',
	'libswiftCoreGraphics.dylib',
	'libswiftCoreImage.dylib',
	'libswiftCoreLocation.dylib',
	'libswiftCoreMedia.dylib',
	'libswiftCoreMIDI.dylib',
	'libswiftCryptoTokenKit.dylib',
	'libswiftDarwin.dylib',
	'libswift_Differentiation.dylib',
	'libswiftDispatch.dylib',
	'libswiftFoundation.dylib',
	'libswiftGameplayKit.dylib',
	'libswiftGLKit.dylib',
	'libswiftIntents.dylib',
	'libswiftIOKit.dylib',
	'libswiftMapKit.dylib',
	'libswiftMetal.dylib',
	'libswiftMetalKit.dylib',
	'libswiftModelIO.dylib',
	'libswiftNaturalLanguage.dylib',
	'libswiftNetwork.dylib',
	'libswiftObjectiveC.dylib',
	'libswiftOpenCL.dylib',
	'libswiftos.dylib',
	'libswiftPhotos.dylib',
	'libswiftQuartzCore.dylib',
	'libswiftRemoteMirror.dylib',
	'libswiftSafariServices.dylib',
	'libswiftSceneKit.dylib',
	'libswiftsimd.dylib',
	'libswiftSpriteKit.dylib',
	'libswiftSwiftLang.dylib',
	'libswiftSwiftOnoneSupport.dylib',
	'libswiftVision.dylib',
	'libswiftXCTest.dylib',
	'libswiftXPC.dylib',
]

# these are the short names for the binaries we want to generate stubs for
IMPORTANT_BINARIES = [
	'AppKit',
	'AudioToolbox',
	'Cocoa',
	'CoreData',
	'CoreGraphics',
	'CoreText',
	'ImageIO',
	'OpenGL',
	'QuartzCore',
]

COPYRIGHT_HEADER = """
/*
 * This file is part of Darling.
 *
 * Copyright (C) {} Darling Developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */
"""

STUB_HEADER = """
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define LOG_FUNC __simple_kprintf
"""

TEMPLATE_STUB = """
void* {0}(void) {{
	if (verbose) LOG_FUNC("STUB: {0} called\\n");
	return NULL;
}};
"""

TEMPLATE_INTERFACE = """
@interface {} : NSObject
@end
"""

TEMPLATE_CATEGORY = """
@interface {}
@end
"""

TEMPLATE_IMPL = """
@implementation {}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}}

"""

TEMPLATE_METHOD = """
{0} (id){1}
{{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
	return nil;
}}
"""

CMAKE_TEMPLATE = """
project({0}_stub)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "{1}")

add_framework({0}
	FAT
	CURRENT_VERSION
	VERSION ${{FRAMEWORK_VERSION}}
	TARGET_NAME {0}${{STUB_SUFFIX}}
	${{NO_INSTALL_ARG}}

	SOURCES
		src/classes.m
		src/main.m

	DEPENDENCIES
		system
		Foundation
)

#target_include_directories({0}${{STUB_SUFFIX}} BEFORE PRIVATE include)
"""

# most variables are just string keys, so that's the default.
# these overrides for the definitions of certain variables prevent them from being defined as strings and instead define them with the appropriate values.
VAR_OVERRIDES = {
	'AppKit': {
		'NSUnderlineByWordMask':   'NSUInteger NSUnderlineByWordMask = 0x8000;',
		'NSApp':                   'NSObject *NSApp = nil;',
		'NSFontWeightBold':        'const CGFloat NSFontWeightBold       = 0x3fd99999a0000000;',
		'NSFontWeightMedium':      'const CGFloat NSFontWeightMedium     = 0x3fcd70a3e0000000;',
		'NSFontWeightRegular':     'const CGFloat NSFontWeightRegular    = 0x0000000000000000;',
		'NSFontWeightSemibold':    'const CGFloat NSFontWeightSemibold   = 0x3fd3333340000000;',
		'NSFontWeightThin':        'const CGFloat NSFontWeightThin       = 0xbfe3333340000000;',
		'NSFontWeightLight':       'const CGFloat NSFontWeightLight      = 0xbfd99999a0000000;',
		'NSFontWeightUltraLight':  'const CGFloat NSFontWeightUltraLight = 0xbfe99999a0000000;',
		'NSFontWeightBlack':       'const CGFloat NSFontWeightBlack      = 0x3fe3d70a40000000;',
		'NSFontWeightHeavy':       'const CGFloat NSFontWeightHeavy      = 0x3fe1eb8520000000;',
		'NSViewNoIntrinsicMetric': 'const CGFloat NSViewNoIntrinsicMetric = 0xbff0000000000000;',
		'NSAppKitVersionNumber':   'const double NSAppKitVersionNumber = 1504;',
	},
	'CoreGraphics': {
		'CGRectZero':            'const CGRect CGRectZero = {{0, 0}, {0, 0}};',
		'CGRectNull':            'const CGRect CGRectNull = {{INFINITY, INFINITY}, {0, 0}};',
		'CGPointZero':           'const CGPoint CGPointZero = {0, 0};',
		'CGSizeZero':            'const CGSize CGSizeZero = {0, 0};',
		'CGRectInfinite':        'const CGRect CGRectInfinite = {{0, 0}, {INFINITY, INFINITY}};',
		'kCGDisplayPixelHeight': 'unsigned int kCGDisplayPixelHeight = 1;',
		'kCGDisplayPixelWidth':  'unsigned int kCGDisplayPixelWidth = 1;',
	},
	'QuartzCore': {
		'CATransform3DIdentity': 'const CATransform3D CATransform3DIdentity = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};'
	},
}

# sometimes, functions are incorrectly detected as variables. this can be used to override certain symbols as functions in those cases.
# note that this should rarely be necessary, since we double-check with `nm`.
FUNC_OVERRIDES = {
	#'CoreGraphics': [
	#	'CGRectGetMinX',
	#	'CGRectGetWidth',
	#],
}

# this is necessary to ensure certain functions that we need but Xcode doesn't need still get stubbed.
ENSURE_FUNCS = {
	'CoreGraphics': [
		'CGSSetDenyWindowServerConnections',
	],
}

# this is necessary to ensure certain methods get stubbed as individual methods.
# for objc, we can stub using an invocation forwarding method. this is fine for the vast majority of cases.
# however, Xcode swizzles some methods, so they have to exist as individual methods.
ENSURE_METHODS = {
	'AppKit': {
		'NSColorWell': [
			'-activate:',
			'-deactivate',
		],
		'NSObject (BindingSupport)': [
			'-bind:toObject:withKeyPath:options:',
			'-infoForBinding:',
			'-unbind:',
		],
		'NSObject (NSKeyValueBindingCreation)': [
			'-exposedBindings',
		],
	},
}

# this is added to the top of the `main.m` source file for the corresponding framework.
PROLOGUES = {
	'AppKit': """
#import <CoreGraphics/CGGeometry.h>
""",

	'CoreGraphics': """
#include <math.h>

#ifdef __LP64__
	typedef double CGFloat;
#else
	typedef float CGFloat;
#endif

struct CGPoint {
	CGFloat x;
	CGFloat y;
};
typedef struct CGPoint CGPoint;

struct CGSize {
	CGFloat width;
	CGFloat height;
};
typedef struct CGSize CGSize;

struct CGVector {
	CGFloat dx;
	CGFloat dy;
};
typedef struct CGVector CGVector;

struct CGRect {
	CGPoint origin;
	CGSize size;
};
typedef struct CGRect CGRect;
""",

	'QuartzCore': """
#ifdef __LP64__
	typedef double CGFloat;
#else
	typedef float CGFloat;
#endif

typedef struct {
	CGFloat m11, m12, m13, m14;
	CGFloat m21, m22, m23, m24;
	CGFloat m31, m32, m33, m34;
	CGFloat m41, m42, m43, m44;
} CATransform3D;
""",
}

for bin in IMPORTANT_BINARIES:
	if not bin in VAR_OVERRIDES:
		VAR_OVERRIDES[bin] = {}
	if not bin in FUNC_OVERRIDES:
		FUNC_OVERRIDES[bin] = []

RPATH_REGEX  = r"\s*cmd LC_RPATH\n\s*cmdsize [0-9]+\n\s*path ([^(]+?)\s*\(offset"
DEPS_REGEX   = r"^\s*([^(\n]+?)\s*\("
VAR_REGEX    = r"^\s*(?:__DATA|__DATA_CONST)\s+[A-Za-z0-9_]+\s+0x[0-9a-fA-F]+\s+[A-Za-z_]+\s+[0-9]+\s+([A-Za-z0-9_]+)\s+((?!_OBJC_)[A-Za-z0-9_$]+)"
FUNC_REGEX   = r"^\s*(?!__DATA|__DATA_CONST)[A-Za-z0-9_]+\s+[A-Za-z0-9_]+\s+0x[0-9a-fA-F]+\s+[A-Za-z_]+\s+[0-9]+\s+([A-Za-z0-9_]+)\s+((?!_OBJC_)[A-Za-z0-9_$]+)"
CLASS_REGEX  = r"^\s*[A-Za-z0-9_]+\s+[A-Za-z0-9_]+\s+0x[0-9a-fA-F]+\s+[A-Za-z_]+\s+[0-9]+\s+([A-Za-z0-9_]+)\s+(_OBJC_[A-Za-z0-9_$]+)"
NM_REGEX     = r"^[0-9A-Fa-f]+\s+T\s+([A-Za-z_0-9$]+)"
LAZY_REGEX   = r"^\s*[A-Za-z0-9_]+\s+[A-Za-z0-9_]+\s+0x[0-9a-fA-F]+\s+0x[0-9a-fA-F]+\s+([A-Za-z0-9_]+)\s+((?!_OBJC_)[A-Za-z0-9_$]+)"

RPATHS: Dict[str, List[str]] = {}
DEPS: Dict[str, List[str]] = {}
NM_OVERRIDES: Dict[str, List[str]] = {}

class ResolutionError(Exception):
	def __init__(self, message: str) -> None:
		self.message = message
		super().__init__(message)

class SymbolType(Enum):
	VARIABLE = 1
	FUNCTION = 2
	OBJC_CLASS = 3

class Symbol:
	def __init__(self, binary: str, name: str, type: SymbolType) -> None:
		self.binary = binary
		self.name = name
		self.type = type
	def __eq__(self, other: object) -> bool:
		return isinstance(other, Symbol) and other.name == self.name and other.binary == self.binary
	def __hash__(self) -> int:
		return hash((self.binary, self.name))
	def __str__(self) -> str:
		return f'({self.binary}, {self.name}, {self.type})'
	def __repr__(self) -> str:
		return str(self)

def basename_no_ext(string: str) -> str:
	result = string
	while True:
		result, ext = os.path.splitext(result)
		if len(ext) == 0:
			break
	return result

def get_rpaths(binary: str, executable: str | None = None, extra_rpaths: List[str] | None = None, ignore_cache: bool = False) -> List[str]:
	if (not ignore_cache) and (binary in RPATHS):
		return RPATHS[binary]

	output = subprocess.check_output([OTOOL, '-l', binary]).decode()
	matches = re.finditer(RPATH_REGEX, output, re.MULTILINE)
	rpaths = [match.group(1) for match in matches]

	if extra_rpaths != None:
		rpaths += extra_rpaths

	final_rpaths: List[str] = []
	for rpath in rpaths:
		parts = pathlib.PurePosixPath(rpath).parts
		if parts[0] == '@executable_path' or parts[0] == '@executable':
			if executable == None:
				raise ResolutionError('Cannot use @executable_path when no executable is specified')
			else:
				final_rpaths.append(os.path.normpath(os.path.join(os.path.dirname(executable), *parts[1:])))
		elif parts[0] == '@loader_path':
			final_rpaths.append(os.path.normpath(os.path.join(os.path.dirname(binary), *parts[1:])))
		elif parts[0].startswith('@'):
			raise ResolutionError('Unrecognized special path root (' + parts[0] + ') in RPATH: ' + rpath)
		else:
			if rpath.startswith(XCODE_PATH + '/') or rpath.startswith(SYSTEM_ROOT + '/'):
				final_rpaths.append(rpath)
			else:
				final_rpaths.append(os.path.normpath(os.path.join(SYSTEM_ROOT, os.path.relpath(rpath, '/'))))

	for template in TRY_PATH_TEMPLATES:
		path = template.format(os.path.dirname(binary))
		if os.path.exists(path):
			final_rpaths.append(os.path.normpath(path))

	final_rpaths = sorted(set(final_rpaths))

	RPATHS[binary] = final_rpaths
	return final_rpaths

def generate_binary_list(xcode_path: str):
	# this is used to collect the root binaries for the search;
	# the Xcode binary in `MacOS` covers most things, but plug-ins are dynamically loaded by
	# Xcode at runtime, so we won't automatically pull them in with our search. instead, we
	# scan them as roots as well.
	bins: List[str] = [XCODE_MAIN_BINARY]
	for plugin in os.listdir(os.path.join(xcode_path, 'Contents', 'PlugIns')):
		full_path = os.path.join(xcode_path, 'Contents', 'PlugIns', plugin)
		basename = os.path.splitext(plugin)[0]
		fw_path = os.path.join(full_path, 'Versions', 'A', basename)
		bundle_path = os.path.join(full_path, 'Contents', 'MacOS', basename)
		if os.path.exists(fw_path):
			bins.append(fw_path)
		elif os.path.exists(bundle_path):
			bins.append(bundle_path)
	return bins

def get_deps(binary: str, recursive: bool = False, executable: str | None = None, processed: List[str] = []) -> List[str]:
	if binary in processed:
		return []

	processed.append(binary)

	output = subprocess.check_output([OTOOL, '-L', binary]).decode()
	matches = re.finditer(DEPS_REGEX, output, re.MULTILINE)
	deps = [match.group(1) for match in matches]

	final_deps: List[str] = []
	rpaths = get_rpaths(binary)

	for dep in deps:
		parts = pathlib.PurePosixPath(dep).parts
		if parts[0] == '@executable_path' or parts[0] == '@executable':
			if executable == None:
				raise ResolutionError('Cannot use @executable_path when no executable is specified')
			else:
				final_deps.append(os.path.normpath(os.path.join(os.path.dirname(executable), *parts[1:])))
		elif parts[0] == '@loader_path':
			final_deps.append(os.path.normpath(os.path.join(os.path.dirname(binary), *parts[1:])))
		elif parts[0] == '@rpath':
			found = False
			for rpath in rpaths:
				if os.path.exists(os.path.join(rpath, *parts[1:])):
					final_deps.append(os.path.normpath(os.path.join(rpath, *parts[1:])))
					found = True
					break
			if not found:
				raise ResolutionError('Could not find dependency in RPATH: ' + dep + '\nSearched: ' + str(rpaths))
		elif parts[0].startswith('@'):
			raise ResolutionError('Unrecognized special path root (' + parts[0] + ') in dependency: ' + dep)
		else:
			if dep.startswith('/Applications/Xcode.app/'):
				final_deps.append(os.path.join(XCODE_PATH, *parts[3:]))
			else:
				final_deps.append(os.path.normpath(dep))

	final_deps = sorted(set(final_deps))

	# some binaries reference themselves as dependencies (for some reason). fix that.
	if binary in final_deps:
		final_deps.remove(binary)

	# get rid of some repetitive duplicated dependencies
	final_deps = [x for x in final_deps if not os.path.basename(x) in EXCLUDED_BINARIES or x.startswith(SYSTEM_ROOT) or x.startswith('/usr')]

	DEPS[binary] = final_deps

	if recursive:
		recursive_deps = final_deps.copy()
		for dep in final_deps:
			if dep.startswith(XCODE_PATH + '/'):
				# overwrite the RPATH cache for the dependency since we're loading it
				get_rpaths(dep, executable, extra_rpaths = EXTRA_PLUGIN_RPATHS + rpaths, ignore_cache = True)
				try:
					recursive_deps += get_deps(dep, True, executable, processed)
				except ResolutionError as e:
					raise ResolutionError('Encountered error while processing dependency "' + dep + '" of "' + binary + '":\n' + e.message)
		recursive_deps = sorted(set(recursive_deps))
		return recursive_deps
	else:
		return final_deps

def get_bind_symbols(binary: str, binary_map: Dict[str, str]) -> List[Symbol]:
	get_bin_name = lambda name: binary_map[name] if name in binary_map else f'@{name}@'
	output = subprocess.check_output([DYLDINFO, '-bind', binary]).decode()
	var_matches = re.finditer(VAR_REGEX, output, re.MULTILINE)
	vars = list(set(Symbol(get_bin_name(match.group(1)), match.group(2), SymbolType.VARIABLE) for match in var_matches))
	func_matches = re.finditer(FUNC_REGEX, output, re.MULTILINE)
	funcs = list(set(Symbol(get_bin_name(match.group(1)), match.group(2), SymbolType.FUNCTION) for match in func_matches))
	class_matches = re.finditer(CLASS_REGEX, output, re.MULTILINE)
	classes = list(set(Symbol(get_bin_name(match.group(1)), match.group(2)[len('_OBJC_CLASS_$_'):], SymbolType.OBJC_CLASS) for match in class_matches if match.group(2).startswith('_OBJC_CLASS_$_')))

	# most lazy symbols are functions, so let's assume that and override manually as necessary
	output = subprocess.check_output([DYLDINFO, '-lazy_bind', binary]).decode()
	matches = re.finditer(LAZY_REGEX, output, re.MULTILINE)
	lazy = list(set(Symbol(get_bin_name(match.group(1)), match.group(2), SymbolType.FUNCTION) for match in matches))

	symbols = vars + funcs + classes + lazy

	for symbol in symbols:
		if symbol.binary.startswith('@') and symbol.binary.endswith('@'):
			continue
		if not symbol.binary in NM_OVERRIDES:
			if os.path.exists(SYSTEM_ROOT + symbol.binary) or os.path.exists(symbol.binary):
				output = subprocess.check_output([NM, '-Ug', '-arch', 'x86_64', (SYSTEM_ROOT + symbol.binary) if os.path.exists(SYSTEM_ROOT + symbol.binary) else symbol.binary]).decode()
				matches = re.finditer(NM_REGEX, output, re.MULTILINE)
				NM_OVERRIDES[symbol.binary] = [x.group(1) for x in matches]
			else:
				NM_OVERRIDES[symbol.binary] = []
		if symbol.name in NM_OVERRIDES[symbol.binary]:
			symbol.type = SymbolType.FUNCTION

	return symbols

def generate_method(method: str) -> str:
	type = method[0]
	signature = ''
	split = method[1:].split(':')
	if len(split) == 1:
		signature = split[0]
	else:
		for previous, current in zip(split, split[1:]):
			if len(signature) > 0:
				signature += ' '
			signature += f'{previous}: (id){previous}'
	return '\n\n' + TEMPLATE_METHOD.format(type, signature).strip()

def main():
	bins = generate_binary_list(XCODE_PATH)

	try:
		# cache the RPATHs for the root binaries
		for bin in bins:
			get_rpaths(bin, executable = XCODE_MAIN_BINARY, extra_rpaths = None if bin == XCODE_MAIN_BINARY else EXTRA_PLUGIN_RPATHS)

		all_bins: List[str] = []
		system_bins: List[str] = []
		xc_bins: List[str] = []

		# collect all the binaries that are loaded
		for bin in bins:
			all_bins.append(bin)
			all_bins += get_deps(bin, True, XCODE_MAIN_BINARY)

		all_bins = sorted(set(all_bins))

		system_bins_tmp = [x for x in all_bins if not x.startswith(XCODE_PATH + '/')]
		xc_bins = sorted(set(all_bins).difference(set(system_bins_tmp)))
		system_bins = sorted(x[len(SYSTEM_ROOT):] if x.startswith(SYSTEM_ROOT + '/') else x for x in system_bins_tmp)

		for bin in system_bins:
			print(bin)

		binary_map = {basename_no_ext(os.path.basename(x)): x for x in all_bins}
		important_bins = [x for x in all_bins if basename_no_ext(os.path.basename(x)) in IMPORTANT_BINARIES]

		# now let's get a list of all the symbols required at load time (and their associated binaries)
		all_symbols: List[Symbol] = []
		for bin in xc_bins:
			all_symbols += get_bind_symbols(bin, binary_map)
		for bin in system_bins:
			short_bin_name = basename_no_ext(os.path.basename(bin))
			if not short_bin_name in IMPORTANT_BINARIES:
				if os.path.exists(SYSTEM_ROOT + bin):
					all_symbols += get_bind_symbols(SYSTEM_ROOT + bin, binary_map)

		all_symbols = list(set(all_symbols))
		system_symbols = [x for x in all_symbols if x.binary in system_bins]
		important_symbols = [x for x in all_symbols if x.binary in important_bins]

		for symbol in important_symbols:
			bin_name = basename_no_ext(os.path.basename(symbol.binary))
			if symbol.name in FUNC_OVERRIDES[bin_name] or symbol.name[1:] in FUNC_OVERRIDES[bin_name]:
				symbol.type = SymbolType.FUNCTION

		important_symbols_separated: Dict[str, List[Symbol]] = {}

		for important_bin in IMPORTANT_BINARIES:
			important_symbols_separated[important_bin] = [x for x in important_symbols if basename_no_ext(os.path.basename(x.binary)) == important_bin]

		for important_bin, symbols in important_symbols_separated.items():
			THIS_DIR = os.path.join(OUT_DIR, important_bin)
			shutil.rmtree(THIS_DIR, ignore_errors=True)
			os.makedirs(THIS_DIR, exist_ok=True)
			classes = sorted(x.name for x in symbols if x.type == SymbolType.OBJC_CLASS)
			vars = sorted(x.name[1:] if x.name[0] == '_' else x.name for x in symbols if x.type == SymbolType.VARIABLE)
			funcs = sorted(x.name[1:] if x.name[0] == '_' else x.name for x in symbols if x.type == SymbolType.FUNCTION)

			os.makedirs(os.path.join(THIS_DIR, 'src'), exist_ok=True)
			#os.makedirs(os.path.join(THIS_DIR, 'include'), exist_ok=True)
			#os.makedirs(os.path.join(THIS_DIR, 'include', important_bin), exist_ok=True)

			with open(os.path.join(THIS_DIR, 'src', 'classes.m'), 'w') as classes_source:
				classes_source.write(COPYRIGHT_HEADER.format(datetime.datetime.now(datetime.timezone.utc).year).strip() + '\n\n')
				classes_source.write('#import <Foundation/Foundation.h>\n\n')

				for klass in classes:
					classes_source.write((TEMPLATE_CATEGORY if klass.find('(') >= 0 else TEMPLATE_INTERFACE).format(klass).strip() + '\n\n' + TEMPLATE_IMPL.format(klass).strip())
					if important_bin in ENSURE_METHODS and klass in ENSURE_METHODS[important_bin]:
						for method in ENSURE_METHODS[important_bin][klass]:
							classes_source.write(generate_method(method))
					classes_source.write('\n\n@end\n\n')

				if important_bin in ENSURE_METHODS:
					for klass in ENSURE_METHODS[important_bin]:
						if not klass in classes:
							classes_source.write((TEMPLATE_CATEGORY if klass.find('(') >= 0 else TEMPLATE_INTERFACE).format(klass).strip() + f'\n\n@implementation {klass}')
							for method in ENSURE_METHODS[important_bin][klass]:
								classes_source.write(generate_method(method))
							classes_source.write('\n\n@end\n\n')

			with open(os.path.join(THIS_DIR, 'CMakeLists.txt'), 'w') as cmake_txt:
				cmake_txt.write(CMAKE_TEMPLATE.format(important_bin, 'C' if important_bin == 'AppKit' else 'A').strip() + '\n')

			with open(os.path.join(THIS_DIR, 'src', 'main.m'), 'w') as file:
				file.write(COPYRIGHT_HEADER.format(datetime.datetime.now(datetime.timezone.utc).year).strip() + '\n\n')
				file.write('#import <objc/NSObject.h>\n\n')
				file.write('@interface NSString : NSObject\n@end\n\n')
				if important_bin in PROLOGUES:
					file.write(PROLOGUES[important_bin].strip() + '\n\n')
				file.write(STUB_HEADER.strip() + '\n\n')

				# most variables are just string keys, so default to that and then manually fix as necessary
				#
				# note that we don't really care what the value is in the string case (these are stubs, after all)
				for var in vars:
					if var in VAR_OVERRIDES[important_bin]:
						file.write(VAR_OVERRIDES[important_bin][var] + '\n')
					else:
						file.write(f'NSString *const {var} = @"{var}";\n')

				file.write('\n')

				for func in funcs:
					file.write(TEMPLATE_STUB.format(func).strip() + '\n\n')

				file.write('\n')

				if important_bin in ENSURE_FUNCS:
					for func in ENSURE_FUNCS[important_bin]:
						file.write(TEMPLATE_STUB.format(func).strip() + '\n\n')

	except ResolutionError as e:
		print(e.message)
		exit(1)

if __name__ == '__main__':
	main()
