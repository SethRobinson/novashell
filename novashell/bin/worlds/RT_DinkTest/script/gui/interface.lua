//this will be an invisible entity that holds all GUI items, like showing how many coins we picked up.  To move all GUI things together,
//we could just move this, as all our GUI pieces (which are just entities) are attached to it.


function OnInit() //run upon initialization
		
	//load the interface BG
	this:SetImage("~/STAT-03.BMP", nil);
	this:SetAlignment(C_ORIGIN_TOP_LEFT, Vector2(0,0));
	//don't save us, we'd rather be rebuilt each time
	this:SetPersistent(false);

	GetApp:AddCallbackOnResolutionChange("OnResChange", this:GetID());
	
end

function OnResChange()

  LogMsg("interface.lua>> Hey, the screen resolution changed to " .. C_SCREEN_X .. " " .. C_SCREEN_Y .. "!");

	SetGUIPosition();
	
end

function SetGUIPosition()

		local v = Vector2(C_SCREEN_X/2,C_SCREEN_Y-this:GetSizeY());
		v.x = v.x - this:GetSizeX()/2;

	  this:SetAttach(C_ENTITY_CAMERA, v); //let the scrollbar stay on the screen
end

function OnMapInsert() //run as it's being place on the map

		SetGUIPosition();
	
		//connecting it to the camera here insures its OnPostInit() will be run, otherwise if it wasn't
		//at least partially onscreen it's logic would never get kickstarted to run	
		
	 	//let's just save our ID as a global
			_G.g_interfaceID = this:GetID();

end

function OnPostInit() //run during the entity's first logic update
		
		LogMsg("Initting interface");

		m_fadeInSpeedMS = 800;

		this:SetLayerByName("GUI 1"); //put it over the other junk on the screen
		this:GetBrainManager():Add("FadeIn", "fade_speed_ms=" .. m_fadeInSpeedMS);		
		SetupGUIOverlays();
		
		//let's have a function that gets run every logic tick
		this:SetRunUpdateEveryFrame(true);
		m_bRequestUpdate = true;

end

function RequestUpdate()

	m_bRequestUpdate = true;
end

//this is run every logic tick
function Update(step)
	
	if (m_bRequestUpdate and GetPlayer) then
		m_bRequestUpdate = false;
		
		//let's refresh the display with any changed numbers
		//LogMsg("Player has " .. GetPlayer:Data():Get("hp") .. " hitpoints.");
	
	end

end

function SetupGUIOverlays()

	//well, we'll need a health bar, won't we.
	LogMsg("Setting up overlays..");
	
		
end