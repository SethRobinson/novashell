
#ifndef header_window_style_Bitmap
#define header_window_style_Bitmap

#if _MSC_VER > 1000
#pragma once
#endif

#include <API/gui.h>

class CL_Window;
class CL_ComponentOptions;
class CL_StyleManager_Bitmap;
class CL_ResourceManager;
class CL_ComponentMoveHandler;

class CL_Window_Bitmap : public CL_ComponentStyle
{
public:
	CL_Window_Bitmap(CL_Window *window,	CL_StyleManager_Bitmap *style);

	virtual ~CL_Window_Bitmap();

private:
	void on_paint();
	void on_quit();
	void on_resize(int old_width, int old_height);
	void on_set_options(const CL_DomElement &options);

	CL_Rect GetTitleRect();

		CL_SlotContainer slots;

	CL_Slot slot_paint;
	CL_Slot slot_quit;

	CL_Button *button_quit;
	CL_Window *window;
	CL_ComponentMoveHandler * move_handler;
	CL_ResourceManager *resources;
	CL_StyleManager_Bitmap *style;
    CL_Font * title_font;

    CL_Rect rc_title;
    CL_Rect rc_main;
    bool first_paint;
	bool draw_bg;
};

#endif
