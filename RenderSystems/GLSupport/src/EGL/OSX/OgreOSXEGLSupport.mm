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
-----------------------------------------------------------------------------
*/

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

#include "OgreOSXEGLSupport.h"
#include "OgreOSXEGLWindow.h"

#include "OgreGLUtil.h"

#import <OpenGL/OpenGL.h>
#import <AppKit/AppKit.h>

namespace Ogre {
    GLNativeSupport* getGLSupport(int profile)
    {
        return new OSXEGLSupport(profile);
    }

    OSXEGLSupport::OSXEGLSupport(int profile)
		: EGLSupport(profile)
    {
        mNativeDisplay = getNativeDisplay();
        mGLDisplay = getGLDisplay();

        CGLRendererInfoObj rend;
      	GLint nrend = 0, maxSamples = 0;

      	CGLQueryRendererInfo(
              CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
              &rend, &nrend);
      	CGLDescribeRenderer(rend, 0, kCGLRPMaxSamples, &maxSamples);
        CGLDestroyRendererInfo(rend);

        // FSAA possibilities
        for(int i = 0; i <= maxSamples; i += 2)
          mFSAALevels.push_back( i );

      	// Video mode possibilities
      	CFArrayRef displayModes =
            CGDisplayCopyAllDisplayModes(CGMainDisplayID(), NULL);
      	CFIndex numModes = CFArrayGetCount(displayModes);
      	CFMutableArrayRef goodModes = NULL;
      	goodModes = CFArrayCreateMutable(kCFAllocatorDefault, numModes, NULL);

      	// Grab all the available display modes, then weed out duplicates...
      	for(int i = 0; i < numModes; ++i)
      	{
      		CGDisplayModeRef modeInfo =
              (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, i);

          // Get IOKit flags for the display mode
          uint32_t ioFlags = CGDisplayModeGetIOFlags(modeInfo);

          bool safeForHardware =
              ioFlags & kDisplayModeSafetyFlags ? true : false;
      		bool stretched =
              ioFlags & kDisplayModeStretchedFlag ? true : false;
      		bool skipped = false;

      		if((safeForHardware) || (!stretched))
      		{
      			size_t width  = CGDisplayModeGetWidth(modeInfo);
      			size_t height = CGDisplayModeGetHeight(modeInfo);

      			for(CFIndex j = 0; j < CFArrayGetCount(goodModes); ++j)
      			{
      				CGDisplayModeRef otherMode =
                  (CGDisplayModeRef)CFArrayGetValueAtIndex(goodModes, j);

      				size_t otherWidth  = CGDisplayModeGetWidth(otherMode);
      				size_t otherHeight = CGDisplayModeGetHeight(otherMode);

      				// If we find a duplicate then skip this mode
      				if((otherWidth == width) && (otherHeight == height))
      					skipped = true;
      			}

      			// This is a new mode, so add it to our goodModes array
      			if(!skipped)
      				CFArrayAppendValue(goodModes, modeInfo);
      		}
      	}

        // Release memory
        CFRelease(displayModes);

      	// Sort the modes...
      	CFArraySortValues(goodModes, CFRangeMake(0, CFArrayGetCount(goodModes)),
            (CFComparatorFunction)_compareModes, NULL);

      	// Now pull the modes out and put them into optVideoModes
      	for(int i = 0; i < CFArrayGetCount(goodModes); ++i)
      	{
      		CGDisplayModeRef resolution =
              (CGDisplayModeRef)CFArrayGetValueAtIndex(goodModes, i);

      		size_t fWidth  = CGDisplayModeGetWidth(resolution);
      		size_t fHeight = CGDisplayModeGetHeight(resolution);
      		// allow 16 and 32 bpp
      		mVideoModes.push_back({uint32(fWidth), uint32(fHeight),0, 16});
      		mVideoModes.push_back({uint32(fWidth), uint32(fHeight),0, 32});
        }

        // Release memory
        CFRelease(goodModes);
    }

    CFComparisonResult OSXEGLSupport::_compareModes (const void *val1, const void *val2, void *context)
    {
    	// These are the values we will be interested in...
    	/*
    	CGDisplayModeGetWidth
    	CGDisplayModeGetHeight
    	CGDisplayModeGetRefreshRate
    	_getDictionaryLong((mode), kCGDisplayBitsPerPixel)
    	CGDisplayModeGetIOFlags((mode), kDisplayModeStretchedFlag)
    	CGDisplayModeGetIOFlags((mode), kDisplayModeSafetyFlags)
    	*/

    	// CFArray comparison callback for sorting display modes.
    	#pragma unused(context)
    	CGDisplayModeRef thisMode = (CGDisplayModeRef)val1;
    	CGDisplayModeRef otherMode = (CGDisplayModeRef)val2;

    	size_t width = CGDisplayModeGetWidth(thisMode);
    	size_t otherWidth = CGDisplayModeGetWidth(otherMode);

    	size_t height = CGDisplayModeGetHeight(thisMode);
    	size_t otherHeight = CGDisplayModeGetHeight(otherMode);

    	// Sort modes in screen size order
    	if (width * height < otherWidth * otherHeight)
    	{
    		return kCFCompareLessThan;
    	}
    	else if (width * height > otherWidth * otherHeight)
    	{
    		return kCFCompareGreaterThan;
    	}

    	// Sort modes by refresh rate.
    	double refreshRate = CGDisplayModeGetRefreshRate(thisMode);
    	double otherRefreshRate = CGDisplayModeGetRefreshRate(otherMode);

    	if (refreshRate < otherRefreshRate)
    	{
    		return kCFCompareLessThan;
    	}
    	else if (refreshRate > otherRefreshRate)
    	{
    		return kCFCompareGreaterThan;
    	}

    	return kCFCompareEqualTo;
    }

    OSXEGLSupport::~OSXEGLSupport()
    {

    }

    //Removed createEGLWindow because it was easier to call new OSXEGLWindow
    //directly to get the native version.
//  EGLWindow* OSXEGLSupport::createEGLWindow(  EGLSupport * support )
//  {
//      return new OSXEGLWindow(support);
//  }

    /*GLESPBuffer* OSXEGLSupport::createPBuffer( PixelComponentType format, size_t width, size_t height )
    {
        return new Win32EGLPBuffer(this, format, width, height);
    }*/

    void OSXEGLSupport::switchMode( uint& width, uint& height, short& frequency )
    {
        //todo
    }

    //Moved to native from EGLSupport
    RenderWindow* OSXEGLSupport::newWindow(const String &name,
        unsigned int width, unsigned int height,
        bool fullScreen,
        const NameValuePairList *miscParams)
    {
//        EGLWindow* window = createEGLWindow(this);

        OSXEGLWindow* window = new OSXEGLWindow(this);
        window->create(name, width, height, fullScreen, miscParams);

        return window;
    }

    //Moved to native from EGLSupport
    NativeDisplayType OSXEGLSupport::getNativeDisplay()
    {
        return EGL_DEFAULT_DISPLAY; // TODO
    }

    //OSXEGLSupport::getGLDisplay sets up the native variable
    //then calls EGLSupport::getGLDisplay
    EGLDisplay OSXEGLSupport::getGLDisplay()
    {
        if (!mGLDisplay)
        {
            mNativeDisplay = getNativeDisplay();
            return EGLSupport::getGLDisplay();
        }
        return mGLDisplay;
    }


}
