//I use this with a "run line of script" helper, just setup the helper to run:
//RunScript("char/hollowdale/whatever this is.lua");
	
		//let's do a little cutscene of him waking up, illustrates how we can take control of a
		//character and have him do things
		
		PlayBattleMusic() //from audio_utils.lua which was loaded in game_start.lua
	
	
	   local goal = GetPlayer:GetGoalManager():PushNewGoal("TrollAttack");
	  goal:AddMoveToPosition(GetPlayer:GetPos()+Vector2(0,70)); //walk down a bit
	  goal:AddDelay(500); //wait a second before starting
	  goal:AddSay("What a beautiful day!", C_FACING_NONE);
    goal:AddDelay(C_TIME_FOREVER); //wait forever, the other character will have to kill this manually when he's done talking
		
		//create a troll and have it talk smack
			
		 local ent = GetEntityByName("template_troll"):Clone(GetMapManager:GetActiveMap(), GetTagManager:GetPosFromName("Hol_TrollSpawnPoint"));
		 GetWatchManager:Add(ent, 1000*10); //can function offscreen for up to 10 seconds, enough time to walk up to the player
		 local entGoal = ent:GetGoalManager():PushNewGoal("Talk");
	   entGoal:AddDelay(1000);
	   entGoal:AddApproachAndSay("..for a freakin' troll attack!", g_playerID, C_DISTANCE_YELL);
	   entGoal:AddSayByID("What the?  No trolls allowed in the village!", g_playerID, ent:GetID());
	   entGoal:AddSayByID("O RLY?", ent:GetID(), g_playerID);
	   entGoal:AddRunScriptString("GetPlayer:GetGoalManager():GetGoalByName('TrollAttack'):Kill();");
	   entGoal:AddSayByID("Die, fiend!", g_playerID, ent:GetID());
			 
	