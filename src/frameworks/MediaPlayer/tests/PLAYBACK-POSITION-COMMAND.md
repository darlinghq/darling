# Playback position command class

Compile `playback-position-command.m` as a Mach-O against this build's
MediaPlayer, Foundation, Objective-C runtime and libSystem, with the normal
framework reexport mappings. Run in a private Darling prefix.

Before the fix, exit 1 reports the missing specialized runtime class. Afterward,
the test checks the documented subclass relationship, stable command identity,
mutation of inherited enabled state, and separation from the play command.
This checks the object contract; it does not test system media-key delivery.

Provenance: Apple public API documentation (rung 3):
- https://developer.apple.com/documentation/mediaplayer/mpremotecommandcenter/changeplaybackpositioncommand
- https://developer.apple.com/documentation/mediaplayer/mpchangeplaybackpositioncommand
