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

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include "OgreViewport.h"

#include "OgreOSXEGLSupport.h"
#include "OgreOSXEGLWindow.h"

#include <iostream>
#include <algorithm>
#include <climits>

#import <Cocoa/Cocoa.h>
#import <AppKit/NSScreen.h>

namespace Ogre {
    OSXEGLWindow::OSXEGLWindow(OSXEGLSupport *glsupport)
        : EGLWindow(glsupport)
    {
        mGLSupport = glsupport;
        mNativeDisplay = glsupport->getNativeDisplay();
    }

    void OSXEGLWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
    {

    }

    void OSXEGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
    {

    }

    void OSXEGLWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
    {

    }

    void OSXEGLWindow::reposition( int left, int top )
    {

    }

    void OSXEGLWindow::resize( unsigned int width, unsigned int height )
    {

    }

    void OSXEGLWindow::windowMovedOrResized()
    {
  		if (!mWindow)
  			return;

      NSWindow* window = reinterpret_cast<NSWindow*>(mWindow);

      NSRect winFrame = [window frame];
      NSRect viewFrame = [[window contentView] frame];
      NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];
      CGFloat leftPt = winFrame.origin.x;
      CGFloat topPt = screenFrame.size.height - winFrame.size.height;
      mWidth = (unsigned int)viewFrame.size.width;
      mHeight = (unsigned int)viewFrame.size.height;
      mLeft = (int)leftPt;
      mTop = (int)topPt;

      for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
      {
          (*it).second->_updateDimensions();
      }
    }

    void OSXEGLWindow::switchFullScreen( bool fullscreen )
    {

    }

    void OSXEGLWindow::create(const String& name, uint width, uint height,
                                bool fullScreen, const NameValuePairList *miscParams)
    {
        String title = name;
        uint samples = 0;
        int gamma;
        short frequency = 0;
        bool vsync = false;
        ::EGLContext eglContext = 0;
        int left = 0;
        int top  = 0;

        getLeftAndTopFromNativeWindow(left, top, width, height);

        mIsFullScreen = fullScreen;

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            if ((opt = miscParams->find("externalWindowHandle")) != end)
            {
                mWindow = (NSWindow*)StringConverter::parseSizeT(opt->second);
            }

            if ((opt = miscParams->find("currentGLContext")) != end &&
                StringConverter::parseBool(opt->second))
            {
                eglContext = eglGetCurrentContext();
                if (!eglContext)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                "currentGLContext was specified with no current GL context",
                                "EGLWindow::create");
                }

                mEglSurface = eglGetCurrentSurface(EGL_DRAW);
                mEglDisplay = eglGetCurrentDisplay();
            }

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

            if ((opt = miscParams->find("left")) != end)
            {
                left = StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("top")) != end)
            {
                top = StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("title")) != end)
            {
                title = opt->second;
            }

            if ((opt = miscParams->find("externalGLControl")) != end)
            {
                mIsExternalGLControl = StringConverter::parseBool(opt->second);
            }
        }

        initNativeCreatedWindow(miscParams);

        if (mWindow)
        {
          mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
          eglInitialize(mEglDisplay, NULL, NULL);

          eglBindAPI(EGL_OPENGL_ES_API);

          mGLSupport->setGLDisplay(mEglDisplay);
          mIsExternal = true;
        }

        if (!mEglConfig)
        {
            int minAttribs[] = {
                EGL_LEVEL, 0,
                EGL_DEPTH_SIZE, 16,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
                EGL_NATIVE_RENDERABLE,  EGL_FALSE,
                EGL_DEPTH_SIZE,         EGL_DONT_CARE,
                EGL_NONE
            };

            int maxAttribs[] = {
                EGL_SAMPLES, (int)samples,
                EGL_STENCIL_SIZE, INT_MAX,
                EGL_NONE
            };

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
            mGLSupport->switchMode (width, height, frequency);
        }

        if (!mIsExternal)
        {
            createNativeWindow(left, top, width, height, title);
        } else {
            mEglSurface = createSurfaceFromWindow(mEglDisplay, mWindow);
        }

        mContext = createEGLContext(eglContext);
        mContext->setCurrent();
        ::EGLSurface oldDrawableDraw = eglGetCurrentSurface(EGL_DRAW);
        ::EGLSurface oldDrawableRead = eglGetCurrentSurface(EGL_READ);
        ::EGLContext oldContext  = eglGetCurrentContext();

        int glConfigID;

        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_CONFIG_ID, &glConfigID);
        LogManager::getSingleton().logMessage("EGLWindow::create used FBConfigID = " + StringConverter::toString(glConfigID));

        mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = left;
        mTop = top;
        mActive = true;
        mVisible = true;

        mClosed = false;
    }

}
