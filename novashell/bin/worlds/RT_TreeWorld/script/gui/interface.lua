//this will be an invisible entity that holds all GUI items, like showing how many coins we picked up.  To move all GUI things together,
//we could just move this, as all our GUI pieces (which are just entities) are attached to it.

function OnInit() //run upon initialization
		
	//load the interface BG
	this:SetImage("invisible.png", nil); //from base
	this:SetAlignment(C_ORIGIN_TOP_LEFT, Vector2(0,0));
	//don't save us, we'd rather be rebuilt each time
	this:SetPersistent(false);
	
end

function OnMapInsert() //run as it's being place on the map

	LogMsg("Inserting interface..");
	
		this:SetAttach(C_ENTITY_CAMERA, Vector2(0,0)); 

		//connecting it to the camera here insures its OnPostInit() will be run, otherwise if it wasn't
		//at least partially onscreen it's logic would never get kickstarted to run	
		
		_G.g_interfaceID = this:GetID();

end

function OnPostInit() //run during the entity's first logic update
	
		LogMsg("Initting interface");

		m_fadeInSpeedMS = 800;

		//at all times

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
		local coinCount = GetPlayer:Data():GetNum("coins");
		m_coin_text:RunFunction("SetText", coinCount .. " of " .. g_totalCoinsPlaced);
	
	end

end

function SetupGUIOverlays()

	LogMsg("Setting up overlays..");
	
	//setup coin icon
	
	m_coin = CreateEntity(nil, Vector2(0,0), "items/coin/coin.lua");
	m_coin:LoadCollisionInfo(""); //none is fine

	m_coin:SetLayerByName("GUI 1"); //overlay  the rest of the action
	m_coin:SetScale(Vector2(1.5,1.5));
	m_coin:SetAttach(this:GetID(), Vector2(30,35)); //so it stays on the screen
	m_coin:SetPersistent(false); //don't save it
	m_coin:SetDensity(0); //don't have it falling and stuff
	m_coin:SetName("inter_coin"); //doesn't matter, but makes debugging easier
	m_coin:SetAnimPause(true); //don't have it rotating..although that might be neat?
	m_coin:SetLockedScale(true); //don't allow scaling when the camera zooms, it looks weird

	//m_coin:SetAlignment(C_ORIGIN_TOP_LEFT, Vector2(0,0));
	//setup text that shows how many coins we have
	
	m_coin_text = GetEntityByName("text_template"):Clone(nil, Vector2(0,0));
	m_coin_text:SetPersistent(false);
	m_coin_text:SetAttach(this:GetID(), Vector2(140,40));
	m_coin_text:SetName("inter_coin_counter");  //doesn't matter, but makes debugging easier
	m_coin_text:SetLayerByName("GUI 1");
	m_coin_text:RunFunction("SetTextScale", Vector2(2,2));
	m_coin_text:SetTextAlignment(C_ORIGIN_CENTER_LEFT, Vector2(0,0));
	m_coin_text:SetLockedScale(true); //don't allow scaling when the camera zooms, it looks weird

end