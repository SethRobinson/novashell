#include "AppPrecomp.h"
#include "main.h"
#include "EntChooseScreenMode.h"
#include "GameLogic.h"
#include "AppUtils.h"
#include "ScriptManager.h"
#include "VisualProfileManager.h"
#include "Console.h"
#include "AI/WatchManager.h"
#include "EntEditMode.h"
#include "EntEditor.h"
 
#define C_PREFS_DAT_FILENAME "prefs.dat"

#define C_DELTA_HISTORY_BUFFER_SIZE 1

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

const unsigned int C_PREF_DAT_VERSION = 0;

ISoundManager *g_pSoundManager;

#ifdef _WIN32
	 //#include "misc/FMSoundManager.h" //FMOD disabled for now
#endif

#include <API/vorbis.h>
#include <API/sound.h>
#include "misc/CL_SoundManager.h"

App MyApp; //declare the main app global
App * GetApp(){return &MyApp;}
GameLogic * GetGameLogic() {return MyApp.GetMyGameLogic();}

void LogMsg(const char *lpFormat, ...)
{
	va_list Marker;
    char szBuf[C_LOGGING_BUFFER_SIZE];
    va_start(Marker, lpFormat);
    vsprintf(szBuf, lpFormat, Marker);
    va_end(Marker);
    char stTemp[C_LOGGING_BUFFER_SIZE+256];

	g_Console.Add(szBuf);

	sprintf(stTemp, "%s\r\n", szBuf);

#ifdef _DEBUG
	#ifdef WIN32
		OutputDebugString(stTemp);
	#endif
#endif   

	std::cout << stTemp;
	//std::cout.flush();
}

void LogError(const char *lpFormat, ...)
{
    va_list Marker;
    char szBuf[C_LOGGING_BUFFER_SIZE];
    va_start(Marker, lpFormat);
	vsprintf(szBuf, lpFormat, Marker);
	va_end(Marker);
    char stTemp[C_LOGGING_BUFFER_SIZE+256];
	g_Console.AddError(szBuf);

	sprintf(stTemp, "Error: %s\r\n", szBuf);

#ifdef _DEBUG
	#ifdef WIN32
		OutputDebugString(stTemp);
	#endif
#endif    

	std::cout << stTemp;
	//std::cout.flush();
}

App::App()
{
    m_bJustRenderedFrame = false;
	m_uniqueNum = 0;
	m_pFrameRate = NULL;
	m_bRequestToggleFullscreen = false;   
	m_pResourceManager = NULL;
	m_pCamera = NULL;
    m_pGUIResourceManager = NULL;
	m_pScriptManager = NULL;
    m_pWindow = NULL;
    m_pBackground = NULL;
    m_bWindowResizeRequest = false;
    m_bQuit = false;
    m_HaveFocus = true;
    m_bClipCursorWhenFullscreen = true;
    m_pStyle = NULL;
    m_pGui = NULL;
	m_pHashedResourceManager = NULL;
	m_gameTick = 0;
	m_bRequestVideoInit = false;
	m_baseGameSpeed = 10;
	m_baseLogicMhz = 1000.0f / 75.0f;
	m_simulationSpeedMod = 1.0f; //2.0 would double the game speed
	m_engineVersion = 0.84f;
	m_engineVersionString = "0.84";
	m_notifyOfScreenChange = false;


	m_pSetup_sound = NULL;
	m_pSetup_vorbis = NULL;
	m_pSound_output = NULL;
	srand(CL_System::get_time());

	ClearTimingAfterLongPause();
	ComputeSpeed();
	m_thinkTicksToUse = 0;
    for (int i=0; i < C_FONT_COUNT; i++)
    {
       m_pFonts[i] = 0;
    }
	m_pConsoleFont = NULL;

#ifdef WIN32
	m_Hwnd = false;
#endif
}

App::~App()
{
}

int App::GetPlatform()
{
#ifdef __APPLE__
	return C_PLATFORM_OSX;
#elif defined (WIN32)
	return C_PLATFORM_WINDOWS;
#else
	return C_PLATFORM_LINUX;
#endif
}

void App::SetWindowTitle(const string &title)
{
	CL_Display::get_current_window()->set_title(title);
}

unsigned int App::GetUniqueNumber()
{
	if (m_uniqueNum == UINT_MAX)
	{
		assert(!"It's about to wrap, handle this?");
	}
	return ++m_uniqueNum;
}


CL_Vector2 GetOriginalScreenSizeOfOS()
{
	CL_Vector2 vScreenSize(0,0);

#ifdef _WIN32

	RECT desktop;  
	// Get a handle to the desktop window  
	const HWND hDesktop = GetDesktopWindow();  
	// Get the size of screen to the variable desktop  
	GetWindowRect(hDesktop, &desktop);  
	vScreenSize.x = desktop.right;  
	vScreenSize.y = desktop.bottom;  
#endif

#ifdef __APPLE__

	int w, h;
	// screen width & height
	w = CGDisplayPixelsWide(kCGDirectMainDisplay );
	h = CGDisplayPixelsHigh(kCGDirectMainDisplay );

	return CL_Vector2(w,h);

#endif

		return vScreenSize;
}

void App::OneTimeDeinit()
{
	SavePrefs();

	g_EntEditModeCopyBuffer.ClearSelection();

	for (int i=0; i < C_FONT_COUNT; i++)
	{
		SAFE_DELETE(m_pFonts[i]);
	}

	SAFE_DELETE(m_pGameLogic);
	SAFE_DELETE(m_pScriptManager);

	SAFE_DELETE(m_pCamera);
	SAFE_DELETE(g_pSoundManager);

	SAFE_DELETE(m_pBackgroundCanvas);
    SAFE_DELETE(m_pBackground);

	SAFE_DELETE(m_pVisualProfileManager);
	SAFE_DELETE(m_pHashedResourceManager);

	g_Console.KillGUI();

	SAFE_DELETE(m_pConsoleFont);
	SAFE_DELETE(m_pGui);
    SAFE_DELETE(m_pStyle);

	SAFE_DELETE(m_pGUIResourceManager);
	SAFE_DELETE(m_pResourceManager);
  
    SAFE_DELETE(m_pWindow);
	SAFE_DELETE(m_pFrameRate);

	SAFE_DELETE(m_pSound_output);
	SAFE_DELETE(m_pSetup_vorbis);
	SAFE_DELETE(m_pSetup_sound);
}

string App::GetDefaultTitle()
{
	return "Novashell Game Creation System " + App::GetEngineVersionAsString();
}

bool App::VidModeIsSupported(CL_Size vidMode, int bit)
{
	std::vector<CL_DisplayMode> modes = CL_DisplayMode::get_display_modes();

	for(unsigned int i=0; i < modes.size(); ++i)
	{
		if (modes[i].get_bpp() == bit)
		{
			if (modes[i].get_resolution() == vidMode)
			{
				//video mode supported
				return true;
			}
		}
	}

	return false;
}

void App::OneTimeInit()
{
    //initialize our main window
	m_originalScreensize = GetOriginalScreenSizeOfOS();
	LogMsg("Original screensize detected as %.2f X %.2f", m_originalScreensize.x, m_originalScreensize.y);
	bool bFullscreen = !CL_String::to_bool(m_prefs.Get("start_in_windowed_mode"));
	
	CL_Size vidMode = CL_Size(1024, 768);
	
	string res = m_prefs.Get("default_screen_resolution");
	vector<string> v = CL_String::tokenize(res, " ", true);
	if (v.size() == 2)
	{
		vidMode.width = CL_String::to_int(v[0]);
		vidMode.height = CL_String::to_int(v[1]);
	} else if (v.size() == 1)
	{

		if (v[0] == "desktop")
		{
			if (m_originalScreensize != CL_Vector2(0,0))
			{
				vidMode = CL_Size(m_originalScreensize.x, m_originalScreensize.y);
			} else
			{
					LogMsg("Ignoring desktop parm, we don't support it on this OS yet");
			}
		}
	} else
	{
		LogMsg("Don't understand a default_screen_resolution of \"%s\", ignoring.", res.c_str());
	}

	int bits = 32;
   
	for (unsigned int i=0; i < GetApp()->GetStartupParms().size(); i++)
	{
		string p1 = GetApp()->GetStartupParms().at(i);
		string p2;
		string p3;

		if (i+1 < GetApp()->GetStartupParms().size())
		{
			//this second part might be needed		
			p2 = GetApp()->GetStartupParms().at(i+1);
		}

		if (i+2 < GetApp()->GetStartupParms().size())
		{
			//this second part might be needed		
			p3 = GetApp()->GetStartupParms().at(i+2);
		}

		if (CL_String::compare_nocase(p1,"-window") || CL_String::compare_nocase(p1,"-windowed") )
		{
			bFullscreen = false;
		}
		if (CL_String::compare_nocase(p1,"-full") || CL_String::compare_nocase(p1,"-fullscreen") )
		{
			bFullscreen = true;
		}

		if (CL_String::compare_nocase(p1, "-resolution") || CL_String::compare_nocase(p1, "-res") )
		{
			if (p2.empty() && p3.empty())
			{
				LogError("Ignored -resolution parm, format incorrect (should be -resolution 640 480)");
			} else
			{
				if (p2 == "desktop")
				{
					if (m_originalScreensize != CL_Vector2(0,0))
					{
						vidMode = CL_Size(m_originalScreensize.x, m_originalScreensize.y);
					} else
					{
						LogMsg("Ignoring desktop parm, we don't support it on this OS yet");
					}
					
				} else
				{
					vidMode = CL_Size(CL_String::to_int(p2), CL_String::to_int(p3));
				}
			}
		}
	}
	
	if (bFullscreen && !VidModeIsSupported(vidMode, bits))
	{
#ifdef WIN32
		char stTemp[512];
		sprintf(stTemp, "%dX%dX%d isn't supported by your video card.\r\nWe'll try 800X600 at 32 bits I guess.", vidMode.width, vidMode.height, bits);

		MessageBox(NULL, stTemp, "Video Resolution Unsupported", MB_ICONWARNING);
		vidMode = CL_Size(800, 600);
#endif

		//vidmodesupport isn't reliable for OSX/Linux, so we don't make this error out..
	} 

	m_WindowDescription.set_size(vidMode);

	m_WindowDescription.set_fullscreen(bFullscreen);
    m_WindowDescription.set_bpp(bits);
    m_WindowDescription.set_title(GetDefaultTitle());
    m_WindowDescription.set_allow_resize(true);

	m_bCursorVisible = true;
    m_WindowDescription.set_flipping_buffers(2);
    m_WindowDescription.set_refresh_rate(75);
	SetRefreshType(FPS_AT_REFRESH);

	try
	{
		m_pWindow = new CL_DisplayWindow(m_WindowDescription);
	} catch(CL_Error error)
	{
		string fullscreen = "fullscreen";
		if (!bFullscreen) fullscreen = "windowed";
		LogError("Unable to create %dX%dX%d (%s) window. (%s)  Changing to windowed mode in prefs.dat.  Try running again.",
			vidMode.width, vidMode.height, bits, fullscreen.c_str(), error.message.c_str());
		
		//set some parms for compability:
		m_prefs.Set("start_in_windowed_mode", "yes");
		m_prefs.Set("default_screen_resolution", "1024 768");
		
		return;
	}
  
	g_Console.Init();

    m_pResourceManager = new CL_ResourceManager("base/resources.xml", false);
    
	CL_ResourceManager temp("base/editor/editor_resources.xml", false);
    m_pResourceManager->add_resources(temp);

    m_pGUIResourceManager = new CL_ResourceManager("base/gui/gui.xml", false);
    
    m_pStyle = new CL_StyleManager_Silver(GetApp()->GetGUIResourceManager());
    m_pGui = new CL_GUIManager(m_pStyle);

	m_pConsoleFont = new CL_Font("Window/font_disabled", m_pGUIResourceManager);

	m_pCamera = new Camera; 
#ifdef WIN32
	m_Hwnd = GetActiveWindow();
	if (m_Hwnd == NULL) throw CL_Error("Error getting a valid HWND."); //can that ever happen?  Doubt it. More likely to just be wrong, rather than NULL.
#endif

    SetupBackground(GetApp()->GetMainWindow()->get_width(), GetApp()->GetMainWindow()->get_height());
	m_pHashedResourceManager = new HashedResourceManager;
	m_pVisualProfileManager = new VisualProfileManager;

	switch (m_soundSystem)
	{
	case C_SOUNDSYSTEM_CLANLIB:
		g_pSoundManager = new CL_SoundManager;
		break;

	case C_SOUNDSYSTEM_FMOD:

	#ifdef _WIN32
		LogError("FMOD sound system not supported on this platform yet.");

		//g_pSoundManager = new CFMSoundManager;
	#else
		LogError("FMOD sound system not supported on this platform yet.");
	#endif
		break;
	}

	m_pScriptManager = new ScriptManager();
	m_pGameLogic = new GameLogic();

#ifdef _WIN32
	//hack for multi monitor problem and the game starting and then alt-tabbing back for some reason
	SetActiveWindow(m_Hwnd);
#endif
	g_inputManager.OneTimeInit();

}
       
bool App::SetScreenSize(int x, int y)
{
	if (m_WindowDescription.get_size() != CL_Size(x, y))
	{
		m_WindowDescription.set_position(CL_Rect(0, 0, x, y));
		//m_WindowDescription.set_size(CL_Size(x,y));
		m_bRequestVideoInit = true;
	}

	return true;
}

bool App::ActivateVideoRefresh(bool bFullscreen)
{
	CL_Display::flip(0);
	//CL_System::keep_alive(); 

	m_bRequestVideoInit = false;
	
	CL_Size oldSize(GetApp()->GetMainWindow()->get_width(), GetApp()->GetMainWindow()->get_height());

	if (!bFullscreen)
	{
		//might as well center it too?
		m_pWindow->set_windowed();


		m_pWindow->set_size(m_WindowDescription.get_size().width,m_WindowDescription.get_size().height );
		
		


#ifdef WIN32
		m_pWindow->set_position( (GetSystemMetrics(SM_CXSCREEN)-GetScreenX)/2, (GetSystemMetrics(SM_CYSCREEN)-GetScreenY)/2);
#endif


	} else
	{
		try
		{
			m_pWindow->set_fullscreen(m_WindowDescription.get_size().width, m_WindowDescription.get_size().height, m_WindowDescription.get_bpp(), m_WindowDescription.get_refresh_rate());
		} catch (CL_Error error)
		{
			string fullscreen = "fullscreen";
			if (!bFullscreen) fullscreen = "windowed";
			LogError("Unable to create %dX%d (%s) window. (%s) trying to recover",
				m_WindowDescription.get_size().width, m_WindowDescription.get_size().height, fullscreen.c_str(), error.message.c_str());

			//set some parms for compability:
			
			SetScreenSize(oldSize.width,oldSize.height);
			return false;
		}
			
		//surfaces are now invalid.  Rebuild them ?
		SetupBackground(GetApp()->GetMainWindow()->get_width(), GetApp()->GetMainWindow()->get_height());
	}

	m_notifyOfScreenChange = true;

	EntEditor * pEnt = (EntEditor *)EntityMgr->GetEntityByName("editor");

	if (pEnt)
	{
		//the editor is open, lets reset it..
		pEnt->Kill();
	}

#ifdef _WIN32
	//hack for multi monitor problem and the game starting and then alt-tabbing back for some reason
	SetActiveWindow(m_Hwnd);
#endif

	return true;
}

//load a pic and tile it to our background surface
void App::SetupBackground(int x, int y)
{
    SAFE_DELETE(m_pBackgroundCanvas);
    SAFE_DELETE(m_pBackground);
	m_pBackground = new CL_Surface(CL_PixelBuffer(x, y,x*4,  CL_PixelFormat::abgr8888));
    m_pBackgroundCanvas = new CL_Canvas(*m_pBackground);
    SetupMouseClipping();
    //LogMsg("Background rebuilt");
    m_pGameLogic->RebuildBuffers();

    ClearTimingAfterLongPause();
}

void App::OnWindowResize(int x, int y)
{
    m_bWindowResizeRequest = true;
}

void App::OnWindowClose()
{
   m_bQuit = true; //quit the app
}

void App::SetCursorVisible(bool bNew)
{
	if (m_bCursorVisible == bNew) return;

	if (bNew)
	{
		m_pWindow->show_cursor();
	} else
	{
		m_pWindow->hide_cursor();
	}
	m_bCursorVisible = bNew;
}

bool App::GetCursorVisible()
{
	return m_bCursorVisible;
}

void App::SetupMouseClipping()
{
#ifdef WIN32
	// Confine cursor to fullscreen window
    if( m_bClipCursorWhenFullscreen )
    {
        if (m_pWindow->is_fullscreen() && m_HaveFocus)
        {
            //std::cout << "Clipped cursor.";
            RECT rcWindow;
            GetWindowRect(GetHWND(), &rcWindow );
            ClipCursor( &rcWindow );
        }
        else
        {
            //std::cout << "Freed cursor.";
           ClipCursor( NULL );
        }
    }
#endif
}

void App::OnLoseFocus()
{
	//LogMsg("Lost focus..");
	m_HaveFocus = false;
    SetupMouseClipping();
    if (m_pWindow->is_fullscreen())
    {
        //m_pWindow->
#ifdef WIN32
		SendMessage(m_Hwnd, WM_SYSCOMMAND, SC_MINIMIZE,0);
        
		//ChangeDisplaySettings(NULL, 0);
#endif
    }
}

void App::OnGotFocus()
{
	//LogMsg("Got focus...");
    m_HaveFocus = true;
  // m_bWindowResizeRequest = true; //draw background again
   SetupMouseClipping();

   if (m_pWindow->is_fullscreen())
   {
        m_pWindow->set_fullscreen(m_WindowDescription.get_size().width, m_WindowDescription.get_size().height, m_WindowDescription.get_bpp(), m_WindowDescription.get_refresh_rate());
       LogMsg("Window description says X:%d, Y:%d", m_WindowDescription.get_size().width, m_WindowDescription.get_size().height);
   }
}

void App::RequestToggleFullscreen()
{
	//it's dangerous to do it whenever so we'll schedule this for after the main game loop is
	//done running
	m_bRequestToggleFullscreen = true;
}

void App::ToggleWindowedMode()
{
   m_bRequestToggleFullscreen = false; 
    
   LogMsg("Toggling windowed mode");
   ActivateVideoRefresh(!m_pWindow->is_fullscreen());
   
//   LogMsg("Screen changed to X:%d, Y:%d", GetApp()->GetMainWindow()->get_width(), GetApp()->GetMainWindow()->get_height());
	ClearTimingAfterLongPause();  
}

void App::OnKeyUp(const CL_InputEvent &key)
{
	if(CL_Keyboard::get_keycode(CL_KEY_MENU))
    {
        //system message?  We should really process these somewhere else
        switch (key.id)
        {
        case CL_KEY_ENTER:
            ToggleWindowedMode();
            break;
            
        case CL_KEY_F4:
            OnWindowClose();
            break;
        }

		return;
    }

#ifdef __APPLE__
	if(CL_Keyboard::get_keycode(CL_KEY_COMMAND))
	{
		//system message?  We should really process these somewhere else
		switch (key.id)
		{
		case CL_KEY_F:
			ToggleWindowedMode();
			break;

		case CL_KEY_Q:
			OnWindowClose();
			break;
		}

		return;
	}
#endif
}

void App::ClearTimingAfterLongPause()
{
    m_lastFrameTime = CL_System::get_time();
 	m_thinkTicksToUse = 0;
	m_deltaHistory.clear();
}

void App::RequestAppExit()
{
	m_bQuit = true;
}

void App::OnRender()
{
	m_pGameLogic->Render();
}

void App::SetGameLogicSpeed(float fNew)
{
	m_baseLogicMhz = fNew;
	ComputeSpeed();
}

void App::ComputeSpeed()
{
	m_delta = GetGameLogicSpeed()/(1000/60);
}

void App::SetGameSpeed(float fNew)
{
	m_baseGameSpeed = fNew;
	ComputeSpeed();
}

void App::SetGameTick(unsigned int num)
{
	 m_gameTick = num;
}

int App::GetAverageDelta()
{
	unsigned int delta = 0;
	for (unsigned int i=0; i < m_deltaHistory.size(); i++)
	{
		delta += m_deltaHistory[i];
	}
	return delta/m_deltaHistory.size();
}

void App::Update()
{
	//figure out our average delta frame

	static unsigned int frameTime;
	frameTime = CL_System::get_time();
	unsigned int deltaTick = frameTime -  m_lastFrameTime;

	//if (deltaTick < 1000/cl_max(1, GetFPS()+3))
	{
	//	return;
	}

	m_lastFrameTime = frameTime;
	
	m_thinkTicksToUse += (float(deltaTick) * m_simulationSpeedMod);
	
	if (deltaTick != 0)
	{
		//very important, here we set the min and max range of allowed logic updates.  Too small, and physics might break, too large and.. uh..
		//well nothing bad should happen but still, 120 is probably enough for any monitor to "look smooth".
		
		int logicMhz = (1000/deltaTick);
		
		logicMhz = cl_max(60, logicMhz);
		logicMhz = cl_min(120, logicMhz);
		
		m_deltaHistory.push_back(logicMhz);
		if (m_deltaHistory.size() > C_DELTA_HISTORY_BUFFER_SIZE)
		m_deltaHistory.pop_front(); //cycle the values

		//get average delta
		logicMhz = GetAverageDelta();
		//LogMsg("Setting logic speed to %d", 1000/logicMhz);
		SetGameLogicSpeed(1000 / logicMhz );
	}

	//OutputDebugString(CL_String::from_int(CL_System::get_time()).c_str());
	//OutputDebugString("\n");
	
	bool bDidUpdate = false;

	#define C_MAX_TIME_PER_LOGIC_UPDATE_MS 200 //avoid mega slow down

	while (m_thinkTicksToUse > GetGameLogicSpeed())
	{
		if (!m_pGameLogic->GetGamePaused())
		{
			m_gameTick += GetGameLogicSpeed();
		}

		m_pGameLogic->Update(m_delta);
		m_bJustRenderedFrame = false;
		m_thinkTicksToUse -= GetGameLogicSpeed();
		
		if (m_thinkTicksToUse > 0 && m_thinkTicksToUse < 3)
		{
			//LogMsg("Skip %.2f", m_thinkTicksToUse);
			//don't allow slow accumulation...
			m_thinkTicksToUse = 0;
		}
		
		/*
		if (m_thinkTicksToUse < GetGameLogicSpeed()*1.5)
		{
			//this stops jerkiness at 60 hz when it 'barely' needs to do a second
			//update cycle...
			break;
		}
		*/
		if ( frameTime + C_MAX_TIME_PER_LOGIC_UPDATE_MS < CL_System::get_time())
		{
			//our CPU can't keep up, skip the rest for now so we can get a visual update
			m_thinkTicksToUse = 0;
		//	 LogMsg("Slowdown..");
			break;
		}

		bDidUpdate = true;
	}
}

void log_to_cout(const std::string &channel, int level, const std::string &message)
{
	LogMsg(string("[" + channel + "] " + message).c_str());
}

bool App::ParmExists(const string &parm)
{
	vector<string>::iterator itor = m_startupParms.begin();

	while (itor != m_startupParms.end())
	{
		if (*itor == parm) return true;
		itor++;
	}

	return false;
}

void App::LoadPrefs()
{
	CL_InputSource *pSource = g_VFManager.GetFileRaw(GetBaseDirectory() + C_PREFS_DAT_FILENAME);
	if (pSource)
	{
		CL_FileHelper helper(pSource);

		unsigned int version;
		helper.process(version);
		m_prefs.Serialize(helper);
		SAFE_DELETE(pSource);
	}
	
	//set defaults, no prefs file exists
	m_prefs.SetIfNull("start_in_windowed_mode", "yes");
	m_prefs.SetIfNull("default_screen_resolution", "1024 768");
	m_prefs.SetIfNull("linux_editor", "gedit");
	m_prefs.SetIfNull("command_line_parms", "");
}

void App::SavePrefs()
{
	//save out our globals
	CL_OutputSource *pSource = g_VFManager.PutFileRaw(GetBaseDirectory() + C_PREFS_DAT_FILENAME);
	
	if (pSource)
	{
		CL_FileHelper helper(pSource);
		helper.process_const(C_PREF_DAT_VERSION);
		m_prefs.Serialize(helper);
		SAFE_DELETE(pSource);
	} else
	{
		LogError("Unable to write %s%s.", GetBaseDirectory().c_str(), C_PREFS_DAT_FILENAME);
	}
}

void App::BuildCommandLineParms()
{
	m_startupParms.clear();
	
	
	string fName = "command_line.txt";

	if (FileExists(fName))
	{
		CL_InputSource_File input(fName);
		vector<string> v =CL_String::tokenize( GetNextLineFromFile(&input), " ", true);

		for (unsigned int i=0; i < v.size(); i++)
		{
			m_startupParms.push_back(v[i]);
		}
	}

	vector<string> v =CL_String::tokenize(m_prefs.Get("command_line_parms"), " ", true);
	for (unsigned int i=0; i < v.size(); i++)
	{
		m_startupParms.push_back(v[i]);
	}

	v = CL_String::tokenize(m_originalParms, " ", true);
	for (unsigned int i=0; i < v.size(); i++)
	{
		m_startupParms.push_back(v[i]);
	}
}

#ifdef __APPLE__
static pascal OSErr open_app_ae_handler(const AppleEvent *apple_event, AppleEvent *reply, SInt32 ref_con)
{
	//LogMsg("Something happene2d!");
	return noErr;
}

void App::AddStartupParm(string parm)
{
	m_startupParms.push_back(parm);
}

static pascal OSErr open_documents_ae_handler(const AppleEvent *apple_event, AppleEvent *reply, SInt32 ref_con)
{
	//never gets called on document double click
	//program does get launched though if it wasn't running
	
	FSRef				vFile;
	AEDescList			docList;
	long				x, numDocs;
	AEKeyword			vKeyword;
	DescType			vOutType;
	Size				vOutActualSize;
	OSErr				vErr;
	
	// Get list of files from Apple Event and count its items:
	vErr = AEGetParamDesc(apple_event, keyDirectObject, typeAEList, &docList );	
	vErr = AECountItems( &docList, &numDocs );
	
	// Go through list and open all  items:
	for( x = 1; x <= numDocs; x++ )
	{
		// Get an FSSpec to our file:
		vErr = AEGetNthPtr( &docList,
							x, 
							typeFSRef, 
							&vKeyword,
							&vOutType, 
							&vFile, 
							sizeof(FSRef), 
							&vOutActualSize );
		
		
		if (vErr != noErr)
		{
			LogError("AEGetNthPtr error of some sort. (%d)", vErr);
			return noErr;
		}
		
		//	Will return the decomposed Unicode name. From there you can convert it to a
		//		CFString and then to a C String.
		HFSUniStr255 name;
		vErr = FSGetCatalogInfo( &vFile, kFSCatInfoNone, NULL, &name, NULL, NULL);

		if (vErr != noErr)
		{
			LogError("FSGetCatalogInfo error of some sort. (%d)", vErr);
			return noErr;
		}
		
		CFStringRef str = CFStringCreateWithCharacters( kCFAllocatorDefault, &name.unicode[0], name.length );
		char s[512], path[512];
		CFStringGetCString(str, s, 512,  kCFStringEncodingMacRoman);
		CFRelease( str );
		
		FSRefMakePath(&vFile, (UInt8*)path, 512);
		if (vErr != noErr)
		{
			LogError("FSRefMakePath error of some sort. (%d)", vErr);
			return noErr;
		}
			
		LogMsg("Opening %s", path, s);
		GetApp()->AddStartupParm(string(path));
	}
	vErr = AEDisposeDesc( &docList );
	
	return noErr;
}
#endif

int App::main(int argc, char **argv)
{
	//first move to our current dir
	CL_Directory::change_to(CL_System::get_exe_path());

#ifdef WIN32
	::SetProcessAffinityMask( ::GetCurrentProcess(), 1 );
#endif

#ifdef __APPLE__
	OSStatus status;

	long response;
	status = Gestalt(gestaltSystemVersion, &response);
	Boolean ok = ((status == noErr) && (response >= 0x00001040));
	if (!ok)
	{
		DialogRef theAlert;
		CreateStandardAlert(kAlertStopAlert, CFSTR("Mac OS X 10.4 (minimum) is required for this application"), NULL, NULL, &theAlert);
		RunStandardAlert(theAlert, NULL, NULL);
		ExitToShell();
	}
	
	char stTemp[512];
	getcwd((char*)&stTemp, 512);
	//	LogMsg("Current working dir is %s", stTemp);
	CL_Directory::change_to("Contents/Resources");
	getcwd((char*)&stTemp, 512);
	LogMsg("Game: Set working dir to %s", stTemp);
	
	//register for file open events
	
	if(AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, open_documents_ae_handler, NULL, false) != noErr)
		LogError("Error installing apple event handler for open documents!");
	
	if(AEInstallEventHandler(kCoreEventClass,  kAEOpenApplication, open_app_ae_handler, NULL, false) != noErr)
		LogError("Error installing apple event handler for open app"); 
	
	//LogMsg("Installed event handlers.");
#endif
	
	for (int i=1; i < argc; i++)
	{
		if (i > 1)
		{
			m_originalParms += " ";
		}
		m_originalParms += argv[i];
	}

	m_baseDirectory = CL_Directory::get_current()+"/"; //save for later

	//load our system prefs
	LoadPrefs();
	
	BuildCommandLineParms();
	
	if ( ParmExists("-h") ||  ParmExists("-help") ||ParmExists("/?") ||ParmExists("/help") || ParmExists("--h") || ParmExists("--help") )
	{
		//let's just show help stuff
		string message = "\nNovashell Game Creation System "+GetEngineVersionAsString()+"\n\n";
		message += 
		
		"Useful parms:\n\n" \
		"	MyWorld.novashell\n" \
		"	-res <desired screen width> <desired screen height> (or use '-res desktop' for current desktop res)\n" \
		"	-window\n" \
		"	-fullscreen\n" \
		"	-nosound\n" \
		"	-nomusic\n" \
		"";

		//		"	-fmodsound (windows only, forces fmod sound system)\n"
		//		"	-clansound (forces Clanlib sound system)\n" \


#ifdef WIN32
		MessageBox(NULL, message.c_str(), "Command line help", MB_ICONINFORMATION);
#else
		cout << message; //hopefully this will just go right to their console
#endif
		return 0;
	}

#ifndef _DEBUG
#define C_APP_REDIRECT_STREAM
	stream_redirector redirect("log.txt", "log.txt");
#endif

	try
    {
		//this is so we can trap clanlib log messages too
		CL_Slot slot_log = CL_Log::sig_log().connect(&log_to_cout);
		
		CL_SetupCore setup_core;
        CL_SetupDisplay setup_display;
        
		//for compatibility
		CL_OpenGL::ignore_extension("GL_EXT_abgr");
		CL_SetupGL setup_gl;
		
    	SetSoundSystem(C_SOUNDSYSTEM_CLANLIB);

#ifdef _WIN32
		//SetSoundSystem(C_SOUNDSYSTEM_FMOD);
		if ( ParmExists("-clansound"))
		{
			SetSoundSystem(C_SOUNDSYSTEM_CLANLIB);
		}

		if ( ParmExists("-fmodsound"))
		{
			SetSoundSystem(C_SOUNDSYSTEM_FMOD);
		}
#endif
	
		if (GetSoundSystem() == C_SOUNDSYSTEM_CLANLIB)
		{
				m_pSetup_sound = new CL_SetupSound();
				m_pSetup_vorbis = new CL_SetupVorbis();
				m_pSound_output = new CL_SoundOutput(44100);
		}

#ifdef _DEBUG
	    // Create a console window for text-output if in debug mode
		CL_ConsoleWindow console("Console");
		console.redirect_stdio();
#endif  

		try
		{
			OneTimeInit();
		
       		if (m_pWindow)
			{
				CL_Slot slot_quit = m_pWindow->sig_window_close().connect(this, &App::OnWindowClose);
       			CL_Slot slot_on_resize = m_pWindow->sig_resize().connect(this, &App::OnWindowResize);
				CL_Slot slot_on_lose_focus = m_pWindow->sig_lost_focus().connect(this, &App::OnLoseFocus);
				CL_Slot slot_on_get_focus = m_pWindow->sig_got_focus().connect(this, &App::OnGotFocus);
				CL_Slot m_slot_button_up = CL_Keyboard::sig_key_up().connect(this, &App::OnKeyUp);
		  
				// Everytime the GUI draws, let's draw the game under it
				CL_Slot m_slotOnPaint = GetGUI()->sig_paint().connect(this, &App::OnRender);
				CL_System::keep_alive(); 

				m_pGameLogic->OneTimeModSetup();
				
				if (!m_pGameLogic || (!m_pGameLogic->Init())) throw CL_Error("Error initting game logic");
			
				ClearTimingAfterLongPause();

				// Class to give us the framerate
				m_pFrameRate = new CL_FramerateCounter();
				//m_pFrameRate->set_fps_limit(60);
				// Run until someone presses escape
		  	
				while (!m_bQuit)
				{
					if (m_HaveFocus)
					{
						if (m_notifyOfScreenChange)
						{
							if (m_pScriptManager)
							{
								m_pScriptManager->UpdateAfterScreenChange(true);
								m_videoCallback.OnActivate();
								m_pGameLogic->OnScreenChanged();
							}
							g_Console.OnScreenChanged();

							m_notifyOfScreenChange = false;
						}
						
						if (m_bRequestVideoInit) ActivateVideoRefresh(m_pWindow->is_fullscreen());

						if (m_bRequestToggleFullscreen) ToggleWindowedMode();

						m_bWindowResizeRequest = false;
	                    
						Update();

						m_bRenderedGameGUI = false;

						if (m_pGameLogic->GetGameGUI())
						{
							m_pGameLogic->GetGameGUI()->show();
						} else
						{
							m_pGui->show();
						}
						
						//the gui is drawn too
						m_bJustRenderedFrame = true; //sometimes we want to know if we just rendered something in the update logic, entities
						//use to check if now is a valid time to see if they are onscreen or not without having to recompute visibility

						 // Flip the display, showing on the screen what we have drawn
						// since last call to flip_display()
						CL_Display::flip(m_videoflipStyle);
	                      
						// This call updates input and performs other "housekeeping"
						// call this each frame
						CL_System::keep_alive(); 
					} else
					{
						//don't currently have focus   
						if (!m_pWindow->is_fullscreen())
						{
							if (GetGameLogic())
							{
								EntChooseScreenMode *pChoose = (EntChooseScreenMode*) EntityMgr->GetEntityByName("choose screen mode");
								{
									if (pChoose)
									{
										if (pChoose->IsGeneratingThumbnails())
										{
											//let's let the game run in the background while its doing this boring
											//duty
											Update();
										}
									}
								}
							}
						}
						CL_System::sleep(1);
						CL_System::keep_alive();
						ClearTimingAfterLongPause();
					}
				}
			}
		}
		catch(CL_Error error)
		{
			LogError("CL_Error Exception: %s",error.message.c_str());
			std::cout << "CL_Error Exception caught : " << error.message << std::endl;			
	        
			// Display console close message and wait for a key
			OneTimeDeinit();

	#ifdef WIN32
			if (error.message.find("WGL_ARB_pbuffer") != std::string::npos)
			{
				MessageBox(NULL, (error.message + "\n\nThis can probably be fixed by installing the latest drivers for your video card.").c_str(), "Error!", MB_ICONEXCLAMATION);
			} else
			{
				MessageBox(NULL, error.message.c_str(), "Error!", MB_ICONEXCLAMATION);
			}
	#endif

	#ifdef _DEBUG
			console.display_close_message();
	#endif
	        
	#ifdef __APPLE__
	#ifdef C_APP_REDIRECT_STREAM
			redirect.DisableRedirection();
			std::cout << "CL_Error Exception caught : " << error.message.c_str() << std::endl;			
	#endif
	#endif
			//PostQuitMessage(0);
		}
    
	    OneTimeDeinit();
	} 
	catch(CL_Error error)
    {
		LogError("Early Exception caught: %s",error.message.c_str());

#ifdef WIN32
        MessageBox(NULL, error.message.c_str(), "Early Error!", MB_ICONEXCLAMATION);
#else
        std::cout << "Early Exception caught : " << error.message.c_str() << std::endl;
#endif

#ifdef __APPLE__
#ifdef C_APP_REDIRECT_STREAM
		redirect.DisableRedirection();
		std::cout << "Early Exception caught : " << error.message.c_str() << std::endl;			
#endif
#endif
		return 0;
    }
	catch (int param)
	{
		std::cout << "Int Exception caught : " << param << std::endl;			
	}
	catch (...)
	{
		std::cout << "Unknown Exception caught : " << std::endl;			
#ifdef __APPLE__
#ifdef C_APP_REDIRECT_STREAM
		redirect.DisableRedirection();
		std::cout << "Unknown Exception caught : " << std::endl;	
#endif
#endif
	}
  
#ifdef __APPLE__
	//DisposeAEEventHandlerUPP(od_handler);
#endif
	return 0;
}

void App::AddCallbackOnResolutionChange(const string &callbackFunction, int entityID)
{
	m_videoCallback.Add(callbackFunction, entityID);
}

void App::OnGameReset()
{
	m_videoCallback.Reset();
}

int App::GetScreenSizeX()
{	
	return m_WindowDescription.get_size().width;
}

int App::GetScreenSizeY()
{	
	return m_WindowDescription.get_size().height;
}

CL_Vector2 App::GetScreenSize()
{
	return CL_Vector2(m_WindowDescription.get_size().width, m_WindowDescription.get_size().height);
}

bool App::GetIsWindowed()
{
	return !CL_Display::is_fullscreen();
	
}

void App::SetIsWindowed( bool bNew )
{
	if (bNew != App::GetIsWindowed())
	{
		RequestToggleFullscreen();
	}
}




//lua natural docs stuff
/*
Object: App
General functions related to the whole application.

About:

This is a global object that can always be accessed.

Usage:

(code)
local gameTicks = GetApp:GetGameTick();
LogMsg("The game's age is: " .. gameTicks);
(end)

Group: Member Functions

func: GetEngineVersion
(code)
number GetEngineVersion()
(end)

Returns:
The current engine version in numerical form, for example, 0.16.

func: GetEngineVersionAsString
(code)
string GetEngineVersionAsString()
(end)

Returns:
The current engine version in string form, for example, "0.16"

func: GetPlatform
(code)
number GetPlatform()
(end)

Returns:
One of the <C_PLATFORM_CONSTANTS>.

func: SetWindowTitle
(code)
nil SetWindowTitle(string newTitle)
(end)
If you don't like the default window text, you can specify your own.  Only visible when tabbed out or in windowed mode.

func: SetCursorVisible
(code)
nil SetCursorVisible(boolean bVisible)
(end)
Parameters:
bVisible - If true, the system cursor (this means the mouse pointer in most cases btw) will be shown, otherwise it's made invisible.

func: GetCursorVisible
(code)
boolean GetCursorVisible()
(end)

Returns: True if the system cursor is currently visible.

func: GetScreenSize
(code)
Vector2 GetScreenSize()
(end)

Returns a <Vector2> object with the current screen size.


func: SetScreenSize
(code)
boolean SetScreenSize(number x, number y);
(end)

Allows on-the-fly screen size changes by script.  Up to you to specify valid resolutions if you're currently fullscreen.

Parameters:

x - The desired screen width
y - The desired screen height

Returns:

True on success.  Except it doesn't detect errors yet, so I guess always true.

func: GetPreLaunchScreenSize
(code)
Vector2 GetPreLaunchScreenSize()
(end)

Returns a <Vector2> object with the size of the screen before novashell was launched. Returns 0,0 if unsupported on the current OS.

func: GetIsWindowed
(code)
boolean GetIsWindowed()
(end)

Returns:

Returns true if the screen is currently in windowed mode

func: SetIsWindowed
(code)
nil SetIsWindowed(boolean bWindowed)
(end)

Changes the video resolution to full screen or windowed mode.  Up to you to make sure the video card can handle it currently...

func: ParmExists
(code)
boolean ParmExists(string parmToFind)
(end)

Allows you to check if the game was run with a certain command line parameter or not.

Parameters:

parmToFind - The parm you want to check for, for example, "-cheatmode" or something.

Returns:

True if the parm was found.

func: GetTick
(code)
number GetTick()
(end)
The current system time. Generally, you want to use <GetGameTick> instead.

Returns:

The current system time in milliseconds.

func: GetGameTick
(code)
number GetGameTick()
(end)

This is what you use to time things.

* This always starts at 0 when a new game/profile is started
* It's persistent, ie, it's automatically saved and loaded with the profile
* Is aware of pausing and changing the game speed.  If the game is running at 2X, this advances twice as fast too.

Returns:

The current game time in milliseconds.

func: SetSimulationSpeedMod
(code)
nil SetSimulationSpeedMod( number speedModifier)
(end)

Setting this to 2 means the game will operate twice as fast. Setting to 0.5 would mean half speed (slow mooootion).

Game Design Note:
By default, TAB is assigned to make the game run fast using this, it's good for testing and I recommend that you leave it in as it's handy for skipping slow sequences for players too.

Worried people will run super fast and cheat?:

It can't be used to cheat, because, well, everything is going fast, not just the player.

func: GetSimulationSpeedMod
(code)
number GetSimulationSpeedMod()
(end)

Return:

Gets the current simulation speed mod.


func: AddCallbackOnResolutionChange
(code)
nil AddCallbackOnResolutionChange(string callbackName, number entityID)
(end)
Allows you to have an entity or global function notified if the screen resolution changes.  The callback is automatically deleted
when an <Entity> dies. In the case of a global function, there is no way to unregister it.

Usage:
(code)
//Inside of the an entities OnInit()
	GetApp:AddCallbackOnResolutionChange("OnResChange", this:GetID());

//Then, elsewhere in the script, define the function "OnResChange"...

function OnResChange()
	LogMsg("interface.lua>> Hey, the screen resolution changed to " .. C_SCREEN_X .. " " .. C_SCREEN_Y .. "!");
//Do stuff like move the GUI or whatever
end
(end)

Parameters:

string callBackName - The name of the callback function that should be called. (Must exist in your code)
entityID - If it's an <Entity> function and not a global function, you'll need to enter the <Entity>'s ID here.  Otherwise, <C_ENTITY_NONE>.


Section: Related Constants

Group: C_PLATFORM_CONSTANTS

constants: C_PLATFORM_WINDOWS

constants: C_PLATFORM_OSX

constants: C_PLATFORM_LINUX

*/