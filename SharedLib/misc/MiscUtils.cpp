#include "MiscUtils.h"
#include <sys/stat.h>
#include <time.h>

float RadiansToDegrees(float rad)
{
	const float k = 180.0f / M_PI;
	return rad * k;
}

inline float DegreesToRadians(float deg)
{
	const float k = M_PI / 180.0f;
	return deg * k;
}
string GetNextLineFromFile(FILE *fp)
{
	string line;
	char c;

	while (!feof(fp))
	{
		fread(&c,1,1,fp);

		if (c == '\r') continue; //don't care about these stupid things

		line += c;

		if (c == '\n')
		{
			return line; //go ahead and quit now, we found a cr
		}
	}
	return line;
}

string GetNextLineFromFile(CL_InputSource *pInput)
{
	string line;
	char c;

	while (pInput->tell() < pInput->size())
	{
		c = pInput->read_char8();

		if (c == '\r') continue; //don't care about these stupid things

		line += c;

		if (c == '\n')
		{
			return line; //go ahead and quit now, we found a cr
		}
	}
	return line;
}

int random(int range)
{
	return static_cast<int>(double(rand()) / (RAND_MAX) * range);
}

int random_range(int rangeMin, int rangeMax)
{
	return static_cast<int>(double(rand()) / RAND_MAX * (rangeMax+1-rangeMin)+rangeMin);
}


bool RemoveFile(const std::string &fileName)
{

#ifdef WIN32
	if (DeleteFile(fileName.c_str()) == 0)
		return false;
#else
	if (remove(fileName.c_str()) != 0)
		return false;
#endif
	return true;
}

#ifdef WIN32

bool open_file(HWND hWnd, const char st_file[])
{
#ifndef _NOSHELL32

	if (!FileExists(st_file))
	{
		//file doesn't exist
		return false;

	}
	int result = (int)ShellExecute(NULL,"open",st_file, NULL,NULL, SW_SHOWDEFAULT  );

	//	Msg("Result is %d.",result);
	if ( (result < 32) && (result != 2))
	{
		//big fat error.

		std::string s;
		s = std::string("Windows doesn't know how to open ")+std::string(st_file)+ 
			std::string("\n\nYou need to use file explorer and associate this file type with something first.");

#ifdef WIN32
		MessageBox(hWnd, s.c_str(), st_file, MB_ICONSTOP);
#else
LogError(s.c_str());
#endif

		return false;
	}

	return true;

#else

	LogError("You must remove _NOSHELL32 to build with this in the all.cpp lib.");
	return false;
#endif
}

#endif


//a more useful mod
//SETH: I simplified this for the ARM compiler 8-7-03
int altmod(int a, int b)
{
	int const rem = a % b;
	if ( (-1 % 2 == 1) || rem >= 0)
		return rem; else
		return rem + abs(b);
}

double altfmod(double a, double b)
{

	double const rem = fmod(a, b);
	if ( (-1 % 2 == 1) || rem >= 0)
		return rem; else
		return rem + fabs(b);
}

/* Add text adds a line of text to a text file.  It creates it if it doesn't
exist. */

void add_text(const char *tex ,const char *filename)
{
	if ( (tex == NULL) || ( filename == NULL) || ( filename[0] == 0))
	{
		//assert(0);
		return;
	}

	FILE *          fp = NULL;
	if (strlen(tex) < 1) return;
	if (FileExists(filename) == false)
	{

		fp = fopen(filename, "wb");
		if (!fp)
		{
			return;
		}
		fwrite( tex, strlen(tex), 1, fp);       
		fclose(fp);
		return;
	} else
	{
		fp = fopen(filename, "ab");
		fwrite( tex, strlen(tex), 1, fp);      
		fclose(fp);
	}
}


bool FileExists(string file)
{
	CL_InputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_InputSource_File(file);

	} catch(CL_Error error)
	{
		//file doesn't exist
		delete pFile;
		return false;
	}
	delete pFile;
	return true;
}


//snippet from Zahlman's post on gamedev:  http://www.gamedev.net/community/forums/topic.asp?topic_id=372125
void StringReplace(const std::string& what, const std::string& with, std::string& in)
{
	int pos = 0;
	int whatLen = what.length();
	int withLen = with.length();
	while ((pos = in.find(what, pos)) != std::string::npos)
	{
		in.replace(pos, whatLen, with);
		pos += withLen;
	}
}

void CreateDirectoryRecursively(string basePath, string path)
{
	StringReplace("\\", "/", path);
	StringReplace("\\", "/", basePath);

	vector<string> tok = CL_String::tokenize(path, "/", true);

	if (basePath[basePath.size()-1] != '/') basePath += "/";
	path = "";
	for (unsigned int i=0; i < tok.size(); i++)
	{
		path += tok[i].c_str();
		//LogMsg("Creating %s%s", basePath.c_str(), path.c_str());
		CL_Directory::create(basePath+path);
		path += "/";
	}

}

void UnzipToDir(CL_Zip_Archive &zip, string outputDir)
{

	vector<unsigned char> buff;
	buff.resize(4096);

	std::vector<CL_Zip_FileEntry> &file_list = zip.get_file_list();

	string path;

	for (unsigned int i=0; i < file_list.size(); i++)
	{
		if (file_list[i].get_filename()[file_list[i].get_filename().length()-1] == '/')
		{
			//it's a directory
		
			//CL_Directory::create(outputDir+'/'+file_list[i].get_filename());
		
		} else
		{

			//in a perfect world we'd always get our directories first, but it seems some zip utilities don't order them this way..
			//first make sure the dir exists
			
			string dir = CL_String::get_path(file_list[i].get_filename());
			if (dir != ".")
			{
				CreateDirectoryRecursively(outputDir, dir);
			}


			//file to write

			CL_OutputSource_File *pFile = NULL;

			try
			{
				pFile = new CL_OutputSource_File(outputDir+'/'+file_list[i].get_filename());

			} catch(CL_Error error)
			{
				delete pFile;
				LogError("Unable to create file %s/%s", outputDir.c_str(), file_list[i].get_filename().c_str());
				return;
			}

			CL_InputSource *pInput = zip.open_source(file_list[i].get_filename());

			//do the copy...
			unsigned int bytesRead = 0;
			while ( (bytesRead = pInput->read(&buff[0], buff.size()))  > 0)
			{
				pFile->write(&buff[0], bytesRead);
				if (bytesRead < buff.size()) break;
			}

			delete (pFile);
			delete (pInput);

		}

		//LogMsg("Unzipping %s", file_list[i].get_filename().c_str());
	}
}


//this lets you apply a number to a number to make it closer to a target
//it will not go pass the target number.
void set_float_with_target(float *p_float, float f_target, float f_friction)
{
	if (*p_float != f_target)
	{
		if (*p_float > f_target)
		{
			*p_float -= f_friction;
			if (*p_float < f_target) *p_float = f_target;
		} else
		{
			*p_float += f_friction;
			if (*p_float > f_target) *p_float = f_target;
		}
	}
}


//taken from Gamedeveloper magazine's InnerProduct (Sean Barrett 2005-03-15)

// circular shift hash -- produces good results if modding by a prime;
// longword at a time would be faster (need alpha-style "is any byte 0"),
// or just use the first longword

unsigned int HashString(const char *str)
{
	unsigned char *n = (unsigned char *) str;
	unsigned int acc = 0x55555555;
	while (*n)
		acc = (acc >> 27) + (acc << 5) + *n++;
	return acc;
}


void ClearPixelBuffer(CL_PixelBuffer* pPixelBuffer, CL_Color color)
{
	cl_assert(pPixelBuffer && "Invalid buffer!");
	int bytes_pp = pPixelBuffer->get_format().get_depth()/8;
	cl_assert(bytes_pp == 4 && "We only support 32 bit 8888 format right now.");

	unsigned int dest_format_color = color.to_pixelformat(pPixelBuffer->get_format());
	int copy_count = pPixelBuffer->get_pitch()/4;

	unsigned int *p_data = (unsigned int*)pPixelBuffer->get_data();

	pPixelBuffer->lock();
	for (int y=0; y < pPixelBuffer->get_height(); y++)
	{
		for (int x=0; x < copy_count; x++)
		{
			*p_data = dest_format_color;
			p_data++;
		}
	}
	pPixelBuffer->unlock();
}

CL_Rectf CombineRects(const CL_Rectf &a, const CL_Rectf &b)
{
	CL_Rectf r;
	r.left = min(a.left, b.left);
	r.top = min(a.top, b.top);
	r.right = max(a.right, b.right);
	r.bottom = max(a.bottom, b.bottom);
	return r;
}

CL_Rect CombineRects(const CL_Rect &a, const CL_Rect &b)
{
	CL_Rect r;
	r.left = min(a.left, b.left);
	r.top = min(a.top, b.top);
	r.right = max(a.right, b.right);
	r.bottom = max(a.bottom, b.bottom);
	return r;
}

//use like this:
// string datVersion = ChangeFileExtension("crap.txt", "dat"); //now datVersion == "crap.dat";
string ChangeFileExtension(const string &input, const string &extension)
{
	int n = input.find_last_of(".");
	if (n == string::npos) return input + "." + extension;
	//otherwise..
	return input.substr(0, n)+extension;
}

time_t GetLastModifiedDateFromFile(string fileName)
{
	struct tm* clock;				// create a time structure
	struct stat attrib;			// create a file attribute structure
	stat(fileName.c_str(), &attrib);		// get the attributes of afile.txt
	clock = gmtime(&(attrib.st_mtime));	// Get the last modified time and put it into the time structure
	if (clock == 0) return 0;
	return mktime(clock);
}

//returns a range of -1 to 1 with the cycle matching the MS sent in, based on a sin wave
float SinPulseByMS(int ms)
{
	int tick = CL_System::get_time() %ms;
	return (float)(sin (   (float(tick)/float(ms))  *M_PI*2   ));
}