#include "CL_AGGBuffer.h"

CL_AGGBuffer::CL_AGGBuffer()
{
   OneTimeInit();
}

void CL_AGGBuffer::OneTimeInit()
{
    m_bytes_pp = 0;
    m_p_pixf = 0;
    m_p_rbase = 0;
    m_p_render_buf = 0;
    m_pPixelBuffer = NULL;

}
CL_AGGBuffer::CL_AGGBuffer(CL_PixelBuffer * p_pixel_buff)
{
    OneTimeInit();
    Init(p_pixel_buff);
}

CL_AGGBuffer::~CL_AGGBuffer()
{
    Kill();
}


void CL_AGGBuffer::Kill()
{
    SAFE_DELETE(m_p_rbase);
    SAFE_DELETE(m_p_pixf);
    SAFE_DELETE(m_p_render_buf);
    SAFE_DELETE(m_pPixelBuffer);
}


void CL_AGGBuffer::lock()
{
    m_pPixelBuffer->lock();
}

void CL_AGGBuffer::unlock()
{
    m_pPixelBuffer->unlock();

}

void CL_AGGBuffer::Init(CL_PixelBuffer * p_pixel_buff)
{
    Kill();

    m_pPixelBuffer = p_pixel_buff;
    std::string stErrorMsg = "AGG Init Error";

    m_bytes_pp = p_pixel_buff->get_format().get_depth()/8;
    cl_assert(m_bytes_pp == 4 && "We only support 32 bit 8888 format right now.");
  
  // FillBackgroundColor(CL_Color(0,0,255));
    
    m_p_render_buf = new agg::rendering_buffer((byte*)p_pixel_buff->get_data(), 
        p_pixel_buff->get_width(), 
        p_pixel_buff->get_height(), 
        p_pixel_buff->get_pitch());
    if (!m_p_render_buf) throw CL_Error(stErrorMsg);
   
    m_p_pixf = new pixformat(*m_p_render_buf);
    if (!m_p_pixf) throw CL_Error(stErrorMsg);
  
    m_p_rbase = new agg::renderer_base<pixformat>(*m_p_pixf);
    if (!m_p_rbase) throw CL_Error(stErrorMsg);
   

}
void CL_AGGBuffer::DrawSprites()
{
    
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
    renderer_solid rs(*m_p_rbase);
    
    // static double x = 1;
    
    agg::ellipse ellip;
    double x = 200;
    
    ellip.init(x,200,200,x/10,100);
    
    //    x = altfmod(x+0.1, GetEngine()->GetScreenX());
    
    m_ras.add_path(ellip);
    rs.color(agg::rgba(0,0,1,0.5));
    
    agg::render_scanlines(m_ras, m_sl_u8, rs);
    
    m_ras.reset();
    
}


namespace agg
{
    class pattern_image_pixmap_argb32
    {
    public:
        typedef rgba8 color_type;
        
        pattern_image_pixmap_argb32(CL_PixelBuffer *p_pb)
        {
          m_PixelBuff = p_pb;
          m_x = m_PixelBuff->get_width();
          m_y = m_PixelBuff->get_height();
          m_data = (unsigned int *)m_PixelBuff->get_data();
          cl_assert(m_PixelBuff->get_pitch() == m_x *4 && "You need to add support for pitch now!");
        }
        
        unsigned width()  const { return m_x; }
        unsigned height() const { return m_y; }
      
        rgba8 pixel(int x, int y) const
        {
            
            int32u p =  m_data[(y * m_x) + x];
         /*   
            //simplifed version for debugging
             
            int r = (p & 0xFF);
            int g = (p >> 8) & 0xFF;
            int b = (p >> 16) & 0xFF;
            int a = (p >> 24) & 0xFF;

            
           return rgba8(r,g,b,a);
           */

           return rgba8(p & 0xFF, (p >> 8) & 0xFF, (p >> 16) & 0xFF,(p >> 24) & 0xFF);
   
        }
    private:
        int32u * m_data;
        unsigned m_x,m_y;
        CL_PixelBuffer *m_PixelBuff;
    };
}



namespace agg
{
    class pattern_pixmap_argb32
    {
    public:
        typedef rgba8 color_type;
        
        pattern_pixmap_argb32(const int32u* pixmap) : m_pixmap(pixmap) {}
        
        unsigned width()  const { return m_pixmap[0]; }
        unsigned height() const { return m_pixmap[1]; }
        
        rgba8 pixel(int x, int y) const
        {
            int32u p = m_pixmap[y * width() + x + 2];
            return rgba8((p >> 16) & 0xFF, (p >> 8) & 0xFF, p & 0xFF, p >> 24);
        }
    private:
        const int32u* m_pixmap;
    };
}


static const agg::int32u pixmap_chain[] = 
{
    16, 6,
        0xffffffff, 0xffffffff, 0xfffbf9f9, 0xff9a5757, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xb4c29999, 0x00ffffff, 0x00ffffff, 0x00ffffff, 
        0x00ffffff, 0x5ae0cccc, 0xffa46767, 0xff660000, 0xff975252, 0x7ed4b8b8, 0x5ae0cccc, 0x5ae0cccc, 0x5ae0cccc, 0x5ae0cccc, 0xa8c6a0a0, 0xff7f2929, 0xff670202, 0x9ecaa6a6, 0x5ae0cccc, 0x00ffffff, 
        0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xa4c7a2a2, 0x3affff00, 0x3affff00, 0xff975151, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 
        0x00ffffff, 0x5ae0cccc, 0xffa46767, 0xff660000, 0xff954f4f, 0x7ed4b8b8, 0x5ae0cccc, 0x5ae0cccc, 0x5ae0cccc, 0x5ae0cccc, 0xa8c6a0a0, 0xff7f2929, 0xff670202, 0x9ecaa6a6, 0x5ae0cccc, 0x00ffffff, 
        0x00ffffff, 0x00ffffff, 0x0cfbf9f9, 0xff9a5757, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xff660000, 0xb4c29999, 0x00ffffff, 0x00ffffff, 0x00ffffff, 
        0x00ffffff, 0xffffffff, 0x00ffffff, 0x00ffffff, 0xb4c29999, 0xff9a5757, 0xff9a5757, 0xff9a5757, 0xff9a5757, 0xff9a5757, 0xff9a5757, 0xb4c29999, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff
};





class spiral
{
public:
    spiral(double x, double y, double r1, double r2, double step, double start_angle=0) :
      m_x(x), 
          m_y(y), 
          m_r1(r1), 
          m_r2(r2), 
          m_step(step), 
          m_start_angle(start_angle),
          m_angle(start_angle),
          m_da(agg::deg2rad(8.0)),
          m_dr(m_step / 45.0)
      {
      }
      
      void rewind(unsigned id) 
      { 
          m_angle = m_start_angle; 
          m_curr_r = m_r1; 
          m_start = true; 
      }
      
      unsigned vertex(double* x, double* y)
      {
          if(m_curr_r > m_r2) return agg::path_cmd_stop;
          
          *x = m_x + cos(m_angle) * m_curr_r;
          *y = m_y + sin(m_angle) * m_curr_r;
          m_curr_r += m_dr;
          m_angle += m_da;
          if(m_start) 
          {
              m_start = false;
              return agg::path_cmd_move_to;
          }
          return agg::path_cmd_line_to;
      }
      
private:
    double m_x;
    double m_y;
    double m_r1;
    double m_r2;
    double m_step;
    double m_start_angle;
    
    double m_angle;
    double m_curr_r;
    double m_da;
    double m_dr;
    bool   m_start;
};



void CL_AGGBuffer::RenderTexturedPathSimple(AGGSimplePath &path, CL_Surface *pSurface, int i_width)
{
    CL_PixelBuffer PixelBuf = pSurface->get_pixeldata();
    PixelBuf.lock();
    
    cl_assert(PixelBuf.get_format().get_depth() /8 == m_bytes_pp && "This needs to be 32 bit.");
 
    //setup a rendering buffer to be used as a source texture
    agg::rendering_buffer m_buff_image;
    
      m_buff_image.attach((byte*)PixelBuf.get_data(),
        PixelBuf.get_width()-1, PixelBuf.get_height()-1, PixelBuf.get_pitch());
  //   i_width = 50;
 //   agg::conv_stroke<agg::path_storage> stroke(path);
   // stroke.width(i_width);
 //  stroke.line_join(agg::round_join);
 // stroke.line_cap(agg::round_cap);
   
    typedef agg::pattern_filter_bilinear_rgba8 pattern_filter;
    typedef agg::line_image_pattern_pow2<pattern_filter> image_pattern;
    typedef agg::renderer_outline_image<renderer_base, image_pattern> renderer_img;
    
    typedef agg::rasterizer_outline_aa<renderer_img> rasterizer_outline_img;
      
    pattern_filter filter;
   agg::pattern_image_pixmap_argb32 src(&PixelBuf);
   agg::line_image_scale<agg::pattern_image_pixmap_argb32> src_scaled(src, 50);
 
 //   agg::pattern_pixmap_argb32 src(pixmap_chain);
  //  agg::line_image_scale<agg::pattern_pixmap_argb32> src_scaled(src, 15);
       
    image_pattern pattern(filter);
     
    pattern.create(src_scaled);

    renderer_img ren_img( *m_p_rbase, pattern);
    rasterizer_outline_img ras_img(ren_img);
    ren_img.scale_x(src.width() / src.height());
   
 //  spiral s5(100, 200, 10, 300, 50, 0);
 //  ras_img.add_path(s5);
  
    ras_img.add_path(path);
   

    PixelBuf.unlock();
   
}
  
//pass zero width to not rescale the image
void CL_AGGBuffer::RenderTexturedPath(agg::path_storage path, CL_Surface *pSurface, int i_width, int StartingIndex)
{
    CL_PixelBuffer PixelBuf = pSurface->get_pixeldata();
    PixelBuf.lock();
    
    cl_assert(PixelBuf.get_format().get_depth() /8 == m_bytes_pp && "This needs to be 32 bit.");
    
    //setup a rendering buffer to be used as a source texture
    agg::rendering_buffer m_buff_image;
    
    m_buff_image.attach((byte*)PixelBuf.get_data(),
        PixelBuf.get_width()-1, PixelBuf.get_height()-1, PixelBuf.get_pitch());
 //   i_width = 50;
   // agg::conv_stroke<agg::path_storage> stroke(path);
  //  stroke.width(i_width);
    //  stroke.line_join(agg::round_join);
    // stroke.line_cap(agg::round_cap);
    
    typedef agg::pattern_filter_bilinear_rgba8 pattern_filter;
    typedef agg::line_image_pattern_pow2<pattern_filter> image_pattern;
    typedef agg::renderer_outline_image<renderer_base, image_pattern> renderer_img;
    
    typedef agg::rasterizer_outline_aa<renderer_img> rasterizer_outline_img;
    
    pattern_filter filter;
    agg::pattern_image_pixmap_argb32 src(&PixelBuf);
    
    //   agg::pattern_pixmap_argb32 src(pixmap_chain);
    //  agg::line_image_scale<agg::pattern_pixmap_argb32> src_scaled(src, 15);
    
    image_pattern pattern(filter);
  
    if (i_width != 0)
    {
        agg::line_image_scale<agg::pattern_image_pixmap_argb32> src_scaled(src, i_width);
         pattern.create(src_scaled);
    } else
    {
        pattern.create(src);

    }
    
    renderer_img ren_img( *m_p_rbase, pattern);
    rasterizer_outline_img ras_img(ren_img);
    ren_img.scale_x(src.width() / src.height());
    
    //  spiral s5(100, 200, 10, 300, 50, 0);
    //  ras_img.add_path(s5);
    
    ras_img.add_path(path, StartingIndex);
   
    PixelBuf.unlock();
    
}
bool CL_AGGBuffer::RenderTextureToPoly(CL_Surface *pSurface, agg::path_storage &path)
{
  
    CL_PixelBuffer PixelBuf = pSurface->get_pixeldata();
    PixelBuf.lock();
    
    cl_assert(PixelBuf.get_format().get_depth() /8 == m_bytes_pp && "This needs to be 32 bit.");
 
    //we've loaded a jpg and that's all fine, but the alpha channel is set to nothing by default.  Let's set it to 255 so we can see
    //this thing.
  /* 
    for (int y =0; y < PixelBuf.get_height(); y++)
    {
        int x = PixelBuf.get_width();
        byte *p_surf = ((byte*)PixelBuf.get_data()) + (PixelBuf.get_pitch() * y);
        p_surf += 3;
        while (--x)
        {
            *p_surf = 255;
            p_surf += 4; //advance to the next alpha
        }
    }
    */

    //setup a rendering buffer to be used as a source texture
    agg::rendering_buffer m_buff_image;
   
    m_buff_image.attach((byte*)PixelBuf.get_data(),
        PixelBuf.get_width()-1, PixelBuf.get_height()-1, PixelBuf.get_pitch());
    
    //it's loaded, now setup it up as a generic pattern buffer to be used as a texture with AGG
    
    
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
    renderer_solid rs(*m_p_rbase);
    
    typedef agg::span_pattern_rgba32<agg::order_rgba32> span_gen_type;
    typedef agg::renderer_scanline_aa<renderer_base, span_gen_type> renderer_type;
    
    agg::span_allocator<agg::rgba8> sa;
    span_gen_type span_generic(sa, m_buff_image, 0, 0);
    
    // span_generic.alpha(span_gen_type::alpha_type(255)); 
    
    rs.color(agg::rgba(1,1,1));
    m_ras.add_path(path);
    
    renderer_type rp(*m_p_rbase, span_generic);
    agg::render_scanlines(m_ras, m_sl_u8, rp);
    PixelBuf.unlock();
 
    return true; //success
}


bool CL_AGGBuffer::RenderTextureToBackground(CL_Surface *pSurface)
{
    
    agg::path_storage path;
    
    path.move_to(0,0);
    path.line_to(m_pPixelBuffer->get_width(),0);
    path.line_to(m_pPixelBuffer->get_width(),m_pPixelBuffer->get_height());
    path.line_to(0,m_pPixelBuffer->get_height());
    path.close_polygon();
    
   return RenderTextureToPoly(pSurface, path);
}



void CL_AGGBuffer::FillBackgroundColor(CL_Color color)
{
    cl_assert(m_pPixelBuffer != NULL && "Call init first!");
   
    int copy_count = m_pPixelBuffer->get_pitch()/4;

    unsigned int *p_data = (unsigned int*)m_pPixelBuffer->get_data();
    
    for (int y=0; y < m_pPixelBuffer->get_height(); y++)
    {
        for (int x=0; x < copy_count; x++)
        {
            *p_data = color.color;
            p_data++;
        }

    }
    
}





