/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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
--------------------------------------------------------------------------*/

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreViewport.h"

#include "OgreElectronEGLSupport.h"
#include "OgreElectronEGLWindow.h"

#include <algorithm>
#include <climits>
#include <iostream>

namespace Ogre
{

static EGLBoolean tryMakeCurrent(::EGLDisplay mEglDisplay, ::EGLSurface mEglSurface, ::EGLContext eglContext) noexcept
{
    return eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, eglContext);
}

ElectronEGLWindow::ElectronEGLWindow(ElectronEGLSupport* glsupport) : EGLWindow(glsupport)
{
    mGLSupport = glsupport;
    mNativeDisplay = glsupport->getNativeDisplay();
}

void ElectronEGLWindow::resize(unsigned int width, unsigned int height)
{
    if (width != mWidth || height != mHeight)
    {
        mWidth = width;
        mHeight = height;
    }
}

void ElectronEGLWindow::create(const String& name, uint width, uint height, bool fullScreen,
                               const NameValuePairList* miscParams)
{
    String title = name;
    uint samples = 0;
    int gamma;
    short frequency = 0;
    bool vsync = false;
    ::EGLContext eglContext = 0;
    int left = 0;
    int top = 0;

    mIsFullScreen = fullScreen;

    if (!miscParams)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "miscParams MUST BE provided when creating a window when using the Electron GLES2 backend",
                    "ElectronEGLWindow::create");
    }

    if (miscParams->count("externalWindowHandle") == 0)
    {
        OGRE_EXCEPT(
            Exception::ERR_RENDERINGAPI_ERROR,
            "miscParams[`externalWindowHandle`] MUST BE provided when creating a window when using the Electron "
            "GLES2 backend",
            "ElectronEGLWindow::create");
    }

    mWindow = (EGLNativeWindowType)StringConverter::parseSizeT(miscParams->at("externalWindowHandle"));
    if (mWindow == nullptr)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "miscParams[`externalWindowHandle`] MUST NOT BE 0 when creating a window when using the Electron "
                    "GLES2 backend",
                    "ElectronEGLWindow::create");
    }

    NameValuePairList::const_iterator opt;
    NameValuePairList::const_iterator end = miscParams->end();

    // Note: Some platforms support AA inside ordinary windows
    if ((opt = miscParams->find("FSAA")) != end)
    {
        samples = StringConverter::parseUnsignedInt(opt->second);
    }

    if ((opt = miscParams->find("displayFrequency")) != end)
    {
        frequency = (short)StringConverter::parseInt(opt->second);
    }

    if ((opt = miscParams->find("vsync")) != end)
    {
        vsync = StringConverter::parseBool(opt->second);
    }

    if ((opt = miscParams->find("gamma")) != end)
    {
        gamma = StringConverter::parseBool(opt->second);
    }

    if ((opt = miscParams->find("title")) != end)
    {
        title = opt->second;
    }

    if ((opt = miscParams->find("externalGLControl")) != end)
    {
        mIsExternalGLControl = StringConverter::parseBool(opt->second);
    }

    EGLBoolean success;
    mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    success = eglInitialize(mEglDisplay, nullptr, nullptr);
    if (!success)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't initialize EGL", "ElectronEGLWindow::create");
    }

    // NOTE(danielm): no need to ever call eglBindAPI, it's a no-op in electron

    mGLSupport->setGLDisplay(mEglDisplay);
    mIsExternal = true;

    if (!mEglConfig)
    {
        int minAttribs[] = {EGL_LEVEL,
                            0,
                            EGL_DEPTH_SIZE,
                            16,
                            EGL_SURFACE_TYPE,
                            EGL_WINDOW_BIT,
                            EGL_RENDERABLE_TYPE,
                            EGL_OPENGL_ES2_BIT,
                            EGL_NATIVE_RENDERABLE,
                            EGL_FALSE,
                            EGL_DEPTH_SIZE,
                            EGL_DONT_CARE,
                            EGL_NONE};

        int maxAttribs[] = {EGL_SAMPLES, samples, EGL_STENCIL_SIZE, INT_MAX, EGL_NONE};

        mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
        mHwGamma = false;
    }

    if (!mIsTopLevel)
    {
        mIsFullScreen = false;
        left = top = 0;
    }

    if (mIsFullScreen)
    {
        mGLSupport->switchMode(width, height, frequency);
    }

    mHwGamma = false;
    EGLint arrNoHwGamma[] = {EGL_NONE};
    mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, mWindow, arrNoHwGamma);

    if (mEglSurface == EGL_NO_SURFACE)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Fail to create EGLSurface based on NativeWindowType",
                    "ElectronEGLWindow::create");
    }

    EGLint contextAttrs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 2, EGL_NONE, EGL_NONE, EGL_NONE};
    eglContext = eglCreateContext(mEglDisplay, mEglConfig, NULL, NULL);

    if (eglContext == NULL)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't create EGL context", "ElectronEGLWindow::create");
    }

    tryMakeCurrent(mEglDisplay, mEglSurface, eglContext);

    mContext = createEGLContext(eglContext);
    mContext->setCurrent();

    mName = name;
    mWidth = width;
    mHeight = height;
    mLeft = left;
    mTop = top;

    finaliseWindow();
}

} // namespace Ogre
