function AttachPointer(entToAttachTo, offsetY)
	
	if (offsetY == nil) then offsetY = 0; end;
	local ent = CreateEntity(entToAttachTo:GetMap(), Vector2(0,0), "items/pointer/pointer.lua");
	ent:SetLayerByName("GUI 1"); //overlay  the rest of the action
	ent:SetScale(Vector2(0.5,0.5));
	ent:SetPersistent(false); //don't save with the map
	ent:SetAttach(entToAttachTo:GetID(), Vector2(0,-(entToAttachTo:GetSizeY() + offsetY))); //so it stays on the screen

end