# Volumes, eject, trash, icons and reveal

`volumes.sh` checks the volume, eject, trash, icon and reveal APIs: the `statfs` mount flags, CoreFoundation's
volume resource keys, Foundation's volume list and trash, and NSWorkspace's volume calls, eject, mount
notifications, icons, recycling and reveal.

```sh
DARLING_SKILLS=<darling-skills checkout> tests/volumes.sh
```

It needs a running Darling with the launcher under test first in `PATH`. For the unprivileged modes it also needs
`DARLING_ROOTLESS=1` or `DARLING_NONROOT=1`, each with a `DPREFIX` of its own.

The programs it builds also build and pass on macOS, unless their comment says otherwise.

## Observed on macOS 26.7.3 (25G342), x86_64

**Volume keys:**

| Volume | Name | Ejectable / Removable | ReadOnly | Local | Internal | Browsable | RootFS | FormatDescription | TypeName | MountFromLocation |
|--------|------|-----------------------|----------|-------|----------|-----------|--------|-------------------|----------|-------------------|
| `/Volumes/Unmount Test` (UDZO) | "Unmount Test" | YES / YES | YES | YES | nil | YES | NO | "Mac OS Extended" | "hfs" | "disk5s1" |
| `/` | "System" | NO / NO | YES | YES | YES | YES | YES | "APFS" | "apfs" | "disk2s4s1" |

- A path inside a volume answers with its volume's values.
- `NSURLVolumeURLKey` is the mount point as a directory URL, and `NSURLVolumeIdentifierKey` is 8 bytes of data.

**`statfs` flags:**

| Volume | Flags |
|--------|-------|
| a disk image | `MNT_REMOVABLE\|MNT_LOCAL` (plus `MNT_RDONLY` when read-only) |
| `/` | `MNT_LOCAL\|MNT_ROOTFS` |
| `/System/Volumes/VM` | `MNT_DONTBROWSE` |
| an autofs mount | `MNT_DONTBROWSE\|MNT_AUTOMOUNTED`, no `MNT_LOCAL` |

**Enumeration:**
- `mountedVolumeURLsIncludingResourceValuesForKeys:options:0` lists every mounted volume, root first.
- With `NSVolumeEnumerationSkipHiddenVolumes` only `/` and the image are left.
- `nil` keys are accepted.

**NSWorkspace volume calls:**
- `mountedLocalVolumePaths` is every volume path from that list (it even includes autofs).
- `mountedRemovableMedia` gives only the image.
- `getFileSystemInfoForPath:`:

| Path | removable | writable | unmountable | description, type |
|------|-----------|----------|-------------|-------------------|
| the image | 1 | 0 | 1 | "hfs", "hfs" |
| `/` | 0 | 0 | 0 | "apfs", "apfs" |
| a folder in `/tmp` | 0 | 1 | 0 | "apfs", "apfs" |

**`unmountAndEjectDeviceAtURL:error:`:**

| Target | Result |
|--------|--------|
| `/nonexistent` | NO, `NSOSStatusErrorDomain` −36 |
| `/tmp`, `/` | NO, −47 |
| the image, with a dissenter running | NO, −47, empty userInfo |
| a file inside the volume | YES: its volume is ejected |

`unmountAndEjectDeviceAtPath:` returns the same answer as a BOOL.

**Notifications** go to `NSWorkspace.sharedWorkspace.notificationCenter`, on the main thread. The object is the workspace. The userInfo has:
- `NSDevicePath`: the mount point as a string;
- `NSWorkspaceVolumeURLKey`: the mount point as a directory URL;
- `NSWorkspaceVolumeLocalizedNameKey`: the volume name.

They arrive as follows:

| Event | Notifications |
|-------|---------------|
| `hdiutil attach` | DidMount |
| `hdiutil detach` | WillUnmount, then DidUnmount |
| our own synchronous eject, on the main thread | WillUnmount, then DidUnmount, both before the call returns |
| an eject that is dissented | WillUnmount only |

**Trash:**
- `URLForDirectory:NSTrashDirectory inDomain:NSUserDomainMask appropriateForURL:nil` is `~/.Trash`.
- For a URL on another volume it is `<mount point>/.Trashes/<uid>`. If that directory doesn't exist yet, the answer is `NSFeatureUnsupportedError` (3328) with an underlying POSIX ENOENT. A read-only volume gives 3328 with an underlying EROFS.
- `trashItemAtURL:` errors:

| Item | Error |
|------|-------|
| an item on another volume | none: it goes to that volume's `.Trashes/<uid>` |
| a missing file | `NSFileNoSuchFileError` (4) |
| a file on a read-only volume | 3328 |
| an `http:` URL | `NSFileReadUnsupportedSchemeError` (262) |

- macOS renames a colliding item `name HH-MM-SS-mmm.ext`. Darling keeps its documented-safe `name N.ext`.
- `recycleURLs:completionHandler:` calls its handler on the main thread. `newURLs` lists only the items that moved. When one item fails, the error has code 4 (that failure's code), and its userInfo has exactly `NSLocalizedDescription` and the key `"NSUnderlyingErrors"` (not the documented `NSMultipleUnderlyingErrorsKey`), whose value lists the failures.

**Icons** (all 32×32; compared by pixels):

| Item | Icon |
|------|------|
| a missing path, `""`, an unknown extension, no extension | the `public.data` type icon |
| a folder | the `public.folder` icon |
| `a.txt` | the `txt` icon |
| a `.dmg` (`com.apple.disk-image-udif`, which conforms to `public.disk-image`) | its own icon |
| an `.app` without an icon | a generic application icon |
| `/` | an internal-disk icon |
| the image's mount point | a removable-volume icon, different from `/` |

`iconForFiles:` gives:
- `@[]`: nil;
- one file: the same pixels as `iconForFile:`;
- two files: `NSImageNameMultipleDocuments`.

### Reveal (observed on macOS 26 by the maintainer, with a logger app registered as `com.apple.finder`)

| Call | Event the file viewer gets |
|------|----------------------------|
| `selectFile:inFileViewerRootedAtPath:` | `aevt`/`odoc`, a list of 1 bookmark-data item, `'aapd'` true |
| `activateFileViewerSelectingURLs:` | one `aevt`/`odoc` with all items in order, `'aapd'` true |
| `selectFile:nil inFileViewerRootedAtPath:<dir>` | `aevt`/`rapp`, no items |

The root path is never sent. The file viewer is found by bundle ID: the `NSFileViewer` default, else `com.apple.finder`.


### Events LaunchServices delivers (macOS 26.7.3, with `ae-receiver.m` as the application)

The receiver installs its handlers in `applicationWillFinishLaunching:`. A process without `NSApplication` (a plain
CFRunLoop or NSRunLoop with `AEInstallEventHandler` or `NSAppleEventManager`) received no events at all.

| Call | Application | Event |
|------|-------------|-------|
| `open -a App` | not running | `aevt`/`oapp`, no `'aapd'` |
| `open -a App` | running | `aevt`/`rapp`, `'aapd'` true |
| `open -a App file`, `LSOpenFromURLSpec` with the app and a file | either | `aevt`/`odoc`, 1 bookmark-data item, `'aapd'` true |
| `LSOpenApplication`, no initial event | not running | `aevt`/`oapp`, no `'aapd'` |
| `LSOpenApplication`, no initial event | running | `aevt`/`rapp`, `'aapd'` true; the same process |
| `LSOpenApplication` with an initial `aevt`/`odoc` of two `typeFileURL` items | either | that event, its items unchanged, `'aapd'` added |

`LSOpenApplication` with a NULL `application` gives `paramErr` (-50). With a directory that has no executable, it
gives `kLSNoExecutableErr` (-10827).
