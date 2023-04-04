#include <API/display.h>
#include <API/gl.h>

#include "window_Bitmap.h"
#include "stylemanager_Bitmap.h"

CL_Window_Bitmap::CL_Window_Bitmap(
	CL_Window *window,
	CL_StyleManager_Bitmap *style)
:
	CL_ComponentStyle(window),
	window(window),
	style(style),
    move_handler(0),
	resources(style->get_resources()),
    title_font(0),
    first_paint(true),
	draw_bg(true)
{
	slot_paint = window->sig_paint().connect(this, &CL_Window_Bitmap::on_paint);

	button_quit = NULL;
	CL_Window::TitlebarButtons buttons = window->get_titlebar_buttons_enum();

    title_font = new CL_Font("Window/font", resources);

	
	rc_title = GetTitleRect();

     rc_main = CL_Rect(0, 0, window->get_width(), window->get_height());
	 //move_handler = new CL_ComponentMoveHandler(rc_main , window);

	if(buttons & CL_Window::close_button)
	{
		button_quit = new CL_Button(CL_Rect(window->get_width() - 12, 3, window->get_width(), 15), "X", window, style);
		slot_quit = button_quit->sig_clicked().connect(this, &CL_Window_Bitmap::on_quit);

	}

	slots.connect(window->sig_set_options(), this, &CL_Window_Bitmap::on_set_options);
	slots.connect(window->sig_resize(), this, &CL_Window_Bitmap::on_resize);

	on_resize(0,0);
}

CL_Window_Bitmap::~CL_Window_Bitmap()
{
    delete title_font;
    delete move_handler;
	delete button_quit;
}

void CL_Window_Bitmap::on_set_options(const CL_DomElement &options)
{
	std::string s;

	s = options.get_attribute("draw_bg", "");

	if (!s.empty()) 
	{
		draw_bg = CL_String::to_bool(s);
	}

}

void CL_Window_Bitmap::on_resize(int old_width, int old_height)
{
	
	if (move_handler)
	move_handler->set_position(GetTitleRect());

}

CL_Rect CL_Window_Bitmap::GetTitleRect()
{
	CL_Rect r = CL_Rect(0, 0,title_font->get_width(window->get_title()) + 16,
		title_font->get_height(window->get_title()) + 8);

	return r;
}


void CL_Window_Bitmap::on_paint()
{
	
	rc_title = GetTitleRect();

    rc_main = CL_Rect(0, rc_title.get_height()+8, window->get_width(), window->get_height());

	if (first_paint)
    {
        if (button_quit)
		{
			button_quit->set_position(CL_Rect(window->get_width() - 12,3,
            window->get_width(),15));
		}
        
		 delete move_handler;
		 move_handler = new CL_ComponentMoveHandler(rc_title, window);

        window->get_client_area()->set_position(CL_Rect(rc_main.left, rc_main.top,
            window->get_width(), window->get_height()));

        first_paint = false;
    }

	int width = window->get_width();
	int height = window->get_height();

	// Outline
	CL_Color clr_outline = style->GetColor(CL_StyleManager_Bitmap::COLOR_WINDOW_BORDER);


	if (draw_bg)
	{
		// Fill BG
		//-------
		CL_Color clr_background = style->GetColor(CL_StyleManager_Bitmap::COLOR_WINDOW_BG);
		style->fill_rounded_rect(window->client_to_screen(rc_main), clr_background);
		style->draw_rounded_rect(window->client_to_screen(rc_main), clr_outline);
	}
    
    CL_Color clr_title_bgnd = style->GetColor(CL_StyleManager_Bitmap::COLOR_WINDOW_TITLE_BG);
 
	if (!window->get_title().empty())
	{

		// title
		style->fill_rounded_rect(window->client_to_screen(rc_title), clr_title_bgnd);
		style->draw_rounded_rect(window->client_to_screen(rc_title), clr_outline);

		// draw title text
		CL_Point title_coord = window->client_to_screen(CL_Point(rc_title.left + 8, rc_title.top + 6));
		title_font->draw(title_coord.x, title_coord.y, window->get_title());
	}

}

void CL_Window_Bitmap::on_quit()
{
	if (window->is_modal())
		window->quit();
	else
        window->close();
}
