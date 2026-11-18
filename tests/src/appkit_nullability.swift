// Copyright (C) 2026 Lubos Dolezel
//
// Optionality and element types of the common AppKit classes as in the public SDK: initialisers and most
// accessors are non-optional, the documented "may be nil" ones are optional, and collections carry their
// element type. Typecheck against the SDK with a Foundation overlay module; nothing is run.
import AppKit

func nonOptionalResults() {
    let menu = NSMenu(title: "x")
    let item = NSMenuItem(title: "i", action: nil, keyEquivalent: "")
    menu.addItem(item)
    let _: [NSMenuItem] = menu.itemArray()
    let _: NSMenuItem? = menu.item(at: 0)
    let _: NSMenu? = item.submenu
    let _: NSMenu? = menu.supermenu

    let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 10, height: 10),
                          styleMask: NSWindow.StyleMask(rawValue: 1), backing: .buffered, defer: true)
    window.title = "x"
    let _: String = window.title
    let _: NSView? = window.contentView
    let _: NSScreen? = window.screen
    let _: NSResponder? = window.firstResponder
    let _: [NSWindow]? = window.childWindows.map { $0 }
    let _: NSWindow? = window.parent

    let view = NSView(frame: NSRect(x: 0, y: 0, width: 5, height: 5))
    let _: [NSView] = view.subviews
    let _: NSView? = view.superview
    let _: NSWindow? = view.window
    let _: [NSTrackingArea] = view.trackingAreas
    view.addSubview(NSView())

    let app = NSApplication.shared
    let _: [NSWindow] = app.windows
    let _: NSWindow? = app.keyWindow
    let _: NSMenu? = app.mainMenu
    let _: NSEvent? = app.currentEvent

    let button = NSButton(title: "t", target: nil, action: nil)
    let _: String = button.title
    let field = NSTextField(labelWithString: "s")
    let _: String = field.stringValue
    let _: NSFont? = field.font
    let _: NSImage? = button.image

    let table = NSTableView()
    let _: [NSTableColumn] = table.tableColumns
    let _: [NSScreen]? = NSScreen.screens
    let _: NSScreen? = NSScreen.main
    let _: [NSRunningApplication] = NSWorkspace.shared.runningApplications
}
