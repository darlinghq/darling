// Copyright (C) 2026 Lubos Dolezel
//
// A dylib that imports a set of system symbols, as an injected game overlay
// library built on macOS does: OpenGL/CGL, CoreGraphics (including the private
// CGS calls), IOSurface, Metal, CoreMedia, ApplicationServices cursor calls,
// AppKit/Foundation classes, libobjc, libc++ and libSystem. The test dlopen()s
// the dylib and checks that dyld resolves every import (a missing symbol makes
// dlopen fail with "Symbol not found").
//
// Build: the dylib from this file with -DBUILD_DYLIB, the loader from this
// file without it (see main()), e.g.
//   cc -dynamiclib -DBUILD_DYLIB -o libimports.dylib t.c <frameworks> -lobjc -lc++
//   cc -o t t.c
// The import list is the dylib's list of undefined symbols as printed by nm;
// each is referenced from a data table, so dyld binds it when the dylib loads.
#ifdef BUILD_DYLIB
extern char imp0 __asm__("_CFAllocatorAllocate");
extern char imp1 __asm__("_CFAllocatorDeallocate");
extern char imp2 __asm__("_CFRelease");
extern char imp3 __asm__("_CFRetain");
extern char imp4 __asm__("_CFRunLoopRun");
extern char imp5 __asm__("_CFRunLoopRunInMode");
extern char imp6 __asm__("_CFStringHasSuffix");
extern char imp7 __asm__("_CGAssociateMouseAndMouseCursorPosition");
extern char imp8 __asm__("_CGColorSpaceCopyName");
extern char imp9 __asm__("_CGCursorIsVisible");
extern char imp10 __asm__("_CGDisplayBounds");
extern char imp11 __asm__("_CGDisplayHideCursor");
extern char imp12 __asm__("_CGDisplayIsCaptured");
extern char imp13 __asm__("_CGDisplayMoveCursorToPoint");
extern char imp14 __asm__("_CGDisplayPixelsHigh");
extern char imp15 __asm__("_CGDisplayPixelsWide");
extern char imp16 __asm__("_CGDisplayShowCursor");
extern char imp17 __asm__("_CGEventCreate");
extern char imp18 __asm__("_CGEventCreateKeyboardEvent");
extern char imp19 __asm__("_CGEventGetFlags");
extern char imp20 __asm__("_CGEventGetIntegerValueField");
extern char imp21 __asm__("_CGEventGetLocation");
extern char imp22 __asm__("_CGEventGetType");
extern char imp23 __asm__("_CGEventKeyboardGetUnicodeString");
extern char imp24 __asm__("_CGEventKeyboardSetUnicodeString");
extern char imp25 __asm__("_CGEventPost");
extern char imp26 __asm__("_CGEventSetFlags");
extern char imp27 __asm__("_CGGetActiveDisplayList");
extern char imp28 __asm__("_CGGetDisplaysWithPoint");
extern char imp29 __asm__("_CGGetLastMouseDelta");
extern char imp30 __asm__("_CGLChoosePixelFormat");
extern char imp31 __asm__("_CGLErrorString");
extern char imp32 __asm__("_CGLFlushDrawable");
extern char imp33 __asm__("_CGLGetCurrentContext");
extern char imp34 __asm__("_CGLGetParameter");
extern char imp35 __asm__("_CGLGetSurface");
extern char imp36 __asm__("_CGLIsEnabled");
extern char imp37 __asm__("_CGLSetCurrentContext");
extern char imp38 __asm__("_CGLTexImageIOSurface2D");
extern char imp39 __asm__("_CGMainDisplayID");
extern char imp40 __asm__("_CGSForceShowCursor");
extern char imp41 __asm__("_CGSGetSurfaceBounds");
extern char imp42 __asm__("_CGSGetWindowBounds");
extern char imp43 __asm__("_CGWarpMouseCursorPosition");
extern char imp44 __asm__("_CMAudioFormatDescriptionGetStreamBasicDescription");
extern char imp45 __asm__("_CMBlockBufferGetDataPointer");
extern char imp46 __asm__("_CMSampleBufferGetDataBuffer");
extern char imp47 __asm__("_CMSampleBufferGetFormatDescription");
extern char imp48 __asm__("_HideCursor");
extern char imp49 __asm__("_IOSurfaceGetHeight");
extern char imp50 __asm__("_IOSurfaceGetWidth");
extern char imp51 __asm__("_IOSurfaceLookup");
extern char imp52 __asm__("_InitCursor");
extern char imp53 __asm__("_MTLCopyAllDevices");
extern char imp54 __asm__("_MTLCreateSystemDefaultDevice");
extern char imp55 __asm__("_NSApp");
extern char imp56 __asm__("_NSClassFromString");
extern char imp57 __asm__("_NSLog");
extern char imp58 __asm__("_NSSelectorFromString");
extern char imp59 __asm__("_NSStringFromSelector");
extern char imp60 __asm__("_OBJC_CLASS_$_MTLRenderPassDescriptor");
extern char imp61 __asm__("_OBJC_CLASS_$_MTLRenderPipelineDescriptor");
extern char imp62 __asm__("_OBJC_CLASS_$_MTLTextureDescriptor");
extern char imp63 __asm__("_OBJC_CLASS_$_NSApplication");
extern char imp64 __asm__("_OBJC_CLASS_$_NSCursor");
extern char imp65 __asm__("_OBJC_CLASS_$_NSDate");
extern char imp66 __asm__("_OBJC_CLASS_$_NSEvent");
extern char imp67 __asm__("_OBJC_CLASS_$_NSLock");
extern char imp68 __asm__("_OBJC_CLASS_$_NSMapTable");
extern char imp69 __asm__("_OBJC_CLASS_$_NSNumber");
extern char imp70 __asm__("_OBJC_CLASS_$_NSObject");
extern char imp71 __asm__("_OBJC_CLASS_$_NSOpenGLContext");
extern char imp72 __asm__("_OBJC_CLASS_$_NSOpenGLView");
extern char imp73 __asm__("_OBJC_CLASS_$_NSScreen");
extern char imp74 __asm__("_OBJC_CLASS_$_NSString");
extern char imp75 __asm__("_OBJC_CLASS_$_NSThread");
extern char imp76 __asm__("_OBJC_METACLASS_$_NSObject");
extern char imp77 __asm__("_ShowCursor");
extern char imp78 __asm__("__Block_object_assign");
extern char imp79 __asm__("__Block_object_dispose");
extern char imp80 __asm__("__CGSDefaultConnection");
extern char imp81 __asm__("__DefaultRuneLocale");
extern char imp82 __asm__("__NSConcreteGlobalBlock");
extern char imp83 __asm__("__NSConcreteStackBlock");
extern char imp84 __asm__("__NSGetExecutablePath");
extern char imp85 __asm__("__Unwind_Resume");
extern char imp86 __asm__("__ZNSt11logic_errorC2EPKc");
extern char imp87 __asm__("__ZNSt12length_errorD1Ev");
extern char imp88 __asm__("__ZNSt20bad_array_new_lengthC1Ev");
extern char imp89 __asm__("__ZNSt20bad_array_new_lengthD1Ev");
extern char imp90 __asm__("__ZSt9terminatev");
extern char imp91 __asm__("__ZTISt12length_error");
extern char imp92 __asm__("__ZTISt20bad_array_new_length");
extern char imp93 __asm__("__ZTVN10__cxxabiv117__class_type_infoE");
extern char imp94 __asm__("__ZTVN10__cxxabiv120__si_class_type_infoE");
extern char imp95 __asm__("__ZTVSt12length_error");
extern char imp96 __asm__("___CFConstantStringClassReference");
extern char imp97 __asm__("____chkstk_darwin");
extern char imp98 __asm__("___bzero");
extern char imp99 __asm__("___cxa_allocate_exception");
extern char imp100 __asm__("___cxa_atexit");
extern char imp101 __asm__("___cxa_begin_catch");
extern char imp102 __asm__("___cxa_free_exception");
extern char imp103 __asm__("___cxa_pure_virtual");
extern char imp104 __asm__("___cxa_throw");
extern char imp105 __asm__("___error");
extern char imp106 __asm__("___gxx_personality_v0");
extern char imp107 __asm__("___stack_chk_fail");
extern char imp108 __asm__("___stack_chk_guard");
extern char imp109 __asm__("___stderrp");
extern char imp110 __asm__("__dispatch_main_q");
extern char imp111 __asm__("__dyld_get_image_name");
extern char imp112 __asm__("__objc_empty_cache");
extern char imp113 __asm__("_atoi");
extern char imp114 __asm__("_bootstrap_look_up");
extern char imp115 __asm__("_bootstrap_port");
extern char imp116 __asm__("_calloc");
extern char imp117 __asm__("_class_addMethod");
extern char imp118 __asm__("_class_copyMethodList");
extern char imp119 __asm__("_class_getClassMethod");
extern char imp120 __asm__("_class_getInstanceMethod");
extern char imp121 __asm__("_class_getName");
extern char imp122 __asm__("_class_getSuperclass");
extern char imp123 __asm__("_class_replaceMethod");
extern char imp124 __asm__("_clock_gettime");
extern char imp125 __asm__("_close");
extern char imp126 __asm__("_connect");
extern char imp127 __asm__("_dispatch_after");
extern char imp128 __asm__("_dispatch_async");
extern char imp129 __asm__("_dispatch_time");
extern char imp130 __asm__("_dlopen");
extern char imp131 __asm__("_dlsym");
extern char imp132 __asm__("_dup");
extern char imp133 __asm__("_fclose");
extern char imp134 __asm__("_fcntl");
extern char imp135 __asm__("_fflush");
extern char imp136 __asm__("_fopen");
extern char imp137 __asm__("_fprintf");
extern char imp138 __asm__("_free");
extern char imp139 __asm__("_ftruncate");
extern char imp140 __asm__("_fwrite");
extern char imp141 __asm__("_getenv");
extern char imp142 __asm__("_getpid");
extern char imp143 __asm__("_getrlimit");
extern char imp144 __asm__("_getuid");
extern char imp145 __asm__("_glBegin");
extern char imp146 __asm__("_glBindTexture");
extern char imp147 __asm__("_glColorMask");
extern char imp148 __asm__("_glDeleteTextures");
extern char imp149 __asm__("_glDisable");
extern char imp150 __asm__("_glDrawArrays");
extern char imp151 __asm__("_glEnable");
extern char imp152 __asm__("_glEnd");
extern char imp153 __asm__("_glGetBooleanv");
extern char imp154 __asm__("_glGetError");
extern char imp155 __asm__("_glGetIntegerv");
extern char imp156 __asm__("_glIsEnabled");
extern char imp157 __asm__("_glPixelStorei");
extern char imp158 __asm__("_glReadPixels");
extern char imp159 __asm__("_glSwapAPPLE");
extern char imp160 __asm__("_glTexCoord2i");
extern char imp161 __asm__("_glTexImage2D");
extern char imp162 __asm__("_glTexParameteri");
extern char imp163 __asm__("_glVertex2f");
extern char imp164 __asm__("_glViewport");
extern char imp165 __asm__("_gmtime_r");
extern char imp166 __asm__("_imp_implementationWithBlock");
extern char imp167 __asm__("_launch_data_alloc");
extern char imp168 __asm__("_launch_data_dict_insert");
extern char imp169 __asm__("_launch_data_free");
extern char imp170 __asm__("_launch_data_new_string");
extern char imp171 __asm__("_launch_msg");
extern char imp172 __asm__("_lround");
extern char imp173 __asm__("_mach_msg");
extern char imp174 __asm__("_mach_port_allocate");
extern char imp175 __asm__("_mach_port_deallocate");
extern char imp176 __asm__("_mach_port_mod_refs");
extern char imp177 __asm__("_mach_port_type");
extern char imp178 __asm__("_mach_task_self_");
extern char imp179 __asm__("_malloc");
extern char imp180 __asm__("_memcmp");
extern char imp181 __asm__("_memcpy");
extern char imp182 __asm__("_method_exchangeImplementations");
extern char imp183 __asm__("_method_getImplementation");
extern char imp184 __asm__("_method_getName");
extern char imp185 __asm__("_method_getTypeEncoding");
extern char imp186 __asm__("_mmap");
extern char imp187 __asm__("_munmap");
extern char imp188 __asm__("_nanosleep");
extern char imp189 __asm__("_objc_alloc");
extern char imp190 __asm__("_objc_alloc_init");
extern char imp191 __asm__("_objc_autorelease");
extern char imp192 __asm__("_objc_autoreleasePoolPop");
extern char imp193 __asm__("_objc_autoreleasePoolPush");
extern char imp194 __asm__("_objc_enumerationMutation");
extern char imp195 __asm__("_objc_getAssociatedObject");
extern char imp196 __asm__("_objc_lookUpClass");
extern char imp197 __asm__("_objc_msgSend");
extern char imp198 __asm__("_objc_msgSendSuper2");
extern char imp199 __asm__("_objc_msgSend_stret");
extern char imp200 __asm__("_objc_opt_class");
extern char imp201 __asm__("_objc_opt_isKindOfClass");
extern char imp202 __asm__("_objc_opt_new");
extern char imp203 __asm__("_objc_opt_respondsToSelector");
extern char imp204 __asm__("_objc_release");
extern char imp205 __asm__("_objc_retain");
extern char imp206 __asm__("_objc_setAssociatedObject");
extern char imp207 __asm__("_objc_setProperty_nonatomic");
extern char imp208 __asm__("_open");
extern char imp209 __asm__("_protocol_getName");
extern char imp210 __asm__("_pthread_key_create");
extern char imp211 __asm__("_pthread_key_delete");
extern char imp212 __asm__("_pthread_mach_thread_np");
extern char imp213 __asm__("_pthread_mutex_destroy");
extern char imp214 __asm__("_pthread_mutex_init");
extern char imp215 __asm__("_pthread_mutex_lock");
extern char imp216 __asm__("_pthread_mutex_unlock");
extern char imp217 __asm__("_pthread_mutexattr_init");
extern char imp218 __asm__("_pthread_mutexattr_settype");
extern char imp219 __asm__("_pthread_self");
extern char imp220 __asm__("_recvmsg");
extern char imp221 __asm__("_sel_registerName");
extern char imp222 __asm__("_sem_close");
extern char imp223 __asm__("_sem_open");
extern char imp224 __asm__("_sem_post");
extern char imp225 __asm__("_sem_trywait");
extern char imp226 __asm__("_sem_unlink");
extern char imp227 __asm__("_sem_wait");
extern char imp228 __asm__("_sendmsg");
extern char imp229 __asm__("_setrlimit");
extern char imp230 __asm__("_setsockopt");
extern char imp231 __asm__("_shm_open");
extern char imp232 __asm__("_shm_unlink");
extern char imp233 __asm__("_snprintf");
extern char imp234 __asm__("_socket");
extern char imp235 __asm__("_sprintf");
extern char imp236 __asm__("_sscanf");
extern char imp237 __asm__("_strcmp");
extern char imp238 __asm__("_strcpy");
extern char imp239 __asm__("_strftime");
extern char imp240 __asm__("_strlen");
extern char imp241 __asm__("_strncpy");
extern char imp242 __asm__("_strrchr");
extern char imp243 __asm__("_strstr");
extern char imp244 __asm__("_strtoull");
extern char imp245 __asm__("_time");
extern char imp246 __asm__("_unlink");
extern char imp247 __asm__("_usleep");
extern char imp248 __asm__("_vfprintf");
void *const imports[] __attribute__((used)) = {
    &imp0,
    &imp1,
    &imp2,
    &imp3,
    &imp4,
    &imp5,
    &imp6,
    &imp7,
    &imp8,
    &imp9,
    &imp10,
    &imp11,
    &imp12,
    &imp13,
    &imp14,
    &imp15,
    &imp16,
    &imp17,
    &imp18,
    &imp19,
    &imp20,
    &imp21,
    &imp22,
    &imp23,
    &imp24,
    &imp25,
    &imp26,
    &imp27,
    &imp28,
    &imp29,
    &imp30,
    &imp31,
    &imp32,
    &imp33,
    &imp34,
    &imp35,
    &imp36,
    &imp37,
    &imp38,
    &imp39,
    &imp40,
    &imp41,
    &imp42,
    &imp43,
    &imp44,
    &imp45,
    &imp46,
    &imp47,
    &imp48,
    &imp49,
    &imp50,
    &imp51,
    &imp52,
    &imp53,
    &imp54,
    &imp55,
    &imp56,
    &imp57,
    &imp58,
    &imp59,
    &imp60,
    &imp61,
    &imp62,
    &imp63,
    &imp64,
    &imp65,
    &imp66,
    &imp67,
    &imp68,
    &imp69,
    &imp70,
    &imp71,
    &imp72,
    &imp73,
    &imp74,
    &imp75,
    &imp76,
    &imp77,
    &imp78,
    &imp79,
    &imp80,
    &imp81,
    &imp82,
    &imp83,
    &imp84,
    &imp85,
    &imp86,
    &imp87,
    &imp88,
    &imp89,
    &imp90,
    &imp91,
    &imp92,
    &imp93,
    &imp94,
    &imp95,
    &imp96,
    &imp97,
    &imp98,
    &imp99,
    &imp100,
    &imp101,
    &imp102,
    &imp103,
    &imp104,
    &imp105,
    &imp106,
    &imp107,
    &imp108,
    &imp109,
    &imp110,
    &imp111,
    &imp112,
    &imp113,
    &imp114,
    &imp115,
    &imp116,
    &imp117,
    &imp118,
    &imp119,
    &imp120,
    &imp121,
    &imp122,
    &imp123,
    &imp124,
    &imp125,
    &imp126,
    &imp127,
    &imp128,
    &imp129,
    &imp130,
    &imp131,
    &imp132,
    &imp133,
    &imp134,
    &imp135,
    &imp136,
    &imp137,
    &imp138,
    &imp139,
    &imp140,
    &imp141,
    &imp142,
    &imp143,
    &imp144,
    &imp145,
    &imp146,
    &imp147,
    &imp148,
    &imp149,
    &imp150,
    &imp151,
    &imp152,
    &imp153,
    &imp154,
    &imp155,
    &imp156,
    &imp157,
    &imp158,
    &imp159,
    &imp160,
    &imp161,
    &imp162,
    &imp163,
    &imp164,
    &imp165,
    &imp166,
    &imp167,
    &imp168,
    &imp169,
    &imp170,
    &imp171,
    &imp172,
    &imp173,
    &imp174,
    &imp175,
    &imp176,
    &imp177,
    &imp178,
    &imp179,
    &imp180,
    &imp181,
    &imp182,
    &imp183,
    &imp184,
    &imp185,
    &imp186,
    &imp187,
    &imp188,
    &imp189,
    &imp190,
    &imp191,
    &imp192,
    &imp193,
    &imp194,
    &imp195,
    &imp196,
    &imp197,
    &imp198,
    &imp199,
    &imp200,
    &imp201,
    &imp202,
    &imp203,
    &imp204,
    &imp205,
    &imp206,
    &imp207,
    &imp208,
    &imp209,
    &imp210,
    &imp211,
    &imp212,
    &imp213,
    &imp214,
    &imp215,
    &imp216,
    &imp217,
    &imp218,
    &imp219,
    &imp220,
    &imp221,
    &imp222,
    &imp223,
    &imp224,
    &imp225,
    &imp226,
    &imp227,
    &imp228,
    &imp229,
    &imp230,
    &imp231,
    &imp232,
    &imp233,
    &imp234,
    &imp235,
    &imp236,
    &imp237,
    &imp238,
    &imp239,
    &imp240,
    &imp241,
    &imp242,
    &imp243,
    &imp244,
    &imp245,
    &imp246,
    &imp247,
    &imp248,
};
#else
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("usage: %s <path to the imports dylib>\n", argv[0]);
        return 2;
    }
    void *handle = dlopen(argv[1], RTLD_NOW);
    if (!handle) {
        printf("FAIL: dlopen: %s\n", dlerror());
        return 1;
    }
    printf("PASS\n");
    return 0;
}
#endif
