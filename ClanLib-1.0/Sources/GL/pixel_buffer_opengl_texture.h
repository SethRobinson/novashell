/*
**  ClanLib SDK
**  Copyright (c) 1997-2005 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
**    (if your name is missing here, please add it)
*/

#ifndef header_pixelbuffer_opengl_texture
#define header_pixelbuffer_opengl_texture

#if _MSC_VER > 1000
#pragma once
#endif

#include "API/Display/graphic_context.h"
#include "Display/pixel_buffer_generic.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "API/GL/opengl.h"

//: Pixel data access of a texture.
class CL_PixelBuffer_OpenGL_Texture : public CL_PixelBuffer_Generic
{
//! Construction:
public:
	CL_PixelBuffer_OpenGL_Texture(
		int offset_x,
		int offset_y,
		int width,
		int height,
		CLuint handle,
		CL_GraphicContext *gc);

	~CL_PixelBuffer_OpenGL_Texture();

//! Operations:
public:
	virtual void *get_data();

	virtual void perform_lock();

	virtual void perform_unlock();

//! Implementation:
private:
	int offset_x, offset_y;

	CLuint handle;

	CL_GraphicContext *gc;

	CLint texture_width, texture_height;

	char *data;
};

#endif
