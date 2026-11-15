// Copyright (C) 2026 Lubos Dolezel
//
// The Swift spellings of AppKit's typed constants and the types that carry them (NSPasteboard.PasteboardType,
// NSImage.Name, NSToolbarItem.Identifier, NSApplication.ModalResponse, ...), written the way applications
// use them. The few string values checked are ones that are the same on every macOS version. Typecheck
// against the SDK with a Foundation overlay module; running it needs the AppKit framework.
import AppKit

var failures = 0
func check(_ ok: Bool, _ what: String) {
    if !ok {
        print("FAIL: \(what)")
        failures += 1
    }
}

func usage(pb: NSPasteboard, panel: NSOpenPanel, alert: NSAlert, window: NSWindow) {
 pb.declareTypes([.string, .fileURL, .URL, .tiff, .png, .pdf, .rtf, .html, .color, .font, .multipleTextSelection, .tabularText], owner: nil)
 _ = pb.string(forType: .string)
 pb.setString("x", forType: .string)
 let _ = NSPasteboard.general
 let _: NSPasteboard.Name = .find
 if panel.runModal() == .OK { }
 if alert.runModal() == .alertFirstButtonReturn { }
 let _: NSApplication.ModalResponse = .cancel
 let _: NSApplication.ModalResponse = .stop
 let _ = NSImage(named: NSImage.advancedName)
 let _ = NSImage(named: NSImage.Name("MyIcon"))
 let _: NSImage.Name = NSImage.applicationIconName
 let _: NSImage.Name = NSImage.folderName
 let _: NSImage.Name = NSImage.actionTemplateName
 let _: NSImage.Name = NSImage.cautionName
 let _: NSImage.Name = NSImage.infoName
 let _: NSImage.Name = NSImage.userName
 window.appearance = NSAppearance(named: .darkAqua)
 window.appearance = NSAppearance(named: .aqua)
 window.setFrameAutosaveName("x")
 let _: [NSToolbarItem.Identifier] = [.flexibleSpace, .space, .separator, .showColors, .showFonts, .print, .customizeToolbar]
 let _: NSColorSpaceName = .calibratedRGB
 let _: NSControl.StateValue = .off
 let _: NSColor.Name = "foo"
 let _: [NSBitmapImageRep.PropertyKey: Any] = [.compressionFactor: 0.5, .interlaced: true]
 let _: [NSAttributedString.DocumentAttributeKey: Any] = [.documentType: NSAttributedString.DocumentType.rtf, .characterEncoding: 4]
 let _: NSAttributedString.DocumentType = .html
 let _: NSAttributedString.DocumentType = .plain
 let _: NSFontDescriptor.AttributeName = .family
 let _: [NSFontDescriptor.AttributeName: Any] = [.size: 12, .name: "x", .traits: [NSFontDescriptor.TraitKey.weight: 0.3]]
 let _: NSFont.Weight = .semibold
 let _: NSAppKitVersion = .current
 let _: NSWindow.Level = .modalPanel
 let _: NSStoryboard.Name = "Main"
 let _: NSNib.Name = "x"
 let _: NSAppearance.Name = .vibrantDark
 let _: NSTouchBarItem.Identifier = .flexibleSpace
 let _: NSSharingService.Name = .composeEmail
 let _: NSPrintInfo.AttributeKey = .jobDisposition
 let _: NSPrintInfo.JobDisposition = .save
}

// Values, run only when the framework is present.
check(NSImage.addTemplateName == "NSAddTemplate", "NSImage.addTemplateName")
check(NSPasteboard.PasteboardType.png.rawValue == "public.png", "NSPasteboard.PasteboardType.png")
check(NSPasteboard.PasteboardType.fileURL.rawValue == "public.file-url", "NSPasteboard.PasteboardType.fileURL")
check(NSPasteboard.PasteboardType("com.example.type").rawValue == "com.example.type", "PasteboardType(_:) keeps its string")
check(NSAppearance.Name.darkAqua.rawValue == "NSAppearanceNameDarkAqua", "NSAppearance.Name.darkAqua")
check(NSToolbarItem.Identifier("com.example.item").rawValue == "com.example.item", "NSToolbarItem.Identifier(_:)")
check(NSApplication.ModalResponse.OK.rawValue == 1, "NSApplication.ModalResponse.OK")
check(NSApplication.ModalResponse.cancel.rawValue == 0, "NSApplication.ModalResponse.cancel")
check(NSApplication.ModalResponse.stop.rawValue == -1000, "NSApplication.ModalResponse.stop")
check(NSApplication.ModalResponse.abort.rawValue == -1001, "NSApplication.ModalResponse.abort")
check(NSApplication.ModalResponse.continue.rawValue == -1002, "NSApplication.ModalResponse.continue")
check(NSApplication.ModalResponse.alertFirstButtonReturn.rawValue == 1000, "NSApplication.ModalResponse.alertFirstButtonReturn")
check(NSApplication.ModalResponse.alertSecondButtonReturn.rawValue == 1001, "NSApplication.ModalResponse.alertSecondButtonReturn")
check(NSControl.StateValue.on.rawValue == 1 && NSControl.StateValue.off.rawValue == 0 && NSControl.StateValue.mixed.rawValue == -1,
      "NSControl.StateValue")

if failures == 0 {
    print("ALL PASSED")
} else {
    exit(1)
}
