
function SetupMainMenu()
		
	GetMapManager:UnloadMapByName("Intro");
	GetMapManager:UnloadMapByName("Game");

	GetMapManager:SetActiveMapByName("Intro");
	
	//If we wanted music..
	
	//g_SmartSound:SetMusic("audio/mysong.ogg", false, false);
	
	//if we wanted a slideshow to play in the bg
	
//	entSlideShow = CreateEntity(GetMapManager:GetActiveMap(), Vector2(0,0), "system/slideshow.lua"); 	
//	entSlideShow:RunFunction("AddSlide", "intro0.png");
//	entSlideShow:RunFunction("AddSlide", "intro2.png");
//	entSlideShow:RunFunction("AddSlide", "intro1.png");
//	entSlideShow:RunFunction("AddSlide", "intro3.png");
//	
//	entSlideShow:RunFunction("Init", true, false); //bLooping, bSkippable

end

function OnMainMenuStart()

	LogMsg("Clicked start");
	SetupGame();
end


function OnMainMenuQuit()

	GetGameLogic:Quit(); //quits the game
end