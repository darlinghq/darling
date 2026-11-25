/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <HIToolbox/MacWindows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
static int failures,checks;
#define CHECK(x) do { ++checks; if (!(x)) { ++failures; printf("FAIL line=%d %s\n",__LINE__,#x); } } while(0)
int main(void) {
 Rect r={10,20,100,150},bad={10,30,10,20},empty={10,20,10,150};
 WindowRef sentinel=(WindowRef)(uintptr_t)0x1234,w=sentinel;
 OSStatus status=CreateNewWindow(13,0,&r,&w);
 CHECK(status==unimpErr); CHECK(w==NULL);
 CHECK(CreateNewWindow(13,0,&r,NULL)==paramErr);
 w=sentinel;CHECK(CreateNewWindow(13,0,NULL,&w)==paramErr);CHECK(w==NULL);
 w=sentinel;CHECK(CreateNewWindow(13,0,&bad,&w)==paramErr);CHECK(w==NULL);
 w=sentinel;CHECK(CreateNewWindow(13,0,&empty,&w)==unimpErr);CHECK(w==NULL);
 CHECK(!IsValidWindowPtr(NULL));CHECK(!IsWindowVisible(NULL));
 DisposeWindow(NULL);ShowWindow(NULL);HideWindow(NULL);ShowHide(NULL,1);
 printf("WINDOW_CONTRACT bits=%zu checks=%d failures=%d\n",sizeof(void*)*8,checks,failures);
 return failures!=0;
}
