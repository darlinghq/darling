// Copyright (C) 2026 Lubos Dolezel
//
// The Swift spellings of the common AppKit classes the way applications use them: singletons and class
// properties, instance properties (as opposed to getter and setter methods) and renamed methods. Typecheck
// against the SDK with a Foundation overlay module; running it needs the AppKit framework.
import AppKit

var failures = 0
func check(_ ok: Bool, _ what: String) {
    if !ok {
        print("FAIL: \(what)")
        failures += 1
    }
}

// Singletons and class properties.
let app = NSApplication.shared
let workspace = NSWorkspace.shared
let pasteboard = NSPasteboard.general
let cursor: NSCursor = NSCursor.arrow
let hand: NSCursor = NSCursor.pointingHand
let ibeam: NSCursor = NSCursor.iBeam
let label: NSColor = NSColor.labelColor
let accent: NSColor = NSColor.controlAccentColor
let windowBackground: NSColor = NSColor.windowBackgroundColor
let fontManager = NSFontManager.shared
let colorPanel = NSColorPanel.shared
let spell = NSSpellChecker.shared
let lineWidth = NSBezierPath.defaultLineWidth

// Properties of the common classes.
func properties(_ window: NSWindow, _ view: NSView, _ event: NSEvent, _ table: NSTableView, _ text: NSTextView,
                _ font: NSFont, _ menu: NSMenu, _ alert: NSAlert, _ panel: NSOpenPanel) {
    window.title = "x"
    window.contentView = view
    window.isOpaque = false
    window.backgroundColor = label
    let _: NSRect = window.frame
    let _: NSScreen? = window.screen
    let _: Bool = window.isVisible && window.isKeyWindow && window.isMainWindow && window.isMiniaturized
    let _: NSResponder? = window.firstResponder
    let _: Int = window.windowNumber
    window.delegate = nil
    window.toolbar = nil

    view.isHidden = false
    view.frame = NSRect(x: 0, y: 0, width: 1, height: 1)
    view.bounds = view.frame
    view.autoresizesSubviews = true
    view.wantsLayer = true
    let _: NSView? = view.superview
    _ = view.subviews
    let _: NSWindow? = view.window
    let _: NSRect = view.visibleRect
    _ = view.trackingAreas
    let _: Int = view.tag

    app.mainMenu = menu
    app.windowsMenu = menu
    app.applicationIconImage = nil
    _ = app.windows
    let _: NSWindow? = app.keyWindow
    let _: NSEvent? = app.currentEvent
    let _: Bool = app.isRunning && app.isActive

    let _: NSPoint = event.locationInWindow
    let _: Int = event.windowNumber
    let _: String? = event.characters
    let _: UInt16 = event.keyCode
    let _: CGFloat = event.deltaX
    let _: Bool = event.isARepeat
    let _: Int = event.buttonNumber

    table.dataSource = nil
    table.delegate = nil
    table.rowHeight = 20
    table.headerView = nil
    let _: Int = table.numberOfRows
    let _: Int = table.selectedRow
    let _: Int = table.clickedRow
    _ = table.tableColumns

    text.string = "x"
    text.isEditable = false
    text.font = font
    text.textColor = label
    let _: NSTextStorage? = text.textStorage
    let _: NSLayoutManager? = text.layoutManager

    let _: CGFloat = font.pointSize
    let _: String = font.fontName
    let _: String? = font.familyName
    let _: CGFloat = font.ascender + font.descender + font.leading + font.xHeight + font.capHeight

    menu.title = "x"
    menu.autoenablesItems = false
    menu.delegate = nil
    let _: Int = menu.numberOfItems

    alert.messageText = "x"
    alert.informativeText = "y"
    alert.icon = nil
    _ = alert.buttons

    panel.canChooseFiles = true
    panel.canChooseDirectories = false
    panel.allowsMultipleSelection = true
    panel.directoryURL = nil
    panel.message = "x"
}

// Renamed methods.
func methods(_ view: NSView, _ window: NSWindow, _ color: NSColor, _ table: NSTableView, _ outline: NSOutlineView) {
    app.activate(ignoringOtherApps: true)
    app.postEvent(NSEvent(), atStart: true)
    app.sendEvent(NSEvent())
    _ = color.blended(withFraction: 0.5, of: label)
    _ = NSColor(srgbRed: 1, green: 0, blue: 0, alpha: 1)
    window.makeKeyAndOrderFront(nil)
    window.orderOut(nil)
    view.setNeedsDisplay(view.bounds)
    outline.expandItem(nil)
    outline.reloadItem(nil, reloadChildren: true)
    table.reloadData()
}

check(NSApplication.shared === app, "NSApplication.shared is one object")
check(NSWorkspace.shared === workspace, "NSWorkspace.shared is one object")
check(NSPasteboard.general === pasteboard, "NSPasteboard.general is one object")
check(NSCursor.arrow === cursor, "NSCursor.arrow is one object")
check(NSBezierPath.defaultLineWidth == lineWidth, "NSBezierPath.defaultLineWidth reads back")

if failures == 0 {
    print("ALL PASSED")
} else {
    exit(1)
}
