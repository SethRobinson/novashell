//by calling this, we create an entity that will control the creation/updating of the interface.
//If that entity is killed, the entire interface will be removed, as everything will be set to be
//attached to it.  (When something dies, anything attached to it also dies)

//this basically just inits an entity with the lua file name sent in, and assuming that entity puts its ID() in g_interfaceID, it
//will be smart enough to not init it already exists.


function InterfaceInit(interfaceScriptFileName)

	
		if (VariableExists("g_interfaceID") == false) then
			_G.g_interfaceID = nil; //set a system wide global, just in case other things want to see if its active or not
		end
	
	
	if (GameIsActive()) then
	
		if (_G.g_interfaceID == nil) then
			local ent = CreateEntity(GetMapManager:GetActiveMap(), Vector2(0,0), interfaceScriptFileName);
				end
	
	end
	
end

function KillInterfaceIfNeeded()

	if (g_interfaceID != nil) then
			 local ent = GetEntityByID(g_interfaceID);
			 if (ent) then
			 	ent:SetDeleteFlag(true);
			 end

		 _G.g_interfaceID = nil;
	 end

end


