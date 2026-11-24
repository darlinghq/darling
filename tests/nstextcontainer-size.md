# NSTextContainer size names

nstextcontainer-size.m checks `-[NSTextContainer size]`, `-setSize:` and `-initWithSize:` against the public documentation: `size` caps what the container returns for a proposed line rectangle, `-initWithSize:` is the designated initializer, and the older `containerSize` and `initWithContainerSize:` are the same property and initializer under their deprecated names. It also checks that a narrower size makes the text wrap onto more lines.

It needs a Darling guest to run (AppKit and a font, no window server), and has not been run. Build it with the ordinary Objective-C command for the other AppKit tests, without ARC. The calls to the new selectors go through a protocol, so against a tree without them the test builds and reports FAIL lines instead of raising: on master, the three `respondsToSelector` checks fail and the test stops.

Not stated on the pages and marked GUESS in the source: that the two spellings read and write the same value, that a size change lays the text out again, and that the layout manager is told of a change once per change whichever name sets it. Which of the two setters calls the other is not tested (it is an implementation choice, not documented behaviour).
