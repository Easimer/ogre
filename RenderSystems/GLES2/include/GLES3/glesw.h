/*

    This file was generated with glesw_gen.cmake, part of glXXw
    (hosted at https://github.com/paroj/glXXw-cmake)

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef __glesw_h_
#define __glesw_h_

#include <GLES2/gl2chromium.h>
#include <GLES3/gl3.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <GLES2/gl2ext.h>
#include <GLES2/gl2extchromium.h>
#include <KHR/khrplatform.h>
#include <GLES2/gl2platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GLESWglProc)(void);
typedef GLESWglProc (*GLESWGetProcAddressProc)(const char *proc);

/* glesw api */
int gleswInit(void);
int gleswInit2(GLESWGetProcAddressProc proc);
int gleswIsSupported(int major, int minor);
GLESWglProc gleswGetProcAddress(const char *proc);

#ifdef __cplusplus
}
#endif
#endif
