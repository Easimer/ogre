#pragma once

/**
 * NOTE(danielm): this file renames GL functions that have different names in
 * the Chromium GL headers than in the standard ones, and stubs out functions
 * that are not implemented on the Chromium interface.
 * 
 * This header file must not be included before GL headers.
 */

#ifdef OGRE_GLES2_ANGLE

#ifndef GL_APIENTRYP
#error "ANGLECompat.h must not be included before GL headers"
#endif

#include <GLES2/gl2chromium.h>
#include <GLES3/gl3.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <GLES2/gl2ext.h>
#include <GLES2/gl2extchromium.h>
#include <KHR/khrplatform.h>
#include <GLES2/gl2platform.h>

#if __cplusplus
extern "C"
{
#endif

#define glGenQueries glGenQueriesEXT
#define glDeleteQueries glDeleteQueriesEXT
#define glBeginQuery glBeginQueryEXT
#define glEndQuery glEndQueryEXT
#define glGetQueryObjectuiv glGetQueryObjectuivEXT

#define glDrawBuffers glDrawBuffersEXT
#define glBlitFramebuffer glBlitFramebufferCHROMIUM

#define glUnmapBufferOES glUnmapBuffer
#define glMapBufferRangeEXT glMapBufferRange

#define glTexSubImage3DOES glTexSubImage3D
#define glCompressedTexSubImage3DOES glCompressedTexSubImage3D
#define glCompressedTexImage3DOES glCompressedTexImage3D
#define glRenderbufferStorageMultisampleAPPLE glRenderbufferStorageMultisampleCHROMIUM
#define glTexStorage2D glTexStorage2DEXT
#define glTexImage3DOES glTexImage3D

#define glDrawArraysInstancedEXT glDrawArraysInstancedANGLE
#define glDrawElementsInstancedEXT glDrawElementsInstancedANGLE
#define glVertexAttribDivisorEXT glVertexAttribDivisorANGLE

#define glGetProgramPipelineInfoLogEXT(obj, len, written, buf)
#define glGetProgramPipelineivEXT(obj, len, buf) 
// NOTE(danielm): Ogre uses this to make sure that `obj` is a pipeline object before calling
// glGetProgramPipelineivEXT or glGetProgramPipelineInfoLogEXT; however these three functions don't exist on ANGLE
// so this always returns false and the other two are no-ops.
#define glIsProgramPipelineEXT(obj) (false)

#define glLabelObjectEXT(type, object, length, label)
#define glProgramParameteriEXT(program, prop, val)

#define glProgramBinaryOES(program, format, buf, len) abort()
#define glGetProgramBinaryOES(program, bufsize, len, format, buf) abort()

// NOTE(danielm): if you're seeing an error from this you're trying to use
// pipeline objects which don't exist under ANGLE.
// If it's Ogre doing it, you can ifdef that part of the code using the macro
// OGRE_GLES2_ANGLE.
#define glBindProgramPipelineEXT() (!!)

#if __cplusplus
}
#endif

#endif
