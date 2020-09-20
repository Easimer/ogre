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

#include <GLES3/glesw.h>
#include <stdio.h>

#include <EGL/egl.h>
static void open_libgl() {}
static void close_libgl() {}
static GLESWglProc get_proc(const char *proc)
{
    return (GLESWglProc)eglGetProcAddress(proc);
}

static struct {
	int major, minor;
} version;

static int parse_version(void)
{
    if (!glGetString)
        return -1;

    const char* pcVer = (const char*)glGetString(GL_VERSION);
    sscanf(pcVer, "OpenGL ES %u.%u", &version.major, &version.minor);

    if (version.major < 2)
        return -1;
    return 0;
}

static void load_procs(GLESWGetProcAddressProc proc) {}

int gleswInit(void)
{
	open_libgl();
	load_procs(get_proc);
	close_libgl();
	return parse_version();
}

int gleswInit2(GLESWGetProcAddressProc proc)
{
	load_procs(proc);
	return parse_version();
}

int gleswIsSupported(int major, int minor)
{
	if (major < 2)
		return 0;
	if (version.major == major)
		return version.minor >= minor;
	return version.major >= major;
}

GLESWglProc gleswGetProcAddress(const char *proc)
{
	return get_proc(proc);
}
