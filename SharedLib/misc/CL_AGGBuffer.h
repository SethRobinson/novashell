
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 1/8/2005 1:10:23 PM
*/


#ifndef CL_AGGBUFFER_H
#define CL_AGGBUFFER_H

#include "../core.h"
#include "../display.h"
#include "../MiscUtils.h"

#include "agg_pixfmt_rgb24.h"
#include "agg_pixfmt_rgba32.h"
#include "agg_pixfmt_amask_adaptor.h"
#include "agg_alpha_mask_u8.h"

#include "agg_renderer_primitives.h"
#include "agg_renderer_base.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"
#include "agg_rounded_rect.h"
#include "agg_conv_stroke.h"   
#include "agg_ellipse.h"
#include "agg_pattern_filters_rgba8.h" 
#include "agg_renderer_outline_image.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_path_storage.h"
#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_span_pattern_rgba32.h"


class AGGPoint
{
public:
    AGGPoint(){};
    
    AGGPoint(float x,float y):m_x(x), m_y(y)
    {
    }
    
    float m_x,m_y;
};

class AGGSimplePath
{
public:
    
    
    AGGSimplePath()
    {
        m_start = true;
        
    }
    void rewind(unsigned id) 
    { 
        m_start = true;
        m_cur_index = 0;
    }
    
    unsigned vertex(double* x, double* y)
    {
        if (m_cur_index > m_vPoint.size()) return agg::path_cmd_stop; //all done
        
        
        *x = m_vPoint[m_cur_index].m_x;
        *y = m_vPoint[m_cur_index].m_y;
        
        if(m_start) 
        {
            m_start = false;
            return agg::path_cmd_move_to;
        }
        
        return agg::path_cmd_line_to;
    }
    
    void AddPoint(float x, float y)
    {
        m_vPoint.push_back(AGGPoint(x,y));
    }
    
private:
    bool   m_start;
    std::vector<AGGPoint> m_vPoint;
    unsigned int m_cur_index;
};





class CL_AGGBuffer
{
public:
    typedef agg::pixfmt_rgba32 pixformat; 
    typedef agg::renderer_base<pixformat> renderer_base;
    

    CL_AGGBuffer();
    CL_AGGBuffer(CL_PixelBuffer * p_pixel_buff);

    virtual ~CL_AGGBuffer();

    void Init(CL_PixelBuffer * p_pixel_buff); 
    void Kill();
    
    void FillBackgroundColor(CL_Color);
    void DrawSprites();
    bool RenderTextureToBackground(CL_Surface *pSurface);
    CL_PixelBuffer * GetPixelBuffer() {return m_pPixelBuffer;}
    void RenderTexturedPathSimple(AGGSimplePath &path, CL_Surface *pSurface, int i_width);
    void RenderTexturedPath(agg::path_storage path, CL_Surface *pSurface, int i_width, int StartingIndex);
    bool RenderTextureToPoly(CL_Surface *pSurface, agg::path_storage &path);

    void lock();
    void unlock();

private:
   
   void OneTimeInit();

   int m_bytes_pp; //bytes per pixel (not bits!!!)

	agg::scanline_u8              m_sl_u8;
    agg::rasterizer_scanline_aa<> m_ras;
    
    pixformat * m_p_pixf;
    agg::renderer_base<pixformat> * m_p_rbase;
    agg::rendering_buffer * m_p_render_buf;
    CL_PixelBuffer * m_pPixelBuffer;
};

#endif
