//Our player controlled character.
RunScript("system/player_utils.lua"); //has some useful global functions, this is in base

m_walkLoopingSound = LoopingSound("audio/walk_loop.ogg");
m_climbLoopingSound = LoopingSound("audio/climb_loop.ogg");


function OnInit()
	
	this:SetVisualProfile("~/player.xml", "player");
	this:LoadCollisionInfo("~/player.col");
	
end

function OnMapInsert() //run as the entity is placed on an actual map, but before OnPostInit()
	if (string.findnocase(this:GetMap():GetName(), "palette")) then
		//the map its on has "Palette" in its name, let's assume we don't really want
		//to activate its AI
		else
			GetWatchManager:Add(this, C_TIME_FOREVER); //always function offscreen
		end
		
end

function SetDefaults()
	this:SetCategory(C_CATEGORY_ENTITY, false);
	this:SetCategory(C_CATEGORY_PLAYER, true);

	this:SetType(C_TYPE_CHARACTER);
  m_bJumpKeyDown = false;
	m_walkSpeed = 4;
	m_climbSpeed = 5; //how fast we go up and down ladders
	m_jumpSound = "audio/jump.ogg";
	m_JumpPower = -6; //controls the height of your jump.  Note, you may want to modifty this to be a two
	//or three burst system depending on if the key is still held down, to be able to control jump height
	m_ladderSensitivity = 10; //how many pixels to look up and down to verify a ladder really is starting or ending
	m_bOnLadder = false; //keep track of if we're currently climbing a ladder or not
	m_bCheckForDoor = false; //if yes, we'll scan for a door.
	m_delayBetweenJumpsMS = 200; //this controls how often you can jump	
	//this:SetGravityOverride(0.02);
	this:Data():SetIfNull("coins", "0");
	m_delayBetweenAttacksMS = 100;
	m_attackTimer = 0;
	this:GetBrainManager():Add("StandardBase","");
	
	//limit movement to horizontal only
	this:GetBrainManager():SendToBrainBase("control_filtery=0");
	this:GetBrainManager():SendToBrainBase("air_control_filterx=0.3;air_control_filtery=0");

	this:SetCollisionListenCategory(C_CATEGORY_TILEPIC, true);
	this:SetCollisionListenCategory(C_CATEGORY_ENTITY, true);
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
	
	this:SetDefaultTalkColor(Color(255,255,150,255));

	this:SetMaxMovementSpeed(2000); //ladder, there could be stacked brains that try to force the player to dodge bulletins etc, this sets max for all brains
	this:SetDesiredSpeed(m_walkSpeed);
	this:SetTurnSpeed(10); //instant
	this:SetDampening(0); //don't need extra ground dampening, the brain uses the players "movement power" to set speed to
	//zero when in idle state

	m_invincibilityTimer = 0; //means not invincible
	m_jumpSfxTimer = 0;
	m_jumpSfxMinTimeBetweenSoundsMS = 150;
	m_jumpTimer = 0;
	m_lastDirection = C_FACING_LEFT;
	ResetKeys()
end
	


function OnPostInit()

end

function InitPlayer(playerNum)

	SetDefaults();
	Startup();
	SetupInputKeys(playerNum);
	
	if (playerNum == 0) then
		this:SetName("Player");
	end

	if (playerNum == 1) then
		this:SetName("Player2");
		this:SetBaseColor(Color(0, 255, 255,255)); //so it looks different
	end
	
	if (playerNum == 2) then
		this:SetName("Player3");
		this:SetBaseColor(Color(255, 0, 255,255)); //so it looks different
	end
	
	if (playerNum == 3) then
		this:SetName("Player4");
		this:SetBaseColor(Color(100, 160, 255,255)); //so it looks different
	end
	
	//let's have a function that gets run every logic tick
	this:SetRunUpdateEveryFrame(true);


end


//this is run every logic tick
function Update(step)
	
	
	//AssignPlayerToCameraIfNeeded(this);  //make us the official player and have the camera track us, if we're not already,
	//and if another entity doesn't already have this role
	
	//if our goal manager is doing something, we'd better not allow ourselves to be humanly
	//controlled, should be useful for cutscenes and the like
	
	if (this:GetGoalManager():GetGoalCount() == 0) then
		
		ProcessLadder(); //handles the ladder checking/climbing modes
		
		if (m_bOnLadder == false) then
			
			if (this:GetBrainManager():InState("Attack") == false ) then
				ProcessMovement(step);
		
				if (m_bAction) then	ProcessAction(); end; //they hit the action key
				if (m_bAttack) then	ProcessAttack(); end; //they hit the attack key
				if (m_bCheckForDoor) then ProcessDoor(); m_bCheckForDoor = false; end; //see if there is a door/warp they can use
				
				CheckForJump();
			
			end
		else		
				ProcessMovementLadder(step);
		end
			
	else
		//we're in a cut scene.  Maybe we should letter box or show a "In cutscene, hold TAB to speed up"?
	end
	
	UpdateSounds();
	
end

function UpdateSounds()

	if (this:GetLinearVelocity():Length() < 0.3 or (this:GetOnGround() == false and m_bOnLadder == false)) then
		//not moving fast enough to want to play a walk/climb sound, or else they are not on the ground or
		//a ladder, so we assume they are falling or something and no sound should be played
		
		m_walkLoopingSound:Play(false);	
		m_climbLoopingSound:Play(false);	
	else

	   //well, we need to play a sound.  But which one?
	   
	   if (m_bOnLadder) then
			m_walkLoopingSound:Play(false);	
			m_climbLoopingSound:Play(true);	
	   else 
			if (m_bRight or m_bLeft) then
				m_walkLoopingSound:Play(true);	
				m_climbLoopingSound:Play(false);	
			else
				m_walkLoopingSound:Play(false);	
				m_climbLoopingSound:Play(false);	
			end
	   end
	
	end

end

function ProcessLadder()

	//first, the point we should use to determine if we're standing on a ladder should be our FEET (well, bottom
	//of the collision rect), not the middle of the entity like normal.  Here's how we can get that.
	
	local vFeetPos = this:GetPos();
	vFeetPos.y = this:GetWorldCollisionRect().bottom;
	
	//LogMsg("Foot point is at " .. tostring(vFeetPos));

	if (m_bOnLadder == false) then
		
		if (m_bUp) then
			if (this:InNearbyZoneByMaterialType(vFeetPos, C_MATERIAL_TYPE_VERTICAL_LADDER)
			and this:InNearbyZoneByMaterialType(vFeetPos - Vector2(0, m_ladderSensitivity), C_MATERIAL_TYPE_VERTICAL_LADDER)) then
				EnterLadderMode();
			end
		elseif (m_bDown) then
				//is there a ladder going down?
				if (this:InNearbyZoneByMaterialType(vFeetPos, C_MATERIAL_TYPE_VERTICAL_LADDER)
				and this:InNearbyZoneByMaterialType(vFeetPos + Vector2(0, m_ladderSensitivity), C_MATERIAL_TYPE_VERTICAL_LADDER)) then
				EnterLadderMode();
				end
		end
	
	else
		//player is currently in ladder mode.  Time to end it or not?  Let's make it so they can hold diagonal without
		//leaving ladder mode, makes it 'feel' a bit better
		
		if (this:InNearbyZoneByMaterialType(vFeetPos, C_MATERIAL_TYPE_VERTICAL_LADDER) == false
		or ( (m_bLeft or m_bRight) and (m_bUp == false and m_bDown == false)) ) then
			EndLadderMode();	
		end
	
	end

end

function EnterLadderMode()

	//setup for ladder mode
	this:GetBrainManager():SendToBrainBase("control_filtery=1"); //turn Y dampening/movement back on
	this:SetGravityOverride(0);
	this:GetBrainManager():Add("ForceVisual", "force_anim=climb"); //look like we're climbing, even
																   //through we're still in idle or walk mode
	this:SetDesiredSpeed(m_climbSpeed);
	m_bOnLadder = true;
			
end


function EndLadderMode()

	if (m_bOnLadder == false) then return; end;

	this:GetBrainManager():SendToBrainBase("control_filtery=0"); //turn Y dampening/movement back off
	this:SetGravityOverride(C_GRAVITY_OVERRIDE_DISABLED); //turn gravity back on
	this:GetBrainManager():Remove("ForceVisual"); //all "ForceVisual" brains are removed
	this:SetAnimPause(false); //make sure we're not still paused
	m_bOnLadder = false;
	m_bAttack = false;
	this:SetOnGround(false); //we might not be..
	this:SetFacingTarget(m_lastDirection); //don't want to keep pointing up or down
	this:SetDesiredSpeed(m_walkSpeed);
		
end

function ProcessDoor()
	
	local z = this:GetActiveZoneByMaterialType(C_MATERIAL_TYPE_WARP);
	
	if (z.entityID != C_ENTITY_NONE) then
		
		//warp found
		local warpEnt = GetEntityByID(z.entityID);
		
		//use an offset so we end up with our FOOT on the destination spot, instead of our true center
		local vOffset = Vector2(0, -(this:GetCollisionRect():GetHeight()/2));
		warpEnt:RunFunction("OnWarp", this, vOffset);
		
	end

end

function ResetKeys()

	m_bLeft = false;
	m_bRight = false;
	m_bUp = false
	m_bDown = false;
	
	m_bAttack = false;
	m_bAction = false;
	
end

function ProcessMovementLadder(step)

	 this:SetOnGround(true);  //we're actually floating in space as far as the game is concerned, this tells it to
	 //treat us like we're on the ground

	//check for movement
	local facing = C_FACING_NONE;

	if (m_bUp) then facing = C_FACING_UP; end;
	if (m_bDown) then facing = C_FACING_DOWN; end;

	if (facing != C_FACING_NONE) then
		this:GetBrainManager():SetStateByName("Walk");
		this:SetAnimPause(false);
		//why not slightly center them on the ladder as well?  So they don't fall off it it kind of curves around.
		local vFeetPos = Vector2(this:GetPos().x, this:GetWorldCollisionRect().bottom);

		local z = this:GetNearbyZoneByMaterialType(vFeetPos, C_MATERIAL_TYPE_VERTICAL_LADDER);
		
		if (z.materialID == C_MATERIAL_VERTICAL_LADDER) then
			local ladderCenterX = z.vPos.x + (z.boundingRect:GetWidth()/2) + z.boundingRect.left;
			//LogMsg("ladder x is " .. ladderCenterX .. " rect: " ..tostring(z.boundingRect));
			this:SetPos(Vector2(Lerp(this:GetPos().x, ladderCenterX, 0.1*step), this:GetPos().y));
	
		end
		
			this:SetFacingTarget(facing); //point up or down
		
		else
			//we're sitting here on a ladder doing nothing, just pause the active animation
			this:GetBrainManager():SetStateByName("Idle");
			this:SetAnimPause(true);
	end	
end

function ProcessMovement(step)

	//check for movement
	local facing = C_FACING_NONE;
	
	if (m_bLeft) then facing = C_FACING_LEFT; end;
	if (m_bRight) then facing = C_FACING_RIGHT; end;
	
		if (this:GetOnGround() and m_jumpTimer < GetApp:GetGameTick() ) then
  			//this:GetBrainManager():Remove("ForceVisual"); //all "ForceVisual" brains are removed
  			this:SetVisualStateOverride(C_VISUAL_STATE_NONE); //same as above but faster
  		else
	 		  //this:GetBrainManager():Add("ForceVisual", "force_set=" .. C_VISUAL_STATE_JUMP );
  			this:SetVisualStateOverride(C_VISUAL_STATE_JUMP); //same as above but faster
  		end
	
	if (facing != C_FACING_NONE) then
		
		//they are pressing towards a direction
		this:SetFacingTarget(facing); //turn the way we want
			this:GetBrainManager():SetStateByName("Walk");
		m_lastDirection = facing; //store this for later, like when we get off the ladder
	else
			this:GetBrainManager():SetStateByName("Idle");
	end
	
end

function CheckForJump()

	//check for jumping
	
	if (m_bJumpKeyDown and m_jumpTimer < GetApp:GetGameTick()) then
		if (this:GetOnGround()) then
			m_jumpTimer = GetApp:GetGameTick()+m_delayBetweenJumpsMS;
  		this:StopY(); //Kill any Y interia so each jump is exactly the same height, feels more consistent
			//this:StopX();
			this:AddForce(Vector2(0, m_JumpPower));
			this:PlaySound(m_jumpSound);
 			
		end
	end
end

function SetupInputKeys(joyNum)

	//Note: the "always" keyword means the key will always react, regardless of what else is also pressed at the same time.  Without it,
	//you couldn't shoot and walk at the same time

if (joyNum == 0) then

	//only map this extra keyboard input for player 0
	
	GetInputManager:AddBinding("left,always", "OnLeft", this:GetID());
	GetInputManager:AddBinding("right,always", "OnRight", this:GetID());
	GetInputManager:AddBinding("up,always", "OnJump", this:GetID());
	//GetInputManager:AddBinding("down,always", "OnDown", this:GetID());
	GetInputManager:AddBinding("control", "OnJump", this:GetID());
	//GetInputManager:AddBinding("space,always", "OnJump", this:GetID()); 
	end
	
	
	if (joyNum == 1) then

	//only map this extra keyboard input for player 1
	
	GetInputManager:AddBinding("a,always", "OnLeft", this:GetID());
	GetInputManager:AddBinding("d,always", "OnRight", this:GetID());
	GetInputManager:AddBinding("w,always", "OnJump", this:GetID());
	end
	
if (joyNum == 2) then

	//only map this extra keyboard input for player 2
	
	GetInputManager:AddBinding("numpad4,always", "OnLeft", this:GetID());
	GetInputManager:AddBinding("numpad6,always", "OnRight", this:GetID());
	GetInputManager:AddBinding("numpad8,always", "OnJump", this:GetID());
	end

if (joyNum == 3) then

	//only map this extra keyboard input for player 3
	
	GetInputManager:AddBinding("j,always", "OnLeft", this:GetID());
	GetInputManager:AddBinding("l,always", "OnRight", this:GetID());
	GetInputManager:AddBinding("i,always", "OnJump", this:GetID());
	end

	//let's also let the joystick control things.  Instead of "any" we could use "0", "1", etc to specify certain joysticks.
	GetInputManager:AddBinding("joy_" .. joyNum .. "_left,always", "OnLeft", this:GetID());
	GetInputManager:AddBinding("joy_" .. joyNum .. "_right,always", "OnRight", this:GetID());
	GetInputManager:AddBinding("joy_" .. joyNum .. "_up,always", "OnUp", this:GetID());
	GetInputManager:AddBinding("joy_" .. joyNum .. "_down,always", "OnDown", this:GetID());
	
	GetInputManager:AddBinding("joy_" .. joyNum .. "_button_0", "OnJump", this:GetID()); //let this key attack,  note, control and shift alone don't need the "always" keyword
	GetInputManager:AddBinding("joy_" .. joyNum .. "_button_1", "OnActionButton", this:GetID()); //let this key be a generic "action" button
	
end

function OnLeft(bKeyDown)
    m_bLeft = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnRight(bKeyDown)
    m_bRight = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnUp(bKeyDown)
    m_bUp = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnJump(bKeyDown)
    m_bJumpKeyDown = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnDown(bKeyDown)
    
    m_bDown = bKeyDown;
    
    if (m_bOnLadder == false and m_bDown) then
    	m_bCheckForDoor = true;		
    end
	
	return true; //continue to process key callbacks for this key stroke
end

function OnAttack(bKeyDown)

	if (bKeyDown) then
		if (m_attackTimer < GetApp:GetGameTick()) then
			m_bAttack = true;
			m_attackTimer = GetApp:GetGameTick() + m_delayBetweenAttacksMS;
		end
	end
	
	return true; 	
end


function OnDialogCloseNotify(dialog) //called on active player if it exists (dialog.lua)

	//after a dialog is over, 
	
	this:Stop(); //so we don't keep skidding around
	
	//so they don't accidently swing/hit again right after the dialog because they had hit the keys

	ResetKeys()
	
end

function OnDialogOpenNotify(dialog) //called on active player if it exists (dialog.lua)

end

function ProcessAttack()

	if (this:Data():Exists("HasSlasher")) then
	
	//make the sword slash
	
	local ent = CreateEntity(this:GetMap(), this:GetPos(), "weapons/slash/slash.lua");
	ent:RunFunction("SetParent", this);
	
	else
		//GetTextManager:Add("I don't have a weapon yet!", this);
	end
	
	m_bAttack = false;
end

function OnAttackLoop() //this is called when the Attack state animation reaches the end/looping point
	//attack animation is done, switch to another state
	this:GetBrainManager():SetStateByName("Idle");
end

function OnActionButton(bKeyDown)

	if (bKeyDown) then
		m_bAction = true;
	end
	
	return true;
end

function ProcessAction()

	m_bAction = false;

	//first, check tiles we're touching
	
	local layerList = this:GetMap():GetLayerManager():GetVisibleLayers(); //what layers we'll scan
	local tileList = this:GetMap():GetTilesByRect( Rect(this:GetWorldCollisionRect()), layerList, false); //grab them

	//LogMsg("Found " .. tileList:GetCount() .. " tiles after scanning the " .. layerList:GetCount() .. " layers.");

	local tile; 
	
	while true do
	
		tile = tileList:GetNext();
		if (tile == nil) then break; end;
		//LogMsg("Found tile type " .. tile:GetType());
		
		if (tile:GetType() == C_TILE_TYPE_ENTITY and tile:GetAsEntity():GetID() != this:GetID()) then
			//well, does this entity have a script, and if so, does it have a OnAction() function to handle
			//us looking at them?
			if (tile:GetAsEntity():FunctionExists("OnAction")) then
				//yay, it does
				//it looks like it's prepared to handle our request that it "do something"
				tile:GetAsEntity():RunFunction("OnAction", this);
				return true;
			end
			
		end
	end
	

	//let's try a totally different way, shooting rays out in front of us. "examine" things we're looking
	//at, as the above method only checks what we're standing on.
	
	local rayRange = 80;
	local raySpread = 8; //causes 8 rays to be shot in a spread formation, easier to detect hits.  0 to use 1 ray only
	local entToIgnore = this;
	local hitZone = this:GetMap():GetCollisionByRay(this:GetPos(), this:GetVectorFacing(), rayRange, raySpread, entToIgnore, C_RAY_ENTITIES, false);
		
	if (hitZone.entityID != C_ENTITY_NONE) then
		//we have an entity in front of us!
		local ent = GetEntityByID(hitZone.entityID);
		
		if (ent:FunctionExists("OnAction")) then
			//it looks like it's prepared to handle our request that it "do something"
			ent:RunFunction("OnAction", this);
			return true;
		end
	end
		
	//they hit the action button and no entity was close
	 //GetTextManager:Add("Hmm?", this);
end


function UpdateGUI()
	
	//tell the interface GUI bar to update everything ASAP
	GetEntityByID(g_interfaceID):RunFunction("RequestUpdate");
end

function Startup()

	if (GameIsActive()) then

		LogMsg("Initializing the player entity.");
		UpdateGUI();

		//this is a real game, not the editor
		if (this:Data():Exists("firsttime") == false) then

			this:Data():SetNum("firsttime", 1);
			
			Schedule(1000, C_ENTITY_NONE,
				'GetTextManager:AddCustomScreen("Use the arrow keys to move, jump, and climb.", Vector2(C_SCREEN_X/2,C_SCREEN_Y-40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);');
			Schedule(6000, C_ENTITY_NONE,
				'GetTextManager:AddCustomScreen("Press SPACE while standing over a sign to examine it.", Vector2(C_SCREEN_X/2,C_SCREEN_Y-40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);');
			Schedule(11000, C_ENTITY_NONE,
				'GetTextManager:AddCustomScreen("F1 to toggle edit mode", Vector2(C_SCREEN_X/2,C_SCREEN_Y-40), Vector2(0, 0), Color(200,200,0,255), 3500, C_FONT_DEFAULT);');
		end
	end

end	


function OnKill()
	
	//make sure the sounds we used are off
	m_walkLoopingSound:Play(false);	
	m_climbLoopingSound:Play(false);	
	
	//RemoveActivePlayerIfNeeded(this); //let the engine know we shouldn't be the active player any more

end

function OnHit(damage)
	OnDamage(nil, 0, nil, damage, 0, nil);
end

//if projectile is nil, the thing that caused the damage was the enemy itself

function OnDamage(normal, depth, enemy, damage, userVar, projectile)
	 
	 if (m_invincibilityTimer != 0) then
		 	if (m_invincibilityTimer > GetApp:GetGameTick()) then
			 	//we can't be hurt yet
			 	return;
		 	else
		 		m_invincibilityTimer = 0; //not invincible anymore
		 	end
	 	end
	 
		 //GetTextManager:AddCustom(tostring(damage), this, this:GetPos(), Vector2(0, -0.3), Color(200,0,0,255), 2000, C_FONT_DEFAULT);
  
  		 //GetTextManager:Add("Ouch!", this);
  		 GetSoundManager:Play("audio/die.ogg");

		 this:GetBrainManager():Add("ColorFlash", "pulse_rate=200;g=-200;b=-200;remove_brain_by_pulses=1");

		//make them fall off the ladder if they were on it.
		 EndLadderMode();
 
		 this:GetBrainManager():SetStateByName("Pain");
		
		//take over the brains of the player for a bit so he can't move for a bit
		local goal = this:GetGoalManager():PushNewGoal("DamageDelay");
		goal:PushDelay(500);
		
		//visual effect of color flashing
		Schedule(500, this:GetID(), 'this:GetBrainManager():Add("ColorFlash", "pulse_rate=50;a=-150;remove_brain_by_pulses=20");');
		m_invincibilityTimer = GetApp:GetGameTick()+2200;
		this:SetVisualStateOverride(C_VISUAL_STATE_DIE); //same as above but faster
	
end

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

//LogMsg("Player touching material", materialID);
if (m_bOnLadder or GetMaterialManager:GetMaterial(materialID):GetType() != C_MATERIAL_TYPE_NORMAL) then 
		return; //only bump our heads on hard things, not ladder zones and such
	end

	//LogMsg("Hit Static: Depth: " .. tostring(depth) .. " normal: " .. tostring(vNormal));

end
