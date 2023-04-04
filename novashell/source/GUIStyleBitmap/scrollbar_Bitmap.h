
#ifndef header_scrollbar_style_Bitmap
#define header_scrollbar_style_Bitmap

#if _MSC_VER > 1000
#pragma once
#endif

#include <API/gui.h>
#include <API/core.h>
#include <API/signals.h>

class CL_ScrollBar;
class CL_ComponentOptions;
class CL_StyleManager_Bitmap;
class CL_ResourceManager;

class CL_ScrollBar_Bitmap : public CL_ComponentStyle
{
public:
    enum ScrollBarType {
        type_normal,
        type_listbox
    };
public:
	CL_ScrollBar_Bitmap(
		CL_ScrollBar *scrollbar,
		CL_StyleManager_Bitmap *style,
        ScrollBarType scrollbar_type = type_normal);
	
	virtual ~CL_ScrollBar_Bitmap();
public:
    void set_type(ScrollBarType scrollbar_type);
private:
	void start_scroll(int delta);
	void stop_scroll();

	void on_paint();
	void on_timer_scroll();

    void on_get_preferred_size(CL_Size &size);

	CL_Timer timer_scroll_;
	int scroll_delta_;

	CL_ScrollBar *scrollbar_;

	CL_Button *button_increase_;
	CL_Button *button_decrease_;

	CL_StyleManager_Bitmap *style_;
	CL_ResourceManager *resources_;
	CL_Surface *sur_increase_normal_;
	CL_Surface *sur_increase_toggled_;
	CL_Surface *sur_decrease_normal_;
	CL_Surface *sur_decrease_toggled_;

    CL_SlotContainer slots_;
    ScrollBarType scrollbar_type_;

    bool initialized_;
};

#endif

