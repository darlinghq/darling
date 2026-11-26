/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Xlib's input method machinery keys on the locale of the Xlib library itself. A Mach-O program
 * that links the libX11 wrapper (/usr/lib/native/libX11.dylib) must be able to receive composed
 * text, dead-key and compose sequences and an input method's committed text through
 * Xutf8LookupString(), whatever its own (Darwin) locale is. The program opens the input method the
 * way an Xlib client does (XSetLocaleModifiers(""), XOpenIM, XCreateIC), puts a focused window up
 * and reports the text that Xutf8LookupString() returns for the key presses it is sent.
 *
 *   native-x11-locale EXPECTED [SECONDS]
 *
 * prints READY once the window has the focus, then PASS when the text EXPECTED (UTF-8) was
 * delivered by one key press, FAIL otherwise. Needs an X server ($DISPLAY); see
 * native-x11-locale.sh, which feeds the keys with xdotool.
 *
 * Xlib's headers are not part of the SDK, so the few types and functions used are declared here.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct _XDisplay Display;
typedef void *XIM;
typedef void *XIC;
typedef unsigned long Window;
typedef unsigned long KeySym;
typedef struct {
    int type;
    unsigned long serial;
    int send_event;
    Display *display;
    Window window, root, subwindow;
    unsigned long time;
    int x, y, x_root, y_root;
    unsigned int state, keycode;
    int same_screen;
} XKeyEvent;
typedef union { int type; XKeyEvent xkey; long pad[24]; } XEvent;

#define KeyPress 2
#define KeyPressMask 1L
#define XIMPreeditNothing 0x0008L
#define XIMStatusNothing 0x0400L
#define RevertToParent 2
#define XBufferOverflow (-1)
#define XLookupChars 2
#define XLookupBoth 4

extern Display *XOpenDisplay(const char *);
extern char *XSetLocaleModifiers(const char *);
extern XIM XOpenIM(Display *, void *, char *, char *);
extern XIC XCreateIC(XIM, ...);
extern void XSetICFocus(XIC);
extern Window XCreateSimpleWindow(Display *, Window, int, int, unsigned, unsigned, unsigned, unsigned long,
                                  unsigned long);
extern int XSelectInput(Display *, Window, long);
extern int XMapWindow(Display *, Window);
extern int XSetInputFocus(Display *, Window, int, unsigned long);
extern int XFlush(Display *);
extern int XSync(Display *, int);
extern int XPending(Display *);
extern int XNextEvent(Display *, XEvent *);
extern int XFilterEvent(XEvent *, Window);
extern int Xutf8LookupString(XIC, XEvent *, char *, int, KeySym *, int *);

extern Window XDefaultRootWindow(Display *);

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s EXPECTED [SECONDS]\n", argv[0]);
        return 2;
    }
    const char *expected = argv[1];
    int seconds = argc > 2 ? atoi(argv[2]) : 15;

    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("FAIL: cannot open the display\n");
        return 1;
    }
    if (XSetLocaleModifiers("") == NULL)
        printf("note: XSetLocaleModifiers(\"\") failed\n");
    XIM im = XOpenIM(display, NULL, NULL, NULL);
    if (im == NULL) {
        printf("FAIL: XOpenIM failed\n");
        return 1;
    }
    Window window = XCreateSimpleWindow(display, XDefaultRootWindow(display), 0, 0, 100, 100, 0, 0, 0);
    XSelectInput(display, window, KeyPressMask);
    XMapWindow(display, window);
    XIC ic = XCreateIC(im, "inputStyle", XIMPreeditNothing | XIMStatusNothing, "clientWindow", window,
                       "focusWindow", window, NULL);
    if (ic == NULL) {
        printf("FAIL: XCreateIC failed\n");
        return 1;
    }
    XSetICFocus(ic);
    XSetInputFocus(display, window, RevertToParent, 0);
    XSync(display, 0);
    printf("input method modifiers: %s\n", getenv("XMODIFIERS") ? getenv("XMODIFIERS") : "(none)");
    puts("READY");
    fflush(stdout);

    int found = 0;
    time_t end = time(NULL) + seconds;
    while (time(NULL) < end && !found) {
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);
            if (XFilterEvent(&event, 0))
                continue;
            if (event.type != KeyPress)
                continue;
            char text[64];
            KeySym symbol = 0;
            int status = 0;
            int length = Xutf8LookupString(ic, &event, text, sizeof text - 1, &symbol, &status);
            if (length < 0)
                length = 0;
            text[length] = '\0';
            printf("key press: keycode %u state 0x%x keysym 0x%lx status %d text \"%s\"\n", event.xkey.keycode,
                   event.xkey.state, symbol, status, text);
            if ((status == XLookupChars || status == XLookupBoth) && strcmp(text, expected) == 0)
                found = 1;
        }
        usleep(20000);
    }
    puts(found ? "PASS" : "FAIL: the expected text was never delivered");
    return found ? 0 : 1;
}
