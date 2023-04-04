
function OnInit() //run upon initialization

	if (GameIsActive()) then
		this:SetVisibilityNotifications(true); //OnGetVisible and OnLoseVisible will get called now
	end

end

function OnPostInit() //run during the entity's first logic update
end

function OnKill() //run when removed
end

function OnGetVisible()



		Schedule(1000, C_ENTITY_NONE,
			'GetTextManager:AddCustomScreen("Use the arrow keys to move.", Vector2(C_SCREEN_X/2,40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);'
		 );
		Schedule(6000, C_ENTITY_NONE,
			'GetTextManager:AddCustomScreen("Press SPACE to examine/talk to the object you are facing.", Vector2(C_SCREEN_X/2,40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);'
		 );

		Schedule(11000, C_ENTITY_NONE,
			'GetTextManager:AddCustomScreen("Press CTRL to punch.", Vector2(C_SCREEN_X/2,40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);'
		 );

		Schedule(16000, C_ENTITY_NONE,
			'GetTextManager:AddCustomScreen("Hold TAB to speedup the game", Vector2(C_SCREEN_X/2,40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);'
		 );

		Schedule(22000, C_ENTITY_NONE,
			'GetTextManager:AddCustomScreen("F1 to toggle edit mode", Vector2(C_SCREEN_X/2,40), Vector2(0, 0), Color(200,200,0,255), 3500, C_FONT_DEFAULT);'
		 );
	
	//destroy this so we don't see this the next time we come in here
	this:SetDeleteFlag(true);

end

function OnLoseVisible()
end

