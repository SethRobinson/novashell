#include "CL_VirtualFileManager.h"

CL_VirtualFileSource::CL_VirtualFileSource()
{
	m_type = eFilePath;
}

bool CL_VirtualFileSource::RemoveFile(const string &fname)
{
	return ::RemoveFile(m_strPath + "/" + fname);
}


bool CL_VirtualFileSource::CreateDir(const string &fname)
{
	return CL_Directory::create(m_strPath + "/" + fname); //create it if it doesn't exist.  I'd test for existence
}

CL_OutputSource * CL_VirtualFileSource::PutFile(const string &fname)
{
	//try to open the file they want for writing
	CL_OutputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_OutputSource_File(m_strPath+"/"+fname);

	} catch(CL_Error error)
	{
		delete pFile;
		return NULL;
	}
	return pFile;
}

CL_InputSource * CL_VirtualFileSource::GetFile(const string &fname)
{
	//try to open the file they want
	CL_InputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_InputSource_File(m_strPath+"/"+fname);

	} catch(CL_Error error)
	{
		//file doesn't exist
		delete pFile;
		return NULL;
	}

	return pFile;
}


CL_VirtualFileManager::CL_VirtualFileManager()
{
}

CL_VirtualFileManager::~CL_VirtualFileManager()
{
}

void CL_VirtualFileManager::Reset()
{
	m_vecSource.clear();
}

bool CL_VirtualFileManager::MountDirectoryPath(const string &path)
{
	CL_VirtualFileSource s;

	s.m_strPath = path;

	//check to see if path is valid?

	m_vecSource.push_back(s);
	return true;
}

CL_InputSource * CL_VirtualFileManager::GetFile(const string &fname)
{
	CL_VirtualFileSource *pSource = NULL;

	CL_InputSource *pInputSource = NULL;

	filesource_vector::reverse_iterator itor;

	for (itor = m_vecSource.rbegin(); itor != m_vecSource.rend(); itor++)
	{

		pSource = &(*itor);

		switch(pSource->m_type)
		{
		case CL_VirtualFileSource::eFilePath:
		
			//does the file exist here?
			//let's try to open it
			if (pInputSource = pSource->GetFile(fname))
			{
				//success
				return pInputSource;
			}

			//failed, keep searching
			
			break;

		default:
			throw CL_Error("Unsupported virtual file format");
		}
	}

//if we got here, we couldn't find it

	return NULL;

}

CL_OutputSource * CL_VirtualFileManager::PutFile(const string &fname)
{
	CL_VirtualFileSource *pSource = NULL;
	CL_OutputSource *pOutputSource = NULL;

	filesource_vector::reverse_iterator itor;

	for (itor = m_vecSource.rbegin(); itor != m_vecSource.rend(); itor++)
	{
		pSource = &(*itor);

		switch(pSource->m_type)
		{
		case CL_VirtualFileSource::eFilePath:

			//does the file exist here?
			//let's try to open it
			if (pOutputSource = pSource->PutFile(fname))
			{
				//success
				return pOutputSource;
			}

			//failed, keep searching

			break;

		default:
			throw CL_Error("Unsupported virtual file format");
		}
	}

	//if we got here, we couldn't find it

	return NULL;
}

bool CL_VirtualFileManager::CreateDir(const string &fname)
{
	CL_VirtualFileSource *pSource = NULL;

	filesource_vector::reverse_iterator itor;

	for (itor = m_vecSource.rbegin(); itor != m_vecSource.rend(); itor++)
	{
		pSource = &(*itor);

		switch(pSource->m_type)
		{
		case CL_VirtualFileSource::eFilePath:

			return pSource->CreateDir(fname);
			break;

		default:
			throw CL_Error("Unsupported virtual file format");
		}	
	}

	return false; //couldn't find it
}


bool CL_VirtualFileManager::RemoveFile(const string &fname)
{
	CL_VirtualFileSource *pSource = NULL;

	filesource_vector::reverse_iterator itor;

	for (itor = m_vecSource.rbegin(); itor != m_vecSource.rend(); itor++)
	{
		pSource = &(*itor);

		switch(pSource->m_type)
		{
		case CL_VirtualFileSource::eFilePath:

			return pSource->RemoveFile(fname);
			break;

		default:
			throw CL_Error("Unsupported virtual file format");
		}	
	}

	return false; //couldn't find it
}

void CL_VirtualFileManager::GetMountedDirectories(vector<string> *pPathsOut)
{
	CL_VirtualFileSource *pSource = NULL;

	filesource_vector::iterator itor;

	for (itor = m_vecSource.begin(); itor != m_vecSource.end(); itor++)
	{
		pSource = &(*itor);

		switch(pSource->m_type)
		{
		case CL_VirtualFileSource::eFilePath:
			pPathsOut->push_back(pSource->m_strPath);
		break;

		default: ;
		}	
	}

return;
}

string CL_VirtualFileManager::GetLastMountedDirectory()
{
	return m_vecSource.back().m_strPath;
}

bool CL_VirtualFileManager::LocateFile(string &fnameOut)
{
	CL_VirtualFileSource *pSource = NULL;

	filesource_vector::reverse_iterator itor;

	for (itor = m_vecSource.rbegin(); itor != m_vecSource.rend(); itor++)
	{
		pSource = &(*itor);

		switch(pSource->m_type)
		{
		case CL_VirtualFileSource::eFilePath:

			if (FileExists( (pSource->m_strPath+"/"+fnameOut)))
			{
				fnameOut = pSource->m_strPath+"/"+fnameOut;
				return true;
			}
			break;

		default:
			throw CL_Error("Unsupported virtual file format");
		}	
	}

	return false; //couldn't find it
}

CL_InputSource * CL_VirtualFileManager::GetFileRaw( const string &fname )
{
	//try to open the file they want
	CL_InputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_InputSource_File(fname);

	} catch(CL_Error error)
	{
		//file doesn't exist
		delete pFile;
		return NULL;
	}
	return pFile;
}

CL_OutputSource * CL_VirtualFileManager::PutFileRaw( const string &fname )
{
	//try to open the file they want for writing
	CL_OutputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_OutputSource_File(fname);

	} catch(CL_Error error)
	{
		delete pFile;
		return NULL;
	}
	return pFile;
}
