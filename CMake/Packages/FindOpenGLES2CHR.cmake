#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find OpenGLES2CHR
#
# Once done this will define
#
#  OPENGLES2CHR_FOUND         - system has OpenGLES
#  OpenGLES2CHR::OpenGLES2CHR - the target to link against

find_path(OPENGLES2CHR_INCLUDE_DIR
      NAMES GLES2/gl2.h
      NO_DEFAULT_PATH
)

if(WIN32)
  find_library(OPENGLES2CHR_gl_LIBRARY
      NAMES electron.lib
      NO_DEFAULT_PATH
  )
else()
  set(OPENGLES2CHR_gl_LIBRARY "")
endif()

if(OPENGLES2CHR_gl_LIBRARY)
    add_library(opengles2chr INTERFACE)
    target_include_directories(opengles2chr INTERFACE ${OPENGLES2CHR_INCLUDE_DIR})
    target_link_libraries(opengles2chr INTERFACE ${OPENGLES2CHR_gl_LIBRARY} ${OPENGLES2CHR_LIBRARIES})
    add_library(OpenGLES2CHR::OpenGLES2CHR ALIAS opengles2chr)
    SET( OPENGLES2CHR_FOUND TRUE )
endif()
