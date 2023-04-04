#include <API/display.h>
#include <API/gl.h>

#include "frame_Bitmap.h"
#include "stylemanager_Bitmap.h"

CL_Frame_Bitmap::CL_Frame_Bitmap(
	CL_Frame *frame,
	CL_StyleManager_Bitmap *style)
:
	CL_ComponentStyle(frame),
	frame(frame),
	style(style)
{
	slot_draw = frame->sig_paint().connect(this, &CL_Frame_Bitmap::on_draw);
}

void CL_Frame_Bitmap::on_draw()
{
	CL_Display::clear(CL_Color(108, 139, 138));
}
