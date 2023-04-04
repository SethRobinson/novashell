
#ifndef header_label_style_Bitmap
#define header_label_style_Bitmap

#if _MSC_VER > 1000
#pragma once
#endif

#include <API/gui.h>

class CL_StyleManager_Bitmap;
class CL_DomElement;

class CL_Label_Bitmap : public CL_ComponentStyle
{
public:
    enum LabelType {
        type1,
        type2
    };
public:
	CL_Label_Bitmap(
		CL_Label *label,
		CL_StyleManager_Bitmap *style,
        LabelType label_type = type1);

	virtual ~CL_Label_Bitmap();

private:
	void on_paint();
	void on_get_preferred_size(CL_Size &size);
	void on_set_options(const CL_DomElement &options);

	CL_Label *label_;
    LabelType label_type_;
	
	CL_ResourceManager *resources_;
	CL_Font *font_;
    CL_SlotContainer slots_;
};

#endif // header_label_style_Bitmap
