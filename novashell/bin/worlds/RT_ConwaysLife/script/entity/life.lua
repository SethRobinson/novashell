

function OnInit() //before the entity is placed in the world, init visuals here, so it knows the size dimensions
	this:SetPersistent(false); //don't save us with the map
	
	this:SetImage("invisible.png", nil); //the image we'll show so we can be seen in  the editor, dunno why we care though
	//this:SetAttach(C_ENTITY_CAMERA, Vector2(0,0); //so we are always on screen and processed	
	this:SetName("Life"); //so we can find it by name later
	this:GetBrainManager():Add("Life",""); //our C++ brain, it also overrides its size

end


function OnPostInit() //run during the entity's first logic update
	//this:SetLayerByName("Hidden Data"); //we can only see the image when using the editor
	
	//show one time message in a bit
	//g_SmartSound:SetMusic("audio/moremusic.ogg", true, true);
	this:GetBrainManager():SendToBrainByName("Life", "init=100=10;add_random=700;set_update_ms=1");
	
	//this:GetBrainManager():SendToBrainByName("Life", "set_cell=5=5=true");
	
end


function OnKill()  //run when removed
end