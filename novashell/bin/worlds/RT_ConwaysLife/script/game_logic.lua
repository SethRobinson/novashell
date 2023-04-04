

function OnInit() //before the entity is placed in the world, init visuals here, so it knows the size dimensions
	this:SetPersistent(false); //don't save us with the map
	LogMsg("Initting gamelogic object");
	this:SetImage("lua_script.png", nil); //the image we'll show so we can be seen in  the editor, dunno why we care though
	//this:SetAttach(C_ENTITY_CAMERA, Vector2(0,0); //so we are always on screen and processed	
	this:SetName("GameLogic"); //so we can find it by name later

	//Create a new entity so we can attach our custom C++ brain to it
		CreateEntity(GetMapManager:GetActiveMap(), Vector2(0,0), "entity/life.lua"); 	

end

function OnPostInit() //run during the entity's first logic update
	this:SetLayerByName("Hidden Data"); //we can only see the image when using the editor

	
	//show one time message in a bit
	//g_SmartSound:SetMusic("audio/moremusic.ogg", true, true);
		
end


function OnKill()  //run when removed
end