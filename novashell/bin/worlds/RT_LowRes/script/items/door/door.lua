function OnInit() //run upon initialization
	this:SetVisualProfile("~/door.xml", "door");
	this:LoadCollisionInfo("~/door.col");
	this:SetDensity(0);

	this:SetAnimByName("normal");
	this:SetAnimPause(true); 
	
end

function OnPostInit() //run during the entity's first logic update
	this:SetEnableRotationPhysics(false);
	this:SetCollisionCategories(C_CATEGORIES_NONE);
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
end

function OnKill() //run when removed
end

function OnSave()
end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	//we're only listening to players, so that must be what just touched us.
	
	
	//unload this map and load a new one.. not a good idea to do it
	//within an entity ON  the map, so we'll schedule it..
	
			Schedule(1, C_ENTITY_NONE, "GetMapManager:UnloadMapByName(GetMapManager:GetActiveMap():GetName())");
	
		if (g_curLevel == C_LEVEL_WON) then
		
			Schedule(2, C_ENTITY_NONE, "InitMainMenu()");
		else
			Schedule(2, C_ENTITY_NONE, "SetupLevel(" .. g_curLevel +1 ..")");
		end
	

//	Schedule(400, C_ENTITY_NONE,
	//		'GetTextManager:AddCustomScreen("Here\'s that same level again!", Vector2(C_SCREEN_X/2,C_SCREEN_Y-40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);'
		// );
	
end


