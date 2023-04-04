
#ifndef header_checkbox_style_Bitmap
#define header_checkbox_style_Bitmap

#if _MSC_VER > 1000
#pragma once
#endif

#include <API/gui.h>
#include <API/core.h>

class CL_CheckBox;
class CL_StyleManager_Bitmap;
class CL_ResourceManager;

class CL_CheckBox_Bitmap : public CL_ComponentStyle
{
public:
	CL_CheckBox_Bitmap(
		CL_CheckBox *button,
		CL_StyleManager_Bitmap *style);

	virtual ~CL_CheckBox_Bitmap();

private:
	CL_Slot slot_paint;
	void on_paint(CL_SlotParent_v0 &super);

	CL_Slot slot_get_preferred_size;
	void on_get_preferred_size(CL_Size &size);

	CL_CheckBox *checkbox;
	
	CL_StyleManager_Bitmap *style;
	CL_ResourceManager *resources;

	CL_Font *font;

	CL_Surface *surface_left;
	CL_Surface *surface_right;
	CL_Surface *surface_ticker;
};

#endif
