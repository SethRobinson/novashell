function SetupGame()
	
	GetMapManager:UnloadMapByName("Intro");
	GetMapManager:SetActiveMapByName("Game");

	//Get our main game logic going
	CreateEntity(GetMapManager:GetActiveMap(), Vector2(0,0), "game_logic.lua"); 	

end