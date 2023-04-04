//Startup a game loop, this function gets called after all entities run their logic each frame

GetGameLogic:AddCallbackOnPostLogicUpdate("OnGameLoop", C_ENTITY_NONE);


//this is run every logic tick
function OnGameLoop(step)
	
	if (g_numPlayers  == 0) then
	  //no game is active right now
		return;
	end;
	
	
	//shake the camera randomly.. earthquake!
	//GetCamera:SetPos(GetCamera:GetPos() + Vector2(10-Random(20), 10-Random(20)));
	
	CalculateCameraPosition();
end


function CalculateCameraPosition()

//let's manually figure out the exact center of all active players, and the enclosing rect that fits all of them
	
	  local vCenterPos = Vector2(0,0);
	  local distance = 0;
		local enclosingRect;
		local cameraPadding = 50; //how much space around the player we want the camera to require when calculation zoom
		
		for i=0, g_numPlayers-1, 1 do

			local playEnt = GetEntityByID(g_PlayID[i]);
			
			if (playEnt == nil) then
				//the player names have changed?
				return;
			end
			//LogMsg(newPlayer:GetPos());
			vCenterPos = vCenterPos + playEnt:GetPos();
			
			//get this players world rect (rectangular area that his picture is in), and pad it out to make it bigger
		  local tempRect = PadRect(playEnt:GetWorldRect(), cameraPadding);
			
			//let's also limit it to our active map area, only returning the part that is inside it
				
			tempRect = GetMapManager:GetActiveMap():GetWorldRect():CalculateUnion(tempRect);

			if (i == 0) then
				enclosingRect = tempRect;
			else
			
				//add it to our total rect that encloses all players
				enclosingRect = tempRect:CalculateCombined(enclosingRect);
			end
			
		end

	 //LogMsg("Enclosing Rect: ", enclosingRect);
	//define by players to get the real center position of all of them
		vCenterPos = vCenterPos / g_numPlayers;
		
		//set the final camera center position
		
		GetCamera:SetPosCenteredTarget(vCenterPos);
		
		//calculate a zoom level needed to see all players
		
		local zoomWidth = C_SCREEN_X / enclosingRect:GetWidth();
		local zoomHeight = C_SCREEN_Y / enclosingRect:GetHeight();
		
		//limit it so it won't zoom in more than 2x, which is our goal, 320X240 but screen doubled to 640X480
		zoomWidth = math.min(zoomWidth, 2);
		zoomHeight = math.min(zoomHeight, 2);
		
		//now, it looks totally funky to have the x/y scaled differently (players get fat/thin, etc) so we'll just
		//require the same scale
		
		local finalScale = math.min(zoomWidth, zoomHeight);
		
		GetCamera:SetScaleTarget(Vector2(finalScale, finalScale));

end



