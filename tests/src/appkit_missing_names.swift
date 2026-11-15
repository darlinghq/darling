// Copyright (C) 2026 Lubos Dolezel
//
// Swift spellings of AppKit enums, option sets and constants that the headers did not declare before:
// pointing devices and button masks, pasteboard options, font traits, dragging, image load status, the
// about-panel keys, scroller parts, table and scroll view enums, and the Touch Bar image names.
// Typecheck against the SDK with a Foundation overlay module; nothing is run.
import AppKit

func events() {
    let _: NSEvent.PointingDeviceType = .pen
    let _: NSEvent.ButtonMask = .penTip
    let _: NSEvent.GestureAxis = .horizontal
    let _: NSEvent.SwipeTrackingOptions = .lockDirection
}

func pasteboard() {
    let _: NSPasteboard.ReadingOptions = .asString
    let _: NSPasteboard.WritingOptions = .promised
}

func fonts() {
    let _: NSFontDescriptor.SymbolicTraits = [.italic, .bold]
    let _: NSFontDescriptor.SymbolicTraits = .monoSpace
}

func dragging() {
    let _: NSDraggingItemEnumerationOptions = .concurrent
    let _: NSDraggingContext = .withinApplication
    let _: NSSpringLoadingHighlight = .standard
}

func appAndImages() {
    let _: NSApplication.AboutPanelOptionKey = .credits
    let _: NSApplication.AboutPanelOptionKey = .applicationName
    _ = NSImage.touchBarAddTemplateName
    _ = NSImage.touchBarGoBackTemplateName
    let _: NSImage.LoadStatus = .readError
    let _: NSApplication.OcclusionState = .visible
}

func views() {
    let _: NSScroller.Part = .knob
    let _: NSScrollView.FindBarPosition = .belowContent
    let _: NSTableView.ColumnAutoresizingStyle = .uniformColumnAutoresizingStyle
    let _: NSTableView.RowActionEdge = .leading
    let _: NSWorkspace.AuthorizationType = .createSymbolicLink
}
