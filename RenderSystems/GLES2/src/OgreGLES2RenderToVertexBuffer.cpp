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

#include "OgreGLES2RenderToVertexBuffer.h"

#include "OgreHardwareBufferManager.h"
#include "OgreGLES2HardwareBuffer.h"
#include "OgreRenderable.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGLSLESProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreTechnique.h"

namespace Ogre {
//-----------------------------------------------------------------------------
    static GLint getR2VBPrimitiveType(RenderOperation::OperationType operationType)
    {
        switch (operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            return GL_POINTS;
        case RenderOperation::OT_LINE_LIST:
            return GL_LINES;
        case RenderOperation::OT_TRIANGLE_LIST:
            return GL_TRIANGLES;
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "GL RenderToVertexBuffer"
                "can only output point lists, line lists, or triangle lists",
                "OgreGLES2RenderToVertexBuffer::getR2VBPrimitiveType");
        }
    }
//-----------------------------------------------------------------------------
    static GLint getVertexCountPerPrimitive(RenderOperation::OperationType operationType)
    {
        //We can only get points, lines or triangles since they are the only
        //legal R2VB output primitive types
        switch (operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            return 1;
        case RenderOperation::OT_LINE_LIST:
            return 2;
        default:
        case RenderOperation::OT_TRIANGLE_LIST:
            return 3;
        }
    }
//-----------------------------------------------------------------------------
    GLES2RenderToVertexBuffer::GLES2RenderToVertexBuffer() : mFrontBufferIndex(-1)
    {
        mVertexBuffers[0].reset();
        mVertexBuffers[1].reset();

        // Create query objects
        OGRE_CHECK_GL_ERROR(glGenQueriesEXT(1, &mPrimitivesDrawnQuery));
    }
//-----------------------------------------------------------------------------
    GLES2RenderToVertexBuffer::~GLES2RenderToVertexBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteQueriesEXT(1, &mPrimitivesDrawnQuery));
    }
//-----------------------------------------------------------------------------
    void GLES2RenderToVertexBuffer::getRenderOperation(RenderOperation& op)
    {
        op.operationType = mOperationType;
        op.useIndexes = false;
        op.vertexData = mVertexData.get();
    }
//-----------------------------------------------------------------------------
    void GLES2RenderToVertexBuffer::update(SceneManager* sceneMgr)
    {
        size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
        if (!mVertexBuffers[0] || mVertexBuffers[0]->getSizeInBytes() != bufSize)
        {
            // Buffers don't match. Need to reallocate.
            mResetRequested = true;
        }
        
        // Single pass only for now
        Ogre::Pass* r2vbPass = mMaterial->getBestTechnique()->getPass(0);
        // Set pass before binding buffers to activate the GPU programs
        sceneMgr->_setPass(r2vbPass);
        
        bindVerticesOutput(r2vbPass);

        r2vbPass->_updateAutoParams(sceneMgr->_getAutoParamDataSource(), GPV_GLOBAL);

        RenderOperation renderOp;
        size_t targetBufferIndex;
        if (mResetRequested || mResetsEveryUpdate)
        {
            // Use source data to render to first buffer
            mSourceRenderable->getRenderOperation(renderOp);
            targetBufferIndex = 0;
        }
        else
        {
            // Use current front buffer to render to back buffer
            this->getRenderOperation(renderOp);
            targetBufferIndex = 1 - mFrontBufferIndex;
        }

        if (!mVertexBuffers[targetBufferIndex] || 
            mVertexBuffers[targetBufferIndex]->getSizeInBytes() != bufSize)
        {
            reallocateBuffer(targetBufferIndex);
        }

        auto vertexBuffer = mVertexBuffers[targetBufferIndex]->_getImpl<GLES2HardwareBuffer>();
/*        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLESProgramPipeline* programPipeline =
                GLSLESProgramPipelineManager::getSingleton().getActiveProgramPipeline();
            programPipeline->getVertexArrayObject()->bind();
        }
        else
        {
            GLSLESLinkProgram* linkProgram = GLSLESLinkProgramManager::getSingleton().getActiveLinkProgram();
            linkProgram->getVertexArrayObject()->bind();
        }
        */
        // Bind the target buffer
        OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertexBuffer->getGLBufferId()));

        // Disable rasterization
        OGRE_CHECK_GL_ERROR(glEnable(GL_RASTERIZER_DISCARD));

        RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();
        // Draw the object
        if (r2vbPass->hasVertexProgram())
        {
            targetRenderSystem->bindGpuProgramParameters(GPT_VERTEX_PROGRAM, 
                                                         r2vbPass->getVertexProgramParameters(), GPV_ALL);
        }
        if (r2vbPass->hasFragmentProgram())
        {
            targetRenderSystem->bindGpuProgramParameters(GPT_FRAGMENT_PROGRAM,
                                                         r2vbPass->getFragmentProgramParameters(), GPV_ALL);
        }
        if (r2vbPass->hasGeometryProgram())
        {
            targetRenderSystem->bindGpuProgramParameters(GPT_GEOMETRY_PROGRAM,
                                                         r2vbPass->getGeometryProgramParameters(), GPV_ALL);
        }
        OGRE_CHECK_GL_ERROR(glBeginQueryEXT(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mPrimitivesDrawnQuery));

        OGRE_CHECK_GL_ERROR(glBeginTransformFeedback(getR2VBPrimitiveType(mOperationType)));

        targetRenderSystem->_render(renderOp);
        
        OGRE_CHECK_GL_ERROR(glEndTransformFeedback());

        // Finish the query
        OGRE_CHECK_GL_ERROR(glEndQueryEXT(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN));

        OGRE_CHECK_GL_ERROR(glDisable(GL_RASTERIZER_DISCARD));

        // Read back query results
        GLuint primitivesWritten;
        OGRE_CHECK_GL_ERROR(glGetQueryObjectuivEXT(mPrimitivesDrawnQuery, GL_QUERY_RESULT, &primitivesWritten));
        mVertexData->vertexCount = primitivesWritten * getVertexCountPerPrimitive(mOperationType);

        // Switch the vertex binding if necessary
        if (targetBufferIndex != mFrontBufferIndex)
        {
            mVertexData->vertexBufferBinding->unsetAllBindings();
            mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffers[targetBufferIndex]);
            mFrontBufferIndex = targetBufferIndex;
        }

        // Enable rasterization
        OGRE_CHECK_GL_ERROR(glDisable(GL_RASTERIZER_DISCARD));

        // Clear the reset flag
        mResetRequested = false;
    }
//-----------------------------------------------------------------------------
    void GLES2RenderToVertexBuffer::reallocateBuffer(size_t index)
    {
        assert(index == 0 || index == 1);
        if (mVertexBuffers[index])
        {
            mVertexBuffers[index].reset();
        }
        
        mVertexBuffers[index] = HardwareBufferManager::getSingleton().createVertexBuffer(
            mVertexData->vertexDeclaration->getVertexSize(0), mMaxVertexCount, 
#if OGRE_DEBUG_MODE
            //Allow to read the contents of the buffer in debug mode
            HardwareBuffer::HBU_DYNAMIC
#else
            HardwareBuffer::HBU_STATIC_WRITE_ONLY
#endif
            );
    }
//-----------------------------------------------------------------------------
    String GLES2RenderToVertexBuffer::getSemanticVaryingName(VertexElementSemantic semantic, unsigned short index)
    {
        switch (semantic)
        {
        case VES_POSITION:
            return "gl_Position";
        case VES_TEXTURE_COORDINATES:
            return String("oUv") + StringConverter::toString(index);
        case VES_DIFFUSE:
            return "oColour";
        case VES_SPECULAR:
            return "oSecColour";
        //TODO : Implement more?
        default:
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Unsupported vertex element semantic in render to vertex buffer",
                "OgreGLES2RenderToVertexBuffer::getSemanticVaryingName");
        }
    }
//-----------------------------------------------------------------------------
    void GLES2RenderToVertexBuffer::bindVerticesOutput(Pass* pass)
    {
        VertexDeclaration* declaration = mVertexData->vertexDeclaration;
        size_t elemCount = declaration->getElementCount();

        if (elemCount > 0)
        {
            GLuint linkProgramId = 0;
            // Have GLSL shaders, using varying attributes
            GLSLESProgramCommon* linkProgram = GLSLESProgramManager::getSingleton().getActiveProgram();
            linkProgramId = linkProgram->getGLProgramHandle();

            // Note: 64 is the minimum number of interleaved attributes allowed by GL_EXT_transform_feedback
            // So we are using it. Otherwise we could query during rendersystem initialisation and use a dynamic sized array.
            // But that would require C99.
            const GLchar *names[64];
            for (unsigned short e = 0; e < elemCount; e++)
            {
                const VertexElement* element = declaration->getElement(e);
                String varyingName = getSemanticVaryingName(element->getSemantic(), element->getIndex());
                names[e] = varyingName.c_str();
            }

            OGRE_CHECK_GL_ERROR(glTransformFeedbackVaryings(linkProgramId, elemCount, names, GL_INTERLEAVED_ATTRIBS));
            OGRE_CHECK_GL_ERROR(glLinkProgram(linkProgramId));
        }
    }
}
