
#ifndef header_listbox_style_Bitmap
#define header_listbox_style_Bitmap

#if _MSC_VER > 1000
#pragma once
#endif

#include <API/gui.h>
#include "listboxitem_Bitmap.h"

class CL_Font;
class CL_ListBox;
class CL_StyleManager_Bitmap;
class CL_ResourceManager;
class CL_DomElement;

class CL_ListBox_Bitmap : public CL_ComponentStyle
{
    enum ListBoxType {
        type_normal,
        type_levels_list
    };
public:
	CL_ListBox_Bitmap(
		CL_ListBox * listbox,
		CL_StyleManager_Bitmap * style);
	
	virtual ~CL_ListBox_Bitmap();
private:
	CL_StyleManager_Bitmap * style_;
	CL_ResourceManager * resources_;
	CL_Font * font_;

    bool first_paint_;
    CL_SlotContainer slots_;
    ListBoxType listbox_type_;

	void on_mouse_down(const CL_InputEvent &key);
	void on_set_options(const CL_DomElement &options);
	void on_scroll_change(int new_offset);
	void on_resize(int old_width, int old_height);
	void on_paint_background();
	void on_paint_listbox();
	void on_item_added(int index);
	void on_item_removed(int index);
	void on_clear();
	void on_get_preferred_size(CL_Size &size);
	void update_scrollbar(bool resize);

	CL_ListBox *listbox_;
	CL_ScrollBar *scrollbar_;
	
	bool visible_scrollbar_;
};

#endif
