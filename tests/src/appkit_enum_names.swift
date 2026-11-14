// Copyright (C) 2026 Lubos Dolezel
//
// The Swift spellings of AppKit's nested enums and option sets, written the way applications use them:
// cases, OptionSet members and the types nested in their classes. The values are the public ones, so the
// same file runs against macOS. Typecheck against the SDK with a Foundation overlay module; running it
// needs the AppKit framework.
import AppKit

var failures = 0
func check(_ ok: Bool, _ what: String) {
    if !ok {
        print("FAIL: \(what)")
        failures += 1
    }
}

// Events.
let down: NSEvent.EventType = .leftMouseDown
check(down.rawValue == 1, "NSEvent.EventType.leftMouseDown")
check(NSEvent.EventType.keyDown.rawValue == 10, "NSEvent.EventType.keyDown")
check(NSEvent.EventType.scrollWheel.rawValue == 22, "NSEvent.EventType.scrollWheel")
let mods: NSEvent.ModifierFlags = [.command, .shift, .option, .control, .capsLock]
check(mods.contains(.command), "NSEvent.ModifierFlags.contains")
let mask: NSEvent.EventTypeMask = [.leftMouseDown, .keyDown]
check(mask.rawValue == (1 << 1) | (1 << 10), "NSEvent.EventTypeMask")
let phase: NSEvent.Phase = .began
check(phase.rawValue == 1, "NSEvent.Phase.began")

// Views, windows and the application.
let resizing: NSView.AutoresizingMask = [.width, .height]
check(resizing.rawValue == 2 | 16, "NSView.AutoresizingMask")
check(NSView.AutoresizingMask.minXMargin.rawValue == 1, "NSView.AutoresizingMask.minXMargin")
check(NSView.AutoresizingMask.maxYMargin.rawValue == 32, "NSView.AutoresizingMask.maxYMargin")
check(NSView.AutoresizingMask.none.rawValue == 0, "NSView.AutoresizingMask.none")
let style: NSWindow.StyleMask = [.titled, .closable, .miniaturizable, .resizable]
check(style.rawValue == 0xf, "NSWindow.StyleMask")
check(NSWindow.StyleMask.borderless.rawValue == 0, "NSWindow.StyleMask.borderless")
check(NSWindow.StyleMask.fullSizeContentView.rawValue == 0x8000, "NSWindow.StyleMask.fullSizeContentView")
check(NSWindow.StyleMask.unifiedTitleAndToolbar.rawValue == 0x1000, "NSWindow.StyleMask.unifiedTitleAndToolbar")
check(NSWindow.BackingStoreType.buffered.rawValue == 2, "NSWindow.BackingStoreType.buffered")
let behavior: NSWindow.CollectionBehavior = .canJoinAllSpaces
check(behavior.rawValue == 1, "NSWindow.CollectionBehavior.canJoinAllSpaces")
check(NSWindow.TitleVisibility.hidden.rawValue == 1, "NSWindow.TitleVisibility.hidden")
check(NSWindow.SelectionDirection.selectingNext.rawValue == 1, "NSWindow.SelectionDirection.selectingNext")
check(NSApplication.ActivationPolicy.regular.rawValue == 0, "NSApplication.ActivationPolicy.regular")
check(NSApplication.ActivationPolicy.accessory.rawValue == 1, "NSApplication.ActivationPolicy.accessory")
check(NSApplication.TerminateReply.terminateNow.rawValue == 1, "NSApplication.TerminateReply.terminateNow")
check(NSApplication.RequestUserAttentionType.criticalRequest.rawValue == 0, "NSApplication.RequestUserAttentionType")
let presentation: NSApplication.PresentationOptions = .autoHideDock
check(presentation.contains(.autoHideDock), "NSApplication.PresentationOptions")
check(NSAlert.Style.warning.rawValue == 0 && NSAlert.Style.critical.rawValue == 2, "NSAlert.Style")
check(NSView.BackgroundStyle.normal.rawValue == 0 && NSView.BackgroundStyle.emphasized.rawValue == 1, "NSView.BackgroundStyle")

// Controls.
check(NSButton.BezelStyle.rounded.rawValue == 1, "NSButton.BezelStyle.rounded")
check(NSButton.BezelStyle.texturedRounded.rawValue == 11, "NSButton.BezelStyle.texturedRounded")
check(NSButton.ButtonType.momentaryLight.rawValue == 0, "NSButton.ButtonType.momentaryLight")
check(NSButton.ButtonType.switch.rawValue == 3, "NSButton.ButtonType.switch")
check(NSButton.ButtonType.radio.rawValue == 4, "NSButton.ButtonType.radio")
check(NSControl.ControlSize.regular.rawValue == 0 && NSControl.ControlSize.small.rawValue == 1, "NSControl.ControlSize")
check(NSControl.ImagePosition.imageLeft.rawValue == 2, "NSControl.ImagePosition.imageLeft")
check(NSTextField.BezelStyle.roundedBezel.rawValue == 1, "NSTextField.BezelStyle.roundedBezel")
check(NSSegmentedControl.SwitchTracking.selectOne.rawValue == 0, "NSSegmentedControl.SwitchTracking")
check(NSSlider.TickMarkPosition.below.rawValue == 0 && NSSlider.TickMarkPosition.above.rawValue == 1, "NSSlider.TickMarkPosition")
check(NSSlider.SliderType.circular.rawValue == 1, "NSSlider.SliderType.circular")
check(NSScroller.UsableParts.allScrollerParts.rawValue == 2, "NSScroller.UsableParts")
check(NSSplitView.DividerStyle.thin.rawValue == 2, "NSSplitView.DividerStyle.thin")
check(NSTabView.TabType.topTabsBezelBorder.rawValue == 0, "NSTabView.TabType.topTabsBezelBorder")
check(NSBox.BoxType.custom.rawValue == 4, "NSBox.BoxType.custom")
check(NSBox.TitlePosition.atTop.rawValue == 2, "NSBox.TitlePosition.atTop")
check(NSImageScaling.scaleProportionallyDown.rawValue == 0, "NSImageScaling.scaleProportionallyDown")
check(NSImageScaling.scaleNone.rawValue == 2, "NSImageScaling.scaleNone")
check(NSImageAlignment.alignCenter.rawValue == 0, "NSImageAlignment.alignCenter")
check(NSImageView.FrameStyle.photo.rawValue == 1, "NSImageView.FrameStyle.photo")
check(NSImage.CacheMode.default.rawValue == 0, "NSImage.CacheMode.default")
check(NSToolbar.DisplayMode.iconOnly.rawValue == 2, "NSToolbar.DisplayMode.iconOnly")
check(NSTokenField.TokenStyle.rounded.rawValue == 2, "NSTokenField.TokenStyle.rounded")
check(NSLevelIndicator.Style.continuousCapacity.rawValue == 1, "NSLevelIndicator.Style.continuousCapacity")
check(NSPopUpButton.ArrowPosition.arrowAtCenter.rawValue == 1, "NSPopUpButton.ArrowPosition")
check(NSColorPanel.Mode.RGB.rawValue == 1 && NSColorPanel.Mode.none.rawValue == -1, "NSColorPanel.Mode")

// Tables, text and drawing.
check(NSTableView.SelectionHighlightStyle.none.rawValue == -1, "NSTableView.SelectionHighlightStyle.none")
check(NSTableView.SelectionHighlightStyle.sourceList.rawValue == 1, "NSTableView.SelectionHighlightStyle.sourceList")
check(NSTableView.DropOperation.on.rawValue == 0 && NSTableView.DropOperation.above.rawValue == 1, "NSTableView.DropOperation")
let gridLines: NSTableView.GridLineStyle = [.solidVerticalGridLineMask, .solidHorizontalGridLineMask]
check(gridLines.rawValue == 3, "NSTableView.GridLineStyle")
check(NSTableColumn.ResizingOptions.autoresizingMask.rawValue == 1, "NSTableColumn.ResizingOptions")
check(NSTextView.SelectionGranularity.selectByWord.rawValue == 1, "NSTextView.SelectionGranularity.selectByWord")
check(NSTextView.SelectionAffinity.downstream.rawValue == 1, "NSTextView.SelectionAffinity.downstream")
check(NSBezierPath.LineCapStyle.round.rawValue == 1, "NSBezierPath.LineCapStyle.round")
check(NSBezierPath.LineJoinStyle.bevel.rawValue == 2, "NSBezierPath.LineJoinStyle.bevel")
check(NSBezierPath.WindingRule.evenOdd.rawValue == 1, "NSBezierPath.WindingRule.evenOdd")
check(NSBezierPath.ElementType.curveTo.rawValue == 2, "NSBezierPath.ElementType.curveTo")
check(NSBitmapImageRep.FileType.png.rawValue == 4, "NSBitmapImageRep.FileType.png")
check(NSBitmapImageRep.FileType.jpeg.rawValue == 3, "NSBitmapImageRep.FileType.jpeg")
check(NSBitmapImageRep.FileType.tiff.rawValue == 0, "NSBitmapImageRep.FileType.tiff")
check(NSBitmapImageRep.TIFFCompression.lzw.rawValue == 5, "NSBitmapImageRep.TIFFCompression.lzw")
check(NSGradient.DrawingOptions.drawsAfterEndingLocation.rawValue == 2, "NSGradient.DrawingOptions")
check(NSDocument.SaveOperationType.saveAsOperation.rawValue == 1, "NSDocument.SaveOperationType")
check(NSAnimation.Curve.easeInOut.rawValue == 0, "NSAnimation.Curve.easeInOut")
check(NSDatePicker.Mode.range.rawValue == 1, "NSDatePicker.Mode.range")
check(NSDatePicker.Style.clockAndCalendar.rawValue == 1, "NSDatePicker.Style.clockAndCalendar")
check(NSDatePicker.ElementFlags.hourMinute.rawValue == 0xc, "NSDatePicker.ElementFlags.hourMinute")
check(NSOpenGLContext.Parameter.swapInterval.rawValue == 222, "NSOpenGLContext.Parameter.swapInterval")
check(NSPrintPanel.Options.showsPreview.rawValue == 1 << 17, "NSPrintPanel.Options.showsPreview")
check(NSRulerView.Orientation.verticalRuler.rawValue == 1, "NSRulerView.Orientation.verticalRuler")
let tracking: NSTrackingArea.Options = [.mouseEnteredAndExited, .activeAlways, .inVisibleRect]
check(tracking.rawValue == 1 | (1 << 7) | (1 << 9), "NSTrackingArea.Options")
let drag: NSDragOperation = [.copy, .move]
check(drag.rawValue == 0x11, "NSDragOperation")

// Layout.
check(NSLayoutConstraint.Attribute.leading.rawValue == 5, "NSLayoutConstraint.Attribute.leading")
check(NSLayoutConstraint.Relation.equal.rawValue == 0, "NSLayoutConstraint.Relation.equal")
check(NSLayoutConstraint.FormatOptions.alignAllTop.rawValue == 1 << 3, "NSLayoutConstraint.FormatOptions.alignAllTop")
check(NSStackView.Distribution.fill.rawValue == 0, "NSStackView.Distribution.fill")
check(NSVisualEffectView.BlendingMode.behindWindow.rawValue == 0, "NSVisualEffectView.BlendingMode.behindWindow")

// Workspace.
check(NSWorkspace.LaunchOptions.async.rawValue == 0x10000, "NSWorkspace.LaunchOptions.async")
check(NSWorkspace.IconCreationOptions.excludeQuickDrawElements.rawValue == 1 << 1, "NSWorkspace.IconCreationOptions")

if failures == 0 {
    print("ALL PASSED")
} else {
    exit(1)
}
