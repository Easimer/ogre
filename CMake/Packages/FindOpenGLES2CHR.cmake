#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find OpenGLES and EGL
# If using ARM Mali emulation you can specify the parent directory that contains the bin and include directories by
# setting the MALI_SDK_ROOT variable in the environment.
#
# For AMD emulation use the AMD_SDK_ROOT variable
#
# Once done this will define
#
#  OPENGLES2CHR_FOUND        - system has OpenGLES
#  OPENGLES2CHR_INCLUDE_DIR  - the GL include directory
#  OPENGLES2CHR_LIBRARIES    - Link these to use OpenGLES

FIND_PATH(OPENGLES2CHR_INCLUDE_DIR
      NAMES GLES2/gl2.h
      NO_DEFAULT_PATH
)

IF (WIN32)
  FIND_LIBRARY(OPENGLES2CHR_gl_LIBRARY
      NAMES electron.lib
      NO_DEFAULT_PATH
  )
ELSE ()
  SET( OPENGLES2CHR_gl_LIBRARY "" )
ENDIF ()

IF(OPENGLES2CHR_gl_LIBRARY)
    SET( OPENGLES2CHR_LIBRARIES ${OPENGLES2CHR_gl_LIBRARY} ${OPENGLES2CHR_LIBRARIES})
    SET( OPENGLES2CHR_FOUND TRUE )
ENDIF(OPENGLES2CHR_gl_LIBRARY)
