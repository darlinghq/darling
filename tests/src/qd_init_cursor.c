// Copyright (C) 2026 Lubos Dolezel
//
// The QuickDraw cursor calls HideCursor, ShowCursor and InitCursor link and can be called in
// any order without a window server connection, including InitCursor undoing nested hides.
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>

int main(void)
{
    HideCursor();
    HideCursor();
    ShowCursor();
    InitCursor();
    InitCursor();
    ShowCursor();
    printf("PASS\n");
    return 0;
}
