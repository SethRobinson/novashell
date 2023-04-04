#ifndef MiscUtils_h__
#define MiscUtils_h__


/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
*/

template<class T1, class T2> inline T1 Lerp( const T1& v1, const T1& v2, const T2& lerp )
{
	return v1 + ( v2 - v1 ) * lerp;
}
 
#include "API/core.h"
#include "API/display.h"

using namespace std;
//misc stuff Seth Added

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)      { if(p) { delete (p); (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif


inline int   RandInt(int x,int y) {return rand()%(y-x+1)+x;}

//returns a random double between zero and 1
inline double RandFloat()      {return ((rand())/(RAND_MAX+1.0));}

inline double RandInRange(double x, double y)
{
	return x + RandFloat()*(y-x);
}

//returns a random bool
inline bool   RandBool()
{
	if (RandInt(0,1)) return true;

	else return false;
}


#include <string>

unsigned int HashString(const char *str);
int random(int range);
int random_range(int rangeMin, int rangeMax);
time_t GetLastModifiedDateFromFile(string fileName);

inline float frandom(float x=1.0f)
{
	return (rand() / (float) RAND_MAX) * x;
}
void StringReplace(const std::string& what, const std::string& with, std::string& in);
void add_text(const char *tex ,const char *filename);
#ifdef _WIN32
bool open_file(HWND hWnd, const char st_file[]);
#endif

int altmod(int a, int b);
double altfmod(double a, double b);

bool RemoveFile(const std::string &fileName);
void set_float_with_target(float *p_float, float f_target, float f_friction);
string GetNextLineFromFile(FILE *fp);
string GetNextLineFromFile(CL_InputSource *pInput);
bool FileExists(string file);
void CreateDirectoryRecursively(string basePath, string path);
void UnzipToDir(CL_Zip_Archive &zip, string outputDir);
CL_Rectf CombineRects(const CL_Rectf &a, const CL_Rectf &b);
CL_Rect CombineRects(const CL_Rect &a, const CL_Rect &b);

//these must exist somewhere in your project, it's how I like to write my errors
void LogMsg(const char *lpFormat, ...);
void LogError(const char *lpFormat, ...);

float RadiansToDegrees(float rad);
float DegreesToRadians(float deg);
string ChangeFileExtension(const string &input, const string &extension);

//convert any base type to a string (like int or float)
template< class C>
std::string toString(C value)
{
	std::ostringstream o;
	o << value;
	return o.str();
}

//clamps the first argument between the second two
template <class T, class U, class V>
inline void Clamp(T& arg, const U& minVal, const V& maxVal)
{
	cl_assert ( (minVal < maxVal) && "<Clamp>MaxVal < MinVal!");

	if (arg < (T)minVal)
	{
		arg = (T)minVal;
	}

	if (arg > (T)maxVal)
	{
		arg = (T)maxVal;
	}
}

//helper to make saving and loading simpler

class CL_FileHelper
{
public:

	CL_FileHelper(CL_InputSource *pInput)
	{
		m_pOutput = NULL;
		m_pInput = pInput;
		m_pInput->set_little_endian_mode();
	}

	CL_FileHelper(CL_OutputSource *pOutput)
	{
		m_pOutput = pOutput;
		m_pInput = NULL;
		m_pOutput->set_little_endian_mode();
	}

	bool IsWriting()
	{
		cl_assert(m_pOutput || m_pInput && "wtf!");
		return m_pOutput != NULL;
	}

	void process(cl_uint8 &val)
	{
		if (m_pInput) val = m_pInput->read_uint8(); else
		m_pOutput->write_uint8(val);
	}
	void process(std::string &val)
	{
		if (m_pInput)
		{	
			//first load the array count
			cl_uint32 count;
			process(count);

			if (count > 1000000)
			{
				throw CL_Error("Found invalid data");
			}
			char *pBuffer = new char[count+1];
			process_array((unsigned char*)pBuffer, count);
			pBuffer[count] = 0; //add the null at the end
			//finally, add it to the string.  I guess we could just load it
			//directly into the string to start with, but does .reserve() work
			//like that safely?

			val = pBuffer;
			SAFE_DELETE_ARRAY(pBuffer);

		} else
		{	
			//save it out
			process_smart_array((cl_uint8*)val.c_str(), val.size());
		
		}
	}

	void process_array(cl_uint8 valArray[], int size)
	{
		if (m_pInput) 
		{
			for (int i=0; i < size; i++)
			{
				valArray[i] = m_pInput->read_uint8();
			}
		} else
		{
			for (int i=0; i < size; i++)
			{
				m_pOutput->write_uint8(valArray[i]);
			}
			
		}
	}

	void process_smart_array(cl_uint8 valArray[], cl_uint32 size)
	{
		if (m_pInput) 
		{
			//first load the array count
			cl_uint32 count;
			process(count);
			cl_assert(count <= size && "Woah, big time error");
			process_array(valArray, count); //read up to whatever we've got available, let's us
			//load outdated data as long as it's smaller
		} else
		{
			process(size); //save out the size so we know how many we can load later
			process_array(valArray, size);
		}
	}

	void process_array(cl_uint32 valArray[], int size)
	{
		if (m_pInput) 
		{
			for (int i=0; i < size; i++)
			{
				valArray[i] = m_pInput->read_uint32();
			}
		} else
		{
			for (int i=0; i < size; i++)
			{
				m_pOutput->write_uint32(valArray[i]);
			}

		}
	}

	void process_smart_array(cl_uint32 valArray[], cl_uint32 size)
	{
		if (m_pInput) 
		{
			//first load the array count
			cl_uint32 count;
			process(count);
			cl_assert(count <= size && "Woah, big time error");
			process_array(valArray, count); //read up to whatever we've got available, let's us
			//load outdated data as long as it's smaller
		} else
		{
			process(size); //save out the size so we know how many we can load later
			process_array(valArray, size);
		}
	}

	
	void process_array(cl_int32 valArray[], int size)
	{
		if (m_pInput) 
		{
			for (int i=0; i < size; i++)
			{
				valArray[i] = m_pInput->read_int32();
			}
		} else
		{
			for (int i=0; i < size; i++)
			{
				m_pOutput->write_int32(valArray[i]);
			}

		}
	}

	void process_smart_array(cl_int32 valArray[], cl_uint32 size)
	{
		if (m_pInput) 
		{
			//first load the array count
			cl_uint32 count;
			process(count);
			cl_assert(count <= size && "Woah, big time error");
			process_array(valArray, count); //read up to whatever we've got available, let's us
			//load outdated data as long as it's smaller
		} else
		{
			process(size); //save out the size so we know how many we can load later
			process_array(valArray, size);
		}
	}

	void process_smart_array(CL_Vector2 valArray[], cl_uint32 size)
	{
		if (m_pInput) 
		{
			//first load the array count
			cl_uint32 count;
			process(count);
			cl_assert(count <= size && "Woah, big time error");
			process_array(valArray, count); //read up to whatever we've got available, let's us
			//load outdated data as long as it's smaller
		} else
		{
			process(size); //save out the size so we know how many we can load later
			process_array(valArray, size);
		}
	}
	void process_array(CL_Vector2 valArray[], int size)
	{
		if (m_pInput) 
		{
			for (int i=0; i < size; i++)
			{
				valArray[i].x = m_pInput->read_float32();
				valArray[i].y = m_pInput->read_float32();
			}
		} else
		{
			for (int i=0; i < size; i++)
			{
				m_pOutput->write_float32(valArray[i].x);
				m_pOutput->write_float32(valArray[i].y);
			}

		}
	}
	void process_smart_array(float valArray[], cl_uint32 size)
	{
		if (m_pInput) 
		{
			//first load the array count
			cl_uint32 count;
			process(count);
			cl_assert(count <= size && "Woah, big time error");
			process_array(valArray, count); //read up to whatever we've got available, let's us
			//load outdated data as long as it's smaller
		} else
		{
			process(size); //save out the size so we know how many we can load later
			process_array(valArray, size);
		}
	}
	void process_array(float valArray[], int size)
	{
		if (m_pInput) 
		{
			for (int i=0; i < size; i++)
			{
				valArray[i] = m_pInput->read_float32();
			}
		} else
		{
			for (int i=0; i < size; i++)
			{
				m_pOutput->write_float32(valArray[i]);
			}

		}
	}


	void process(float &val)
	{
		if (m_pInput) val = m_pInput->read_float32(); else
			m_pOutput->write_float32(val);
	}
	
	void process(double &val)
	{
		if (m_pInput) val = m_pInput->read_float64(); else
			m_pOutput->write_float64(val);
	}


	void process(CL_Vector2 &val)
	{
		if (m_pInput)
		{
			val.x = m_pInput->read_float32(); 
			val.y = m_pInput->read_float32();
		} else
		{
			m_pOutput->write_float32(val.x);
			m_pOutput->write_float32(val.y);
		}
	}

	void process(cl_int32 &val)
	{
		if (m_pInput) val = m_pInput->read_int32(); else
			m_pOutput->write_int32(val);
	}

	void process(cl_uint32 &val)
	{
		if (m_pInput) val = m_pInput->read_uint32(); else
			m_pOutput->write_uint32(val);
	}
	
	void process_const(const cl_uint32 &val)
	{
		if (m_pInput)
		{
			cl_assert(0);
		} else
			m_pOutput->write_uint32(val);
	}
	
	void process_const(const double &val)
	{
		if (m_pInput)
		{
			cl_assert(0);
		} else
			m_pOutput->write_float64(val);
	}
	
	void process_const(const cl_uint8 &val)
	{
		if (m_pInput)
		{
			cl_assert(0);
		} else
			m_pOutput->write_uint8(val);
	}

	void process_const(const cl_int32 &val)
	{
		if (m_pInput)
		{
			cl_assert(0);
		} else
			m_pOutput->write_int32(val);
	}
	
	void process_const(const CL_Vector2 &val)
	{
		if (m_pInput)
		{
			cl_assert(0);
		} else
		{
			m_pOutput->write_float32(val.x);
			m_pOutput->write_float32(val.y);
		}
	}

	void process_const(const std::string &val)
	{
		if (m_pInput)
		{	
			cl_assert(0);
		} else
		{	
			//save it out
			process_smart_array((cl_uint8*)val.c_str(), val.size());
		}
	}


	void process(CL_Rect &val)
	{
		if (m_pInput)
		{
			val.left = m_pInput->read_int32();
			val.top = m_pInput->read_int32();
			val.right = m_pInput->read_int32();
			val.bottom = m_pInput->read_int32();
		} else
		{
			m_pOutput->write_int32(val.left);
			m_pOutput->write_int32(val.top);
			m_pOutput->write_int32(val.right);
			m_pOutput->write_int32(val.bottom);
		}
	}

	void process(CL_Rectf &val)
	{
		if (m_pInput)
		{
			val.left = m_pInput->read_float32();
			val.top = m_pInput->read_float32();
			val.right = m_pInput->read_float32();
			val.bottom = m_pInput->read_float32();
		} else
		{
			m_pOutput->write_float32(val.left);
			m_pOutput->write_float32(val.top);
			m_pOutput->write_float32(val.right);
			m_pOutput->write_float32(val.bottom);
		}
	}
	void process(CL_PixelBuffer *pPix, const CL_PixelFormat &format)
	{
		cl_assert(m_pOutput && "This is only for saving, for loading, you need to send another parm");
		cl_assert(pPix->get_format().get_type() !=  pixelformat_index && "we don't support palettes!");
		unsigned int pixelSize = pPix->get_format().get_depth()/8;
		//save out the dimensions
		process_const(pPix->get_width());
		process_const(pPix->get_height());
		process_const(pPix->get_pitch());
		cl_assert(pPix->get_pitch()/3 == pPix->get_width() && "Um, this isn't a standard pitch.  Is this on video hardware or something?");
		
		pPix->lock();
		process_array((cl_uint8*)pPix->get_data(), pixelSize*pPix->get_width()*pPix->get_height());
		pPix->unlock();
	}

	//send in null pointers, we set them, you must delete both of them later
void process(CL_PixelBuffer **ppPix, cl_uint8 **ppPixBuffOut, const CL_PixelFormat &format)
{
	int width, height;
	unsigned int pitch;
	
	process(width);
	process(height);
	process(pitch);
	unsigned int pixelSize = format.get_depth()/8;
	unsigned int imageBufferSize = pixelSize*height*width;
	
	*ppPixBuffOut = new cl_uint8[imageBufferSize];
	process_array(*ppPixBuffOut, imageBufferSize);
	if (!ppPixBuffOut) throw CL_Error("Out of memory");
	*ppPix = new CL_PixelBuffer(width, height, pitch, format, *ppPixBuffOut);
}

private:

	CL_OutputSource *m_pOutput;
	CL_InputSource *m_pInput;

};

class CL_PixelBuffer;


//if you send NULL for the color it uses a special fast version (to clear to black)
void ClearPixelBuffer(CL_PixelBuffer* pPixelBuffer, CL_Color color);
float SinPulseByMS(int ms);

#endif // MiscUtils_h__

