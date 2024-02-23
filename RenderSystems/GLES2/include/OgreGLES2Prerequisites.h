/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __GLES2Prerequisites_H__
#define __GLES2Prerequisites_H__

#include "OgrePrerequisites.h"

#include "OgreGLES2Exports.h"
#include "OgreGLES2Config.h"

namespace Ogre {
    class GLContext;
    typedef GLContext GLES2Context;
}

#include <GLES3/glesw.h>

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
#   ifdef __OBJC__
#       include <OpenGLES/EAGL.h>
#   endif
#elif (OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN 1
#   endif
#   ifndef NOMINMAX
#       define NOMINMAX // required to stop windows.h messing up std::min
#   endif
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
namespace Ogre {
extern float EAGLCurrentOSVersion;
}
#define OGRE_IF_IOS_VERSION_IS_GREATER_THAN(vers) \
    if(EAGLCurrentOSVersion >= vers)
#else
#define OGRE_IF_IOS_VERSION_IS_GREATER_THAN(vers)
#endif

#define getGLES2RenderSystem() dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())

// Copy this definition from desktop GL.  Used for polygon modes.
#ifndef GL_FILL
#   define GL_FILL    0x1B02
#endif

namespace Ogre {
    class GLNativeSupport;
    class GLES2GpuProgram;
    class GLES2Texture;
    typedef shared_ptr<GLES2GpuProgram> GLES2GpuProgramPtr;
    typedef shared_ptr<GLES2Texture> GLES2TexturePtr;
};

#if OGRE_NO_GLES3_SUPPORT == 0
#undef GL_DEPTH_COMPONENT32_OES
#define GL_DEPTH_COMPONENT32_OES GL_DEPTH_COMPONENT32F
#endif

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
// an error in all windows gles sdks...
#   undef GL_OES_get_program_binary
#endif

#define ENABLE_GL_CHECK 0

#if ENABLE_GL_CHECK
#include "OgreLogManager.h"
#define OGRE_CHECK_GL_ERROR(glFunc) \
{ \
    glFunc; \
    int e = glGetError(); \
    if (e != 0) \
    { \
        const char * errorString = ""; \
        switch(e) \
        { \
        case GL_INVALID_ENUM:       errorString = "GL_INVALID_ENUM";        break; \
        case GL_INVALID_VALUE:      errorString = "GL_INVALID_VALUE";       break; \
        case GL_INVALID_OPERATION:  errorString = "GL_INVALID_OPERATION";   break; \
        case GL_OUT_OF_MEMORY:      errorString = "GL_OUT_OF_MEMORY";       break; \
        default:                                                            break; \
        } \
        String funcname = #glFunc; \
        funcname = funcname.substr(0, funcname.find('(')); \
        LogManager::getSingleton().logError(StringUtil::format("%s failed with %s in %s at %s(%d)",          \
                                                                funcname.c_str(), errorString, __FUNCTION__, \
                                                                __FILE__, __LINE__));                        \
    } \
}
#else
#   define OGRE_CHECK_GL_ERROR(glFunc) { glFunc; }
#endif

#endif

// MaxWhere start

// Tokens introduced by EXT_texture_compression_rgtc
#ifndef GL_EXT_texture_compression_rgtc
#define GL_EXT_texture_compression_rgtc 1
#define GL_COMPRESSED_RED_RGTC1_EXT 0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1_EXT 0x8DBC
#define GL_COMPRESSED_RED_GREEN_RGTC2_EXT 0x8DBD
#define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT 0x8DBE
#endif /* GL_EXT_texture_compression_rgtc */

// Tokens introduced by EXT_texture_compression_bptc
#ifndef GL_EXT_texture_compression_bptc
#define GL_EXT_texture_compression_bptc 1
#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT 0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT 0x8E8F
#endif /* GL_EXT_texture_compression_bptc */

// Tokens introduced by WEBGL_compressed_texture_s3tc_srgb
#ifndef GL_EXT_texture_compression_s3tc_srgb
#define GL_EXT_texture_compression_s3tc_srgb 1
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#endif /* GL_EXT_texture_compression_s3tc_srgb */

// MaxWhere end
