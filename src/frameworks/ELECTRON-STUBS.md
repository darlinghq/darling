# Frameworks added so that Electron-based apps can get past the dyld stage

Electron's main framework links several system frameworks that Darling did
not provide. dyld aborts the process at load time when a non-weak dependency
is missing (see upstream issues #838 and #1708), so the frameworks below
exist first of all to satisfy the loader and the imported symbols.

None of them implements real functionality. They follow the same convention
as the other `gui_stubs` frameworks:

| Framework | Exports | Behavior |
|---|---|---|
| Accessibility | `AXCustomContent`, `AXPrefersNonBlinkingTextInsertionIndicator()`, `...DidChangeNotification` | class is a stub; the function returns `false` (documented default) |
| CoreHaptics | `CHHapticEvent`, `CHHapticEventParameter`, `CHHapticDynamicParameter`, `CHHapticPattern`, 5 `CHHaptic*ID`/type string constants | stub classes, constants only |
| CoreML | `MLModel`, `MLModelConfiguration`, `MLMultiArray`, `MLFeatureValue`, `MLPredictionOptions`, `MLComputePlan`, `ML{CPU,GPU,NeuralEngine}ComputeDevice` | stub classes |
| MediaPlayer | `MPMediaItemArtwork`, `MPNowPlayingInfoCenter`, `MPRemoteCommandCenter`, 8 `MP*Property*` string constants | stub classes, constants only |
| QuickLookThumbnailing | `QLThumbnailGenerator`, `QLThumbnailGenerationRequest` | stub classes |
| ScreenCaptureKit | `SC*` classes (6), 7 `SCStreamFrameInfo*` string constants | stub classes, constants only |
| MediaAccessibility (existing) | added `kMACaptionAppearanceSettingsChangedNotification` | the rest of the framework was already a stub |

Stub classes accept any instance or class message, log
`Stub called: <selector> in <class>` through NSLog, and return nil/0.
Constants are plain strings whose values equal their names; they are
identifiers, not behavior.

The symbol lists were taken from the undefined-symbol imports of a publicly
distributed Electron binary (names only). Frameworks the bundle also lists but
which are still absent: CryptoKit, GSS, Kerberos (only used by optional native
add-ons, not by the Electron main framework).
