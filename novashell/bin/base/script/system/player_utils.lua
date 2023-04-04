//Misc helper functions to help with human controlled players


//Converts four bool key inputs to a C_FACING direction

function ConvertKeysToFacing(bLeft, bRight, bUp, bDown)

	if (bLeft == false and bRight == false and bUp == false and bDown == false) then
		//no keys are pressed
		return C_FACING_NONE;
	end
	
	local facing;
		
	if (bLeft) then
		facing = C_FACING_LEFT;		
	end

	if (bRight) then
		facing = C_FACING_RIGHT;		
	end

	if (bDown) then
		facing = C_FACING_DOWN;
	end

	if (bUp) then
		facing = C_FACING_UP;
	end

	//also check for diagonals
	
	if (bDown and bLeft) then
		facing = C_FACING_DOWN_LEFT;

	elseif (bUp and bLeft) then
		facing = C_FACING_UP_LEFT;

	elseif (bDown and bRight) then
		facing = C_FACING_DOWN_RIGHT;

	elseif (bUp and bRight) then
		facing = C_FACING_UP_RIGHT;
	end

	return facing;
end


//set this as the default player if one isn't already set, this way even if you cut
//and paste players in the editor the camera will figure out who to track

function AssignPlayerToCameraIfNeeded(playerEnt)
	
	if (GetPlayer == nil) then
		LogMsg("Setting player to " .. tostring(playerEnt));
		GetGameLogic:SetPlayer(playerEnt);

		if (g_editorActive == false) then
			GetCamera:SetEntityTrackingByID(playerEnt:GetID());
			LogMsg("Setting camera tracking to player " .. tostring(playerEnt));
			GetCamera:InstantUpdate();
		end

	else
		//LogMsg("Player is " .. tostring(GetPlayer));
	end
end


//upon deletion, a player can call this to let the engine know it's gone.  That way,
//AssignPlayerToCameraIfNeeded will kick in if another suitable entity is running.

function RemoveActivePlayerIfNeeded(playerEnt)
	if (GetPlayer and GetPlayer:GetID() == playerEnt:GetID()) then
		LogMsg("Main player removed");
		GetGameLogic:SetPlayer(nil);
		GetCamera:SetEntityTrackingByID(C_ENTITY_NONE);
	end
end

