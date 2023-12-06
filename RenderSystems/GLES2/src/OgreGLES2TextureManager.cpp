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

#include "OgreGLES2TextureManager.h"
#include "OgreGLRenderTexture.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGLES2StateCacheManager.h"
#include "OgreGLES2PixelFormat.h"

namespace Ogre {
    GLES2TextureManager::GLES2TextureManager(GLES2RenderSystem* renderSystem)
        : TextureManager(), mRenderSystem(renderSystem)
    {
        // Register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }

    GLES2TextureManager::~GLES2TextureManager()
    {
        // Unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }

    Resource* GLES2TextureManager::createImpl(const String& name, ResourceHandle handle, 
                                           const String& group, bool isManual,
                                           ManualResourceLoader* loader,
                                           const NameValuePairList* createParams)
    {
        return OGRE_NEW GLES2Texture(this, name, handle, group, isManual, loader, mRenderSystem);
    }

    PixelFormat GLES2TextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Adjust requested parameters to capabilities
        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        // Check compressed texture support
        // if a compressed format not supported, revert to PF_A8R8G8B8
        if (PixelUtil::isCompressed(format) &&
            !caps->hasCapability(RSC_TEXTURE_COMPRESSION))
        {
            return PF_BYTE_RGBA;
        }
        // if floating point textures not supported, revert to PF_A8R8G8B8
        if (PixelUtil::isFloatingPoint(format) &&
            !caps->hasCapability(RSC_TEXTURE_FLOAT))
        {
            return PF_BYTE_RGBA;
        }

        // Check if this is a valid rendertarget format
        if (usage & TU_RENDERTARGET)
        {
            /// Get closest supported alternative
            /// If mFormat is supported it's returned
            return GLRTTManager::getSingleton().getSupportedAlternative(format);
        }

#if EMSCRIPTEN
        if(format == PF_A8R8G8B8) {
            return PF_BYTE_RGBA;
        }
#endif /* EMSCRIPTEN */

        // format not supported by GLES2: e.g. BGR
        if(GLES2PixelUtil::getGLInternalFormat(format) == GL_NONE)
        {
            return PF_BYTE_RGBA;
        }

        // Supported
        return format;
    }

    bool GLES2TextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
            bool preciseFormatOnly)
    {
        // precise format check
        if (!TextureManager::isHardwareFilteringSupported(ttype, format, usage, preciseFormatOnly))
            return false;

        // Assume non-floating point is supported always
        if (!PixelUtil::isFloatingPoint(getNativeFormat(ttype, format, usage)))
            return true;
        
        // check for floating point extension
        return mRenderSystem->checkExtension("GL_OES_texture_float_linear");
    }

    SamplerPtr GLES2TextureManager::_createSamplerImpl()
    {
        auto hasAnisotropy = mRenderSystem->getCapabilities()->hasCapability(RSC_ANISOTROPY);
        return std::make_shared<WebGL2Sampler>(hasAnisotropy);
    }

    WebGL2Sampler::WebGL2Sampler(bool supportsAnisotropy) : mHandle(0), mSupportsAnisotropy(supportsAnisotropy)
    {
    }

    WebGL2Sampler::~WebGL2Sampler() {
        if(mHandle == 0) {
            return;
        }

        glDeleteSamplers(1, &mHandle);
    }

    static GLint getTextureAddressingMode(TextureAddressingMode tam)
    {
        switch (tam)
        {
            case TextureUnitState::TAM_CLAMP:
            case TextureUnitState::TAM_BORDER:
                return GL_CLAMP_TO_EDGE;
            case TextureUnitState::TAM_MIRROR:
                return GL_MIRRORED_REPEAT;
            case TextureUnitState::TAM_WRAP:
            default:
                return GL_REPEAT;
        }
    }

    static GLint convertCompareFunction(CompareFunction func)
    {
        switch(func)
        {
            case CMPF_ALWAYS_FAIL:
                return GL_NEVER;
            case CMPF_ALWAYS_PASS:
                return GL_ALWAYS;
            case CMPF_LESS:
                return GL_LESS;
            case CMPF_LESS_EQUAL:
                return GL_LEQUAL;
            case CMPF_EQUAL:
                return GL_EQUAL;
            case CMPF_NOT_EQUAL:
                return GL_NOTEQUAL;
            case CMPF_GREATER_EQUAL:
                return GL_GEQUAL;
            case CMPF_GREATER:
                return GL_GREATER;
        };
        // To keep compiler happy
        return GL_ALWAYS;
    }

    static GLint getCombinedMinMipFilter(FilterOptions min, FilterOptions mip)
    {
        switch(min)
        {
        case FO_ANISOTROPIC:
        case FO_LINEAR:
            switch (mip)
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                // linear min, linear mip
                return GL_LINEAR_MIPMAP_LINEAR;
            case FO_POINT:
                // linear min, point mip
                return GL_LINEAR_MIPMAP_NEAREST;
            case FO_NONE:
                // linear min, no mip
                return GL_LINEAR;
            }
            break;
        case FO_POINT:
        case FO_NONE:
            switch (mip)
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                // nearest min, linear mip
                return GL_NEAREST_MIPMAP_LINEAR;
            case FO_POINT:
                // nearest min, point mip
                return GL_NEAREST_MIPMAP_NEAREST;
            case FO_NONE:
                // nearest min, no mip
                return GL_NEAREST;
            }
            break;
        }

        // should never get here
        return 0;
    }


    void WebGL2Sampler::bind(GLuint unit)
    {
        if (!mDirty && mHandle != 0)
        {
            glBindSampler(unit, mHandle);
            return;
        }

        if (mHandle == 0)
        {
            glGenSamplers(1, &mHandle);
        }

        const Sampler::UVWAddressingMode &uvw = getAddressingMode();

        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_S, getTextureAddressingMode(uvw.u));
        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_T, getTextureAddressingMode(uvw.v));
        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_R, getTextureAddressingMode(uvw.w));

        bool compareEnabled = getCompareEnabled();
        glSamplerParameteri(mHandle, GL_TEXTURE_COMPARE_MODE, compareEnabled ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
        if (compareEnabled)
        {
            glSamplerParameteri(mHandle, GL_TEXTURE_COMPARE_FUNC, convertCompareFunction(getCompareFunction()));
        }

        glSamplerParameteri(
            mHandle, GL_TEXTURE_MIN_FILTER,
            getCombinedMinMipFilter(getFiltering(FT_MIN), getFiltering(FT_MIP)));

        switch (getFiltering(FT_MAG))
        {
        case FO_ANISOTROPIC: // GL treats linear and aniso the same
        case FO_LINEAR:
            glSamplerParameteri(mHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case FO_POINT:
        case FO_NONE:
            glSamplerParameteri(mHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        }

        if (mSupportsAnisotropy) {
            glSamplerParameteri(mHandle, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                getAnisotropy());
        }

        glBindSampler(unit, mHandle);
        mDirty = false;
    }
}
