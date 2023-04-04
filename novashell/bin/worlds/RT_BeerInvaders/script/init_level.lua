//this controls our main game logic

function OnAddEnemy()
	//g_EnemiesLeft = g_EnemiesLeft + 1;
end

function OnRemoveEnemy()
	g_EnemiesLeft = g_EnemiesLeft - 1;
	
	if (g_EnemiesLeft < 1) then
	  LogMsg("g_EnemiesLeft is", g_EnemiesLeft);
	  OnCompleteLevel();
	end
end

function InitLevel(levelNum)

	g_levelNum = levelNum;
	g_EnemiesLeft = 0;

	GetSoundManager:Play("audio/start_music.ogg");
	
	ClearScreen(); //inits a blank screen and puts the camera at 0,0
	
	InitPlayer(); //well, the gun thingie
	
	GetApp:SetCursorVisible(false); //so we can't see the mouse
	
	local timeTaken = ShowGetReadyText(levelNum);
	
	SpawnEnemies(levelNum, timeTaken);
	
end

function InitPlayer()

	local thisMap = GetMapManager:GetActiveMap();
	
	local vecGunStartingY = C_SCREEN_Y - 128;
	
	//copy and paste a new gun thing
	local entGun = GetEntityByName("gun_template"):Clone(thisMap, Vector2(C_SCREEN_X/2, vecGunStartingY));
	entGun:SetName("Gun"); //make it easy to find later
end

function ClearScreen()
	
	//kill any left over game pieces so we can start with a blank map.  This only works right
	//because we have the map set as not persistent, so it doesn't auto-save when a real game
	//is going.

	GetMapManager:UnloadMapByName("Main");

	//switch back to it and set our default view
	GetMapManager:SetActiveMapByName("Main");
	
	GetCamera:SetScale(Vector2(1,1)); //1 to 1 pixel ratio
    GetCamera:SetPos(Vector2(0,0)); //set camera position of upper left corner in world coordinates
    GetCamera:InstantUpdate(); //make sure it updates now
	
end

function ShowText(msg)

		local entText = GetEntityByName("text_template"):Clone(GetMapManager:GetActiveMap(), Vector2(C_SCREEN_X/2, (C_SCREEN_Y/2)+25  ));
		entText:RunFunction("SetText", msg);
		entText:RunFunction("SetTextScale", Vector2(2,2));
		entText:GetBrainManager():Add("FadeIn", "fade_speed_ms=300");		

		//make the text disappear in a bit
		Schedule(2000, entText:GetID(), "this:GetBrainManager():Add('FadeOutAndDelete','fade_speed_ms=300')");

end

function ShowGetReadyText(levelNum)

		Schedule(1000, C_ENTITY_NONE, "ShowText('Level " .. levelNum .. "')");
	
	if (levelNum == 1) then
		//give them some extra directions
		Schedule(4000, C_ENTITY_NONE, "ShowText('Use the mouse to move left and right.\\nClick to fire!')");
	end
	

	return 3400; //return the time when the next event should happen
end

function SpawnEnemy(name, x, y)

	//LogMsg("Spawning " .. name .. " At " .. x .. " " .. y);

	//copy and paste a new enemy
	local ent = GetEntityByName(name):Clone(GetMapManager:GetActiveMap(), Vector2(x, y));

	//play a sound effect with a little speed mod effect applied
	local soundID = ent:PlaySoundPositioned("audio/spawn_enemy.ogg");
	GetSoundManager:SetSpeedFactor(soundID, 1+random());
	
	ent:SetName(""); //remove the custom name, waste of space since we don't need to find it by name later

end;


function SpawnEnemies(levelNum, timeToStartMS)

local enemyName = "can_template";
local startingX = 100;
local y = 150;
local x = startingX;
local spaceBetweenEnemiesY = 90;
local spaceBetweenEnemiesX = 130;
local enemiesToSpawn = 5 * levelNum;
g_EnemiesLeft = enemiesToSpawn;
local timeBetweenSpawnsMS = 60; //1000 would mean wait one second before spawning each enemy

	//loop through and build each enemy
	for i=1, enemiesToSpawn do
		//schedule when the enemy should spawn, so they pop-up in rows instead of instantly, looks cooler
		Schedule(timeToStartMS, C_ENTITY_NONE, "SpawnEnemy('" .. enemyName .. "'," .. x .. "," .. y .. ")");

		//get it ready for the next spawn
		timeToStartMS = timeToStartMS + timeBetweenSpawnsMS;
		x = x + spaceBetweenEnemiesX;
		if (x + spaceBetweenEnemiesX > C_SCREEN_X) then
			//uh oh, we're pretty far off to the right, let's go down to the next row for the next spawn
			x = startingX;
			y = y + spaceBetweenEnemiesY;
		end
		
	end
	
end

function OnPlayerDeath()

	GetSoundManager:Play("audio/end_music.ogg");
	Schedule(1000, C_ENTITY_NONE, "ShowText('GAME OVER')");
	Schedule(4000, C_ENTITY_NONE, "GetGameLogic:SetRestartEngineFlag(true)");
	
end

function OnCompleteLevel()

	GetSoundManager:Play("audio/end_music.ogg");
	Schedule(1000, C_ENTITY_NONE, "ShowText('Level complete!')");
	Schedule(4000, C_ENTITY_NONE, "InitLevel(" .. g_levelNum+1 .. ")");


end