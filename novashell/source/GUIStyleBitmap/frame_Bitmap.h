
#ifndef header_frame_style_Bitmap
#define header_frame_style_Bitmap

#if _MSC_VER > 1000
#pragma once
#endif

#include <API/gui.h>

class CL_ComponentOptions;
class CL_StyleManager_Bitmap;

class CL_Frame_Bitmap : public CL_ComponentStyle
{
public:
	CL_Frame_Bitmap(
		CL_Frame *frame,
		CL_StyleManager_Bitmap *style);

private:
	void on_draw();

	CL_Frame *frame;
	CL_Slot slot_draw;
	CL_StyleManager_Bitmap *style;
};

#endif
