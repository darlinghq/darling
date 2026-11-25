// Copyright (C) 2026 Lubos Dolezel
//
// The Interface Builder Carbon runtime: nib references, and the menus and the menu bar of a Carbon
// nib (a directory with an objects.xib, see the Interface Builder Carbon nib format). The nib is
// written by the test, in a bundle of its own with a localized resource, from a hand-written
// description of a small interface; nothing in it comes from an application.
//
// CreateWindowFromNib reads the window, but making it and its controls is up to the Window Manager:
// until it provides that, the call fails with unimpErr. The test accepts any status other than the
// two that say the object is not there or not a window (a process with no window server gets another
// error from macOS, too).
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long long _a = (long long) (a), _b = (long long) (b); if (_a != _b) { printf("FAIL line %d: %s = %lld, expected %lld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static const char kObjects[] =
	"<?xml version=\"1.0\" standalone=\"yes\"?>\n"
	"<object class=\"NSIBObjectData\">\n"
	"  <string name=\"targetFramework\">IBCarbonFramework</string>\n"
	"  <object name=\"rootObject\" class=\"NSCustomObject\" id=\"1\">\n"
	"    <string name=\"customClass\">NSApplication</string>\n"
	"  </object>\n"
	"  <array count=\"4\" name=\"allObjects\">\n"
	"    <object class=\"IBCarbonMenu\" id=\"30\">\n"
	"      <string name=\"title\">Find</string>\n"
	"      <int name=\"menuID\">201</int>\n"
	"      <array count=\"1\" name=\"items\">\n"
	"        <object class=\"IBCarbonMenuItem\" id=\"31\">\n"
	"          <string name=\"title\">Next</string>\n"
	"          <ostype name=\"command\">fnxt</ostype>\n"
	"        </object>\n"
	"      </array>\n"
	"    </object>\n"
	"    <object class=\"IBCarbonMenu\" id=\"10\">\n"
	"      <string name=\"title\">Bar</string>\n"
	"      <array count=\"2\" name=\"items\">\n"
	"        <object class=\"IBCarbonMenuItem\" id=\"11\">\n"
	"          <string name=\"title\">File</string>\n"
	"          <object name=\"submenu\" class=\"IBCarbonMenu\" id=\"12\">\n"
	"            <string name=\"title\">File</string>\n"
	"            <int name=\"menuID\">301</int>\n"
	"            <array count=\"6\" name=\"items\">\n"
	"              <object class=\"IBCarbonMenuItem\" id=\"13\">\n"
	"                <string name=\"title\">New</string>\n"
	"                <string name=\"keyEquivalent\">n</string>\n"
	"                <ostype name=\"command\">new </ostype>\n"
	"              </object>\n"
	"              <object class=\"IBCarbonMenuItem\" id=\"14\">\n"
	"                <boolean name=\"separator\">TRUE</boolean>\n"
	"              </object>\n"
	"              <object class=\"IBCarbonMenuItem\" id=\"15\">\n"
	"                <string name=\"title\">Close</string>\n"
	"                <string name=\"keyEquivalent\">w</string>\n"
	"                <int name=\"keyEquivalentModifier\">1048576</int>\n"
	"                <ostype name=\"command\">clos</ostype>\n"
	"              </object>\n"
	"              <object class=\"IBCarbonMenuItem\" id=\"16\">\n"
	"                <string name=\"title\">Save &amp; Share&#8230;</string>\n"
	"                <string name=\"keyEquivalent\">S</string>\n"
	"                <int name=\"keyEquivalentModifier\">1179648</int>\n"
	"                <ostype name=\"command\">shar</ostype>\n"
	"              </object>\n"
	"              <object class=\"IBCarbonMenuItem\" id=\"17\">\n"
	"                <boolean name=\"disabled\">TRUE</boolean>\n"
	"                <boolean name=\"checked\">TRUE</boolean>\n"
	"                <string name=\"title\">Revert</string>\n"
	"                <ostype name=\"command\">rvrt</ostype>\n"
	"              </object>\n"
	"              <object class=\"IBCarbonMenuItem\" id=\"18\">\n"
	"                <string name=\"title\">Hide</string>\n"
	"                <string name=\"keyEquivalent\">h</string>\n"
	"                <int name=\"keyEquivalentModifier\">1572864</int>\n"
	"              </object>\n"
	"            </array>\n"
	"          </object>\n"
	"        </object>\n"
	"        <object class=\"IBCarbonMenuItem\" id=\"19\">\n"
	"          <string name=\"title\">Edit</string>\n"
	"          <object name=\"submenu\" class=\"IBCarbonMenu\" id=\"20\">\n"
	"            <string name=\"title\">Edit</string>\n"
	"            <array count=\"2\" name=\"items\">\n"
	"              <object class=\"IBCarbonMenuItem\" id=\"21\">\n"
	"                <string name=\"title\">Find</string>\n"
	"                <reference name=\"submenu\" idRef=\"30\"/>\n"
	"              </object>\n"
	"              <object class=\"IBCarbonMenuItem\" id=\"22\">\n"
	"                <string name=\"title\">Raw</string>\n"
	"                <string name=\"keyEquivalent\">r</string>\n"
	"                <int name=\"keyEquivalentModifier\">131072</int>\n"
	"              </object>\n"
	"            </array>\n"
	"          </object>\n"
	"        </object>\n"
	"      </array>\n"
	"    </object>\n"
	"    <object class=\"IBCarbonWindow\" id=\"40\">\n"
	"      <string name=\"title\">Fixture</string>\n"
	"      <string name=\"windowRect\">50 60 250 360</string>\n"
	"      <int name=\"carbonWindowClass\">5</int>\n"
	"      <boolean name=\"isResizable\">TRUE</boolean>\n"
	"    </object>\n"
	"  </array>\n"
	"  <dictionary count=\"4\" name=\"nameTable\">\n"
	"    <string>Files Owner</string>\n"
	"    <reference idRef=\"1\"/>\n"
	"    <string>MenuBar</string>\n"
	"    <reference idRef=\"10\"/>\n"
	"    <string>Popup</string>\n"
	"    <reference idRef=\"30\"/>\n"
	"    <string>Window</string>\n"
	"    <reference idRef=\"40\"/>\n"
	"  </dictionary>\n"
	"  <unsigned_int name=\"nextObjectID\">50</unsigned_int>\n"
	"</object>\n";

static void writeFile(const char* path, const char* contents)
{
	FILE* file = fopen(path, "w");

	if (!file || fwrite(contents, 1, strlen(contents), file) != strlen(contents))
		printf("FAIL: cannot write %s\n", path);
	if (file)
		fclose(file);
}

static CFStringRef cf(const char* s)
{
	return CFStringCreateWithCString(NULL, s, kCFStringEncodingUTF8);
}

static CFBundleRef makeBundle(const char* root, const char* nibName, const char* objects)
{
	char path[1024];

	snprintf(path, sizeof(path), "%s/Fixture.app", root);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents", root);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents/Info.plist", root);
	writeFile(path, "<?xml version=\"1.0\"?><plist version=\"1.0\"><dict><key>CFBundleIdentifier</key><string>test.nibfixture</string>"
		"<key>CFBundleDevelopmentRegion</key><string>en</string><key>CFBundleName</key><string>Fixture</string></dict></plist>");
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents/Resources", root);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents/Resources/en.lproj", root);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents/Resources/en.lproj/%s.nib", root, nibName);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents/Resources/en.lproj/%s.nib/objects.xib", root, nibName);
	writeFile(path, objects);

	snprintf(path, sizeof(path), "%s/Fixture.app", root);
	CFStringRef string = cf(path);
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, string, kCFURLPOSIXPathStyle, true);
	CFBundleRef bundle = CFBundleCreate(NULL, url);

	CFRelease(url);
	CFRelease(string);
	return bundle;
}

static void checkItem(MenuRef menu, MenuItemIndex index, const char* title, MenuCommand command, UInt16 key, UInt8 modifiers)
{
	CFStringRef text = NULL;
	CFStringRef expected = cf(title);
	MenuCommand actualCommand = 99;
	UInt16 actualKey = 99;
	UInt8 actualModifiers = 99;

	CHECK_EQ(CopyMenuItemTextAsCFString(menu, index, &text), noErr);
	CHECK(text && CFEqual(text, expected));
	if (!text || !CFEqual(text, expected))
		CFShow(text);
	CHECK_EQ(GetMenuItemCommandID(menu, index, &actualCommand), noErr);
	CHECK_EQ(actualCommand, command);
	CHECK_EQ(GetMenuItemCommandKey(menu, index, false, &actualKey), noErr);
	CHECK_EQ(actualKey, key);
	CHECK_EQ(GetMenuItemModifiers(menu, index, &actualModifiers), noErr);
	CHECK_EQ(actualModifiers, modifiers);
	if (text)
		CFRelease(text);
	CFRelease(expected);
}

static void test_nib(CFBundleRef bundle)
{
	IBNibRef nib = NULL;

	// Nibs that do not exist
	CHECK_EQ(CreateNibReferenceWithCFBundle(bundle, CFSTR("NoSuchNib"), &nib), kIBCarbonRuntimeCantFindNibFile);
	CHECK(nib == NULL);

	// The nib is found in the localized resources, by its name without the extension
	IBNibRef second = NULL;
	CHECK_EQ(CreateNibReferenceWithCFBundle(bundle, CFSTR("Fixture.nib"), &second), kIBCarbonRuntimeCantFindNibFile);
	CHECK_EQ(CreateNibReferenceWithCFBundle(bundle, CFSTR("Fixture"), &nib), noErr);
	CHECK(nib != NULL);

	// Objects that are not there, and of another kind
	MenuRef menu = (MenuRef) 1;
	CHECK_EQ(CreateMenuFromNib(nib, CFSTR("Nope"), &menu), kIBCarbonRuntimeCantFindObject);
	CHECK(menu == NULL);
	CHECK_EQ(CreateMenuFromNib(nib, CFSTR("Window"), &menu), kIBCarbonRuntimeObjectNotOfRequestedType);
	CHECK_EQ(SetMenuBarFromNib(nib, CFSTR("Nope")), kIBCarbonRuntimeCantFindObject);
	CHECK_EQ(SetMenuBarFromNib(nib, CFSTR("Window")), kIBCarbonRuntimeObjectNotOfRequestedType);

	// A menu by name
	CHECK_EQ(CreateMenuFromNib(nib, CFSTR("Popup"), &menu), noErr);
	CHECK(menu != NULL);
	CHECK_EQ(GetMenuID(menu), 201);
	CHECK_EQ(CountMenuItems(menu), 1);
	checkItem(menu, 1, "Next", 'fnxt', 0, 0);
	CFRelease((CFTypeRef) menu);

	// The menu bar: the submenus of the items of the menu named MenuBar. The ID is the one of the nib,
	// 0 for a menu that has none.
	ClearMenuBar();
	CHECK_EQ(SetMenuBarFromNib(nib, CFSTR("MenuBar")), noErr);
	MenuRef file = GetMenuHandle(301);

	CHECK(file != NULL);
	if (file)
	{
		CHECK_EQ(CountMenuItems(file), 6);
		CFStringRef title = NULL;
		CopyMenuTitleAsCFString(file, &title);
		CHECK(title && CFEqual(title, CFSTR("File")));
		if (title)
			CFRelease(title);
		// (the key of an item of a nib is the capital letter)
		checkItem(file, 1, "New", 'new ', 'N', 0);
		MenuItemAttributes attributes = 0;
		GetMenuItemAttributes(file, 2, &attributes);
		CHECK(attributes & kMenuItemAttrSeparator);
		// The Command flag of the modifiers is the default; Shift and Option come as modifiers
		checkItem(file, 3, "Close", 'clos', 'W', 0);
		checkItem(file, 4, "Save & Share\xE2\x80\xA6", 'shar', 'S', kMenuShiftModifier);
		checkItem(file, 6, "Hide", 0, 'H', kMenuOptionModifier);
		// disabled and checked
		CHECK(!IsMenuItemEnabled(file, 5));
		UInt16 mark = 0;
		GetItemMark(file, 5, &mark);
		CHECK_EQ(mark, checkMark);
		CHECK(IsMenuItemEnabled(file, 1));
	}

	MenuRef root = AcquireRootMenu();

	CHECK_EQ(CountMenuItems(root), 2);
	MenuRef edit = NULL;
	CHECK_EQ(GetMenuItemHierarchicalMenu(root, 2, &edit), noErr);
	CHECK(edit != NULL);
	if (edit)
	{
		CHECK_EQ(GetMenuID(edit), 0);
		// Shift without Command: no Command key
		checkItem(edit, 2, "Raw", 0, 'R', kMenuShiftModifier | kMenuNoCommandModifier);
		// A submenu by reference
		MenuRef find = NULL;
		CHECK_EQ(GetMenuItemHierarchicalMenu(edit, 1, &find), noErr);
		CHECK(find != NULL);
		if (find)
		{
			CHECK_EQ(GetMenuID(find), 201);
			checkItem(find, 1, "Next", 'fnxt', 0, 0);
		}
		// A command of a submenu is found from the menu bar
		MenuRef found = NULL;
		MenuItemIndex index = 0;
		CHECK_EQ(GetIndMenuItemWithCommandID(NULL, 'fnxt', 1, &found, &index), noErr);
		CHECK(found == find && index == 1);
	}
	CFRelease((CFTypeRef) root);

	// Replacing the menu bar by the same nib again leaves one set of menus
	CHECK_EQ(SetMenuBarFromNib(nib, CFSTR("MenuBar")), noErr);
	root = AcquireRootMenu();
	CHECK_EQ(CountMenuItems(root), 2);
	CFRelease((CFTypeRef) root);

	// The window: its class, attributes, bounds and title come from the nib (this needs a window system)
	WindowRef window = (WindowRef) 1;
	OSStatus status = CreateWindowFromNib(nib, CFSTR("Window"), &window);
	CHECK(status != kIBCarbonRuntimeCantFindObject && status != kIBCarbonRuntimeObjectNotOfRequestedType);
	if (status != noErr)
		CHECK(window == NULL);
	else
	{
		WindowClass windowClass = 0;
		WindowAttributes attributes = 0;
		Rect content;
		CFStringRef title = NULL;

		CHECK_EQ(GetWindowClass(window, &windowClass), noErr);
		CHECK_EQ(windowClass, kFloatingWindowClass);
		CHECK_EQ(GetWindowAttributes(window, &attributes), noErr);
		CHECK(attributes & kWindowResizableAttribute);
		CHECK(!(attributes & kWindowCloseBoxAttribute));
		CHECK_EQ(GetWindowBounds(window, kWindowContentRgn, &content), noErr);
		CHECK(content.top == 50 && content.left == 60 && content.bottom == 250 && content.right == 360);
		CHECK_EQ(CopyWindowTitleAsCFString(window, &title), noErr);
		CHECK(title && CFStringCompare(title, CFSTR("Fixture"), 0) == kCFCompareEqualTo);
		if (title)
			CFRelease(title);
		CHECK(!IsWindowVisible(window));
		DisposeWindow(window);
	}
	CHECK_EQ(CreateWindowFromNib(nib, CFSTR("Popup"), &window), kIBCarbonRuntimeObjectNotOfRequestedType);
	CHECK_EQ(CreateWindowFromNib(nib, CFSTR("Nope"), &window), kIBCarbonRuntimeCantFindObject);

#if !__LP64__
	Handle bar = NULL;

	ClearMenuBar();
	CHECK_EQ(CreateMenuBarFromNib(nib, CFSTR("MenuBar"), &bar), noErr);
	CHECK(bar != NULL);
	CHECK(GetMenuHandle(301) == NULL);
	SetMenuBar(bar);
	CHECK(GetMenuHandle(301) != NULL);
	CHECK_EQ(CreateMenuBarFromNib(nib, CFSTR("Nope"), &bar), kIBCarbonRuntimeCantFindObject);
#endif

	ClearMenuBar();
	DisposeNibReference(nib);
	DisposeNibReference(NULL);
}

int main(void)
{
	char root[] = "/tmp/carbon_menus_nib.XXXXXX";

	setvbuf(stdout, NULL, _IONBF, 0);
	if (!mkdtemp(root))
	{
		printf("FAIL: no temporary directory\n");
		return 1;
	}
	CFBundleRef bundle = makeBundle(root, "Fixture", kObjects);

	CHECK(bundle != NULL);
	if (bundle)
		test_nib(bundle);

	// A nib that is not well formed opens, and has no objects
	char bad[1024];

	snprintf(bad, sizeof(bad), "%s/bad", root);
	mkdir(bad, 0755);
	CFBundleRef broken = makeBundle(bad, "Broken", "<object class=\"NSIBObjectData\"><array name=\"allObjects\"><object");
	IBNibRef nib = NULL;

	CHECK(broken != NULL);
	if (broken)
		CHECK_EQ(CreateNibReferenceWithCFBundle(broken, CFSTR("Broken"), &nib), noErr);
	if (nib)
	{
		MenuRef menu = NULL;

		CHECK(CreateMenuFromNib(nib, CFSTR("MenuBar"), &menu) != noErr);
		CHECK(menu == NULL);
		DisposeNibReference(nib);
	}

	if (g_failures)
	{
		printf("FAIL: %d checks failed\n", g_failures);
		return 1;
	}
	printf("PASS carbon_menus_nib (%s)\n", sizeof(void*) == 8 ? "x86_64" : "i386");
	return 0;
}
