/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreGLSLESExtSupport.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreGLUtil.h"
#include "OgreGLES2RenderSystem.h"

namespace Ogre
{
    namespace GLSLES {
    String logObjectInfo(const String& msg, GLuint obj)
    {
        String logMessage = getObjectInfo(obj);

        if (logMessage.empty())
            return msg;

        logMessage = msg + "\n" + logMessage;

        LogManager::getSingleton().logMessage(LML_CRITICAL, logMessage);

        return logMessage;
    }

    String getObjectInfo(const GLuint obj)
    {
        String logMessage;

        if (obj > 0)
        {
            GLint infologLength = 0;

            if(glIsShader(obj))
            {
                OGRE_CHECK_GL_ERROR(glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength));
            }
            else if(glIsProgram(obj))
            {
                OGRE_CHECK_GL_ERROR(glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength));
            }

            if (infologLength > 1)
            {
                GLint charsWritten  = 0;

                char * infoLog = new char [infologLength];
                infoLog[0] = 0;

                if(glIsShader(obj))
                {
                    OGRE_CHECK_GL_ERROR(glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog));
                }
                else if(glIsProgram(obj))
                {
                    OGRE_CHECK_GL_ERROR(glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog));
                }

                if (strlen(infoLog) > 0)
                {
                    logMessage = String(infoLog);
                }

                delete [] infoLog;

                StringUtil::trim(logMessage, false, true);
            }
        }

        return logMessage;
    }

    }
} // namespace Ogre
