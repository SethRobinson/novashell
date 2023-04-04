#include "AppPrecomp.h"
#include "DataManager.h"
#include "GameLogic.h"

string DataObject::Get()
{
	if (m_dataType == DataManager::E_STRING) return m_value;

	//must be a number
	assert(m_dataType == DataManager::E_NUM);
	return CL_String::from_float(m_num);
}

void DataObject::Set(const string &keyName, const string &value)
{
	m_key = keyName;
	m_value = value;
	m_dataType = DataManager::E_STRING;
}

void DataObject::SetNum(const string &keyName, float value)
{
	m_key = keyName;
	m_num = value;
	m_value.clear();
	m_dataType = DataManager::E_NUM;
}

float DataObject::GetNum()
{
	if (m_dataType == DataManager::E_NUM) return m_num;

	return CL_String::to_float(m_value);
}

void DataObject::Serialize(CL_FileHelper &helper, int version)
{
	//loads or saves to disk

	helper.process(m_dataType);
	helper.process(m_key);
	helper.process(m_value);

 //automatically convert old data types
	if (version >= 20000000 || helper.IsWriting())
 {
	 helper.process(m_num);
 }

}

DataManager::DataManager()
{
}

DataManager::~DataManager()
{
}

void DataManager::Clear()
{
	m_data.clear();
}

//modify the number by a num, creates the key if it doesn't exist
float DataManager::ModNum(const string &keyName, float value)
{
	
	DataObject *pData = FindDataByKey(keyName);

	if (!pData)
	{
		//we need to create it too it looks like
		DataObject d;
		d.SetNum(keyName, value);
		m_data[keyName] = d;
		return value;
	}

	
	if (pData->m_dataType == DataManager::E_STRING)
	{
		//we'll, we need to convert this first..
		pData->SetNum(keyName, pData->GetNum());
	}

	pData->m_num += value;
  return pData->m_num;
}

DataObject * DataManager::FindDataByKey(const string &keyName)
{
	dataList::iterator itor = m_data.find(keyName);

	if (itor != m_data.end())
		{
			//bingo!
			return &(itor->second);
		}
	return NULL; //doesn't exist
}

string DataManager::Get(const string &keyName)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData) return "";
	return pData->Get();
}

string DataManager::GetWithDefault(const string &keyName, const string &value)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData) 
	{
		Set(keyName, value);
		return value;
	}
	
	return pData->Get();
}

float DataManager::GetNum(const string &keyName)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData) return 0;
	
	return pData->GetNum();
}

float DataManager::GetNumWithDefault(const string &keyName, float value)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData) 
	{
		SetNum(keyName, value);
		return value;
	}

	return pData->GetNum();
}


//returns true if we actually set the value, which we only do if it didn't exist
string DataManager::SetIfNull(const string &keyName, const string &value)
{
	DataObject *pData = FindDataByKey(keyName);

	if (pData) return pData->Get(); //didn't need to set it

	DataObject d;
	d.Set(keyName, value);
	m_data[keyName]=d;
	return value; //true that we created a key
}

//returns true if we actually set the value, which we only do if it didn't exist
float DataManager::SetNumIfNull(const string &keyName, float value)
{
	DataObject *pData = FindDataByKey(keyName);

	if (pData) return pData->GetNum(); //didn't need to set it

	DataObject d;
	d.SetNum(keyName, value);
	m_data[keyName]=d;
	return value;
}

bool DataManager::Exists(const string &keyName)
{
	return FindDataByKey(keyName) != 0;
}

void DataManager::Delete(const string &keyName)
{
	m_data.erase(keyName);
}


bool DataManager::Set(const string &keyName, const string &value)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData)
	{
		//we need to create it too it looks like
		DataObject d;
		d.Set(keyName, value);
		m_data[keyName] = d;
		return true; //true that we created a key
	}

	pData->m_value = value;
	pData->m_dataType = E_STRING; //in case it was a num before
	
	return false; //false indicating that we didn't need to create a new key
}


bool DataManager::SetNum(const string &keyName, float num)
{
	DataObject *pData = FindDataByKey(keyName);

	if (!pData)
	{
		//we need to create it too it looks like
		DataObject d;
		d.SetNum(keyName, num);
		m_data[keyName] = d;
		return true; //true that we created a key
	}

	
	pData->m_num = num;
	pData->m_dataType = E_NUM; 
	return false; //false indicating that we didn't need to create a new key
}


void DataManager::Serialize(CL_FileHelper &helper)
{
	//load/save needed data
	int size = m_data.size();

	int version = DATAMANAGER_VERSION;
	
	if (helper.IsWriting())
	{
		helper.process(version);
	}

	helper.process(size); //load or write it, depends
	//these hacks were for backward compatibility after I changed the format..
	if (!helper.IsWriting())
	{
		if (size >= 20000000)
		{
			//this is a new style format
			version = size;
			helper.process(size); //load or write it, depends
		} else
		{
			version = 0;
		}
	}

	if (helper.IsWriting())
	{
		dataList::iterator itor = m_data.begin();
		while (itor != m_data.end())
		{
			itor->second.Serialize(helper, version);
			itor++;
		}
	} else
	{
		//reading
		for (int i=0; i < size; i++)
		{
			DataObject o;
			o.Serialize(helper, version);
			m_data[o.m_key] = o;
		}
	}
}

void DataManager::Save( string filename )
{
	//save out our globals
	CL_OutputSource *pSource = g_VFManager.PutFile(filename);

	if (pSource)
	{
		CL_FileHelper helper(pSource);
		Serialize(helper);
		SAFE_DELETE(pSource);
	}

}

void DataManager::Load( string filename )
{
	CL_InputSource *pFile = g_VFManager.GetFile(filename);

	if (!pFile)
	{
		return; //nothing to load
	}
	try
	{
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving
	Serialize(helper);
	} catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		ShowMessage(error.message.c_str(), "Error loading database.  Corrupted?");
		SAFE_DELETE(pFile);
		return;
	}
	SAFE_DELETE(pFile);

}


/*
Object: DataManager
An object designed to flexibly store and retrieve numbers and strings very quickly use key/value pairs.

About:

In addition to any <Entity> having its own unique <DataManager> via <Entity::Data> a global one is always available through <GameLogic::Data> as well.

Data is persistant automatically, ie, saved between sessions.  There are no size limits and lookups are extremely fast. (Internally, a binary search tree is used)

Access from the editor:

Using the Entity Properties Editor, you can also add/remove/modify data from within editor, in addition to normal script access.

Group: Storing Data

func: Set
(code)
boolean Set(string keyName, string value)
(end)
Store a string as a named piece of data to be retrieved later.
Replaces data by this key name if it already exists.

keyName - Any name you wish.
value - A string of any length with the text/data that you wish to store.

Returns:

True if a new piece of data was created, false if existing data was replaced.

func: SetNum
(code)
boolean SetNum(string keyName, number num)
(end)
Similar to <Set> but saves the data as a number which saves space and is faster to access internally.

When <Get>is used with a number (instead of <GetNum>) it is automatically converted into a string.

Decimal points are ok to use.  The accuracy maintained is that of a C "float".

keyName - Any name you wish.
num - The number you wish to store.

Returns:

True if a new piece of data was created, false if existing data was replaced.

func: SetIfNull
(code)
string SetIfNull(string keyName, string value)
(end)
Similar to <Set> but stores the data only if the key didn't already exist.

keyName - Any name you wish.
value - The string data you wish to store.

Returns:

String that was set, or existed before.

func: SetNumIfNull
(code)
number SetNumIfNull(string keyName, number value)
(end)
Similar to <Set> but stores the data only if the key didn't already exist.

Like <Data:SetIfNull> but hints that we want it stored as a number, not a string.

keyName - Any name you wish.
value - The number you wish to store.

Returns:

Returns the number that was set or existed before.

Group: Retrieving Data

func: Get
(code)
string Get(string keyName)
(end)
Retrieve a previously stored value by its key name.

keyName - The key-name used when it was stored.

Returns:

The data in string form or a blank string if the key wasn't found.  Use <Exists> to verify if a key exists or not.

func: GetNum
(code)
number Get(string keyName)
(end)
Like <Get> but returns the data as a number.  If the data was stored as a number, this is the fastest way to access it.

If the data was stored as a string, an attempt to convert it to a number is made.

keyName - The key-name used when it was stored.

Returns:

The number that was stored.

func: GetNumWithDefault
(code)
number GetNumWithDefault(string keyName, number defaultValue)
(end)
Like <Get> but allows you to also set the default data if the key doesn't exist yet.

keyName - The key-name used when it was stored.
defaultValue - If the key doesn't exist, it will be created with this value.  (this value will be returned as well)

Returns:

The value that was stored or created.

func: GetWithDefault
(code)
number GetWithDefault(string keyName, string defaultValue)
(end)
Like <Get> but allows you to also set the default data if the key doesn't exist yet.

keyName - The key-name used when it was stored.
defaultValue - If the key doesn't exist, it will be created with this value.  (this value will be returned as well)

Returns:

The value that was stored or created.

Group: Miscellaneous

func: Exists
(code)
boolean Exists(string keyName)
(end)

keyName - The key name used when it was stored.

Returns:

True if data with this key name exists.

func: ModNum
(code)
number Exists(string keyName, number modAmount)
(end)
Modifies an existing key by a number.  Creates the key if it didn't exist.

Usage:
(code)

//set hitpoints to 100
this:Data():SetAsNum("life", 100);

//remove 5
local curHitpoints = this:Data():ModNum("life", -5);

LogMsg("I only have " .. curHitpoints .. " hitpoints!");
(end)

keyName - The key name used when it was stored.
modAmount - How much the number should be changed by.

Returns:

The new number that was stored.

func: Delete
(code)
nil Delete(string keyName)
(end)
Completely removes a key/value pair from the database.

keyName - The key name used when it was stored.

func: Clear
(code)
nil Clear()
(end)
Completely removes all stored data from this database.
*/


