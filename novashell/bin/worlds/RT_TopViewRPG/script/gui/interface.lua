//this will be an invisible entity that holds all GUI items, like showing how many coins we picked up.  To move all GUI things together,
//we could just move this, as all our GUI pieces (which are just entities) are attached to it.

function OnInit() //run upon initialization
		
	//load the interface BG
	this:SetImage("~/interface_bg.png", nil);
	this:SetAlignment(C_ORIGIN_TOP_LEFT, Vector2(0,0));
	//don't save us, we'd rather be rebuilt each time
	this:SetPersistent(false);
	this:SetPriorityLevel(C_PRIORITY_LEVEL_HIGH); //so it updates even while dialogs are active
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
		
		m_healthBar:RunFunction("SetNum", GetPlayer:RunFunction("GetHP"));
		//LogMsg("Player has " .. GetPlayer:Data():Get("hp") .. " hitpoints.");
		m_coin_text:RunFunction("SetText", GetPlayer:Data():GetNum("Money"));

		m_health_text:RunFunction("SetText", GetPlayer:RunFunction("GetHP") .. "/" .. GetPlayer:RunFunction("GetMaxHP"));

		m_strength_text:RunFunction("SetText", GetPlayer:RunFunction("GetStrength")); 
	end

end

function SetupGUIOverlays()

	LogMsg("Setting up overlays..");

	//well, we'll need a health bar, won't we.
	
	m_healthBar = this:CreateEntity(this:GetMap(), Vector2(0,0), "gui/bar.lua");
	//preload these so they can be accessed as map/tilepics from anywhere from only the file name
	this:AddImageToMapCache("~/life_bar_overlay.png");
	m_healthBar:RunFunction("Setup", "Horizontal", "life_bar_overlay", 10, 20, C_ORIGIN_TOP_LEFT);
	m_healthBar:SetAttach(this:GetID(), Vector2(13,14)); //attach it to us
	m_healthBar:GetBrainManager():Add("FadeIn", "fade_speed_ms=" .. m_fadeInSpeedMS);		
	m_healthBar:SetLockedScale(true); //don't scale when the camera zooms, it looks weird
	
	//health info in text form, shown to the right of the health bar
		
	m_health_text = GetEntityByName("text_template"):Clone(nil, Vector2(0,0));
	m_health_text:SetPersistent(false);
	m_health_text:SetAttach(this:GetID(), Vector2(272,41));
	m_health_text:SetName("inter_health_text");  //doesn't matter, but makes debugging easier
	m_health_text:SetLayerByName("GUI 2");
	m_health_text:RunFunction("SetTextScale", Vector2(1,1)); //how big we want it
	m_health_text:SetTextAlignment(C_ORIGIN_CENTER, Vector2(0,0));
	m_health_text:SetLockedScale(true); //don't allow scaling when the camera zooms, it looks weird
	m_health_text:SetPriorityLevel(C_PRIORITY_LEVEL_HIGH); //so it updates even while dialogs are active
	//coin text
	
	m_coin_text = GetEntityByName("text_template"):Clone(nil, Vector2(0,0));
	m_coin_text:SetPersistent(false);
	m_coin_text:SetAttach(this:GetID(), Vector2(448,17));
	m_coin_text:SetName("inter_coin");  //doesn't matter, but makes debugging easier
	m_coin_text:SetLayerByName("GUI 2");
	m_coin_text:RunFunction("SetTextScale", Vector2(1,1)); //how big we want it
	m_coin_text:SetTextAlignment(C_ORIGIN_CENTER_RIGHT, Vector2(0,0));
	m_coin_text:SetLockedScale(true); //don't allow scaling when the camera zooms, it looks weird
	m_coin_text:SetPriorityLevel(C_PRIORITY_LEVEL_HIGH); //so it updates even while dialogs are active

  //strength text
	
	m_strength_text = GetEntityByName("text_template"):Clone(nil, Vector2(0,0));
	m_strength_text:SetPersistent(false);
	m_strength_text:SetAttach(this:GetID(), Vector2(448,43));
	m_strength_text:SetName("inter_strength");  //doesn't matter, but makes debugging easier
	m_strength_text:SetLayerByName("GUI 2");
	m_strength_text:RunFunction("SetTextScale", Vector2(1,1)); //how big we want it
	m_strength_text:SetTextAlignment(C_ORIGIN_CENTER_RIGHT, Vector2(0,0));
	m_strength_text:SetLockedScale(true); //don't allow scaling when the camera zooms, it looks weird
	m_strength_text:SetPriorityLevel(C_PRIORITY_LEVEL_HIGH); //so it updates even while dialogs are active

	
end