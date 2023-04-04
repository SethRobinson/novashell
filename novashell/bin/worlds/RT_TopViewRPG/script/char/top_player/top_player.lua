//Our player controlled character.
RunScript("system/player_utils.lua"); //has some useful global functions, this is in base
	
function OnInit()
	
	//setup what he looks like, so the editor knows how big its rect is and such.
	//it hasn't been placed on map yet, just initialized
	
	this:SetVisualProfile("~/../man/man.xml", "man");
	//this:Data():SetIfNull("walk_sound", "audio/effects/walk/walk_wood_loop.ogg");
	this:LoadCollisionInfo("~/../man/man.col");
	
end

function OnPostInit()

	//player has actually been placed on the map, let's get serious
	LogMsg("Initting player");

	this:SetCategory(C_CATEGORY_ENTITY, false);
	this:SetCategory(C_CATEGORY_PLAYER, true);
	this:SetName("Player"); //note, if  there is already a Player this can fail and you'll end up with PlayerA or something.
	//It's important to call the main player "Player", because when you load a saved game, it looks for "Player"
	//and moves the camera there.  This functionality could easily be overidden though, it's just how into_menu.lua does it.
	
	SetDefaults();
	SetupInputKeys();
	//let's have a function that gets run every logic tick
	this:SetRunUpdateEveryFrame(true);

	UpdateGUI();
end

function GetInventory()
	return m_inventory;
end

function OnSave()
	LogMsg("Saving..");
		//save to our permanent storage
	this:Data():SetNum("hp", m_hp);
	this:Data():SetNum("max_hp", m_max_hp);

	serialize_to_db(m_inventory, "inventory", this:Data());
end

function UpdateGUI()
	
	//tell the interface GUI bar to update everything ASAP
	if (g_interfaceID != nil) then
		GetEntityByID(g_interfaceID):RunFunction("RequestUpdate");
	end
  
end

function OnKill()
	//LogMsg("Killing player.. ");
	RemoveActivePlayerIfNeeded(this); //let the engine know we shouldn't be the active player any more
end


//this is run every logic tick
function Update(step)
	
	AssignPlayerToCameraIfNeeded(this);  //make us the official player and have the camera track us, if we're not already,
	//and if another entity doesn't already have this role
	
	if (this:GetBrainManager():InState("Die")) then
			//we're currently dead or dying
			return;
		end
	
	
	
	//if our goal manager is doing something, we'd better not allow ourselves to be humanly
	//controlled
	
	if (this:GetGoalManager():GetGoalCount() == 0) then
		
		if (this:GetBrainManager():InState("Attack") == false ) then
			ProcessMovement();
		end
		
		if (m_bAction) then	ProcessAction(); end; //they hit the action key
		if (m_bAttack) then	ProcessAttack(); end; //they hit the attack key
	
	else
		//we're in a cut scene.  Maybe we should letter box or show a "In cutscene, hold TAB to speed up"?
	end
	
end


function SetDefaults()

	this:GetBrainManager():Add("StandardBase","");
	this:SetDefaultTalkColor(Color(255,255,150,255));
	this:SetMaxMovementSpeed(10.0);
	this:SetDesiredSpeed(8.0);
	this:SetTurnSpeed(0.4);

	m_invincibilityTimer = 0; //means not invincible

	//persistent data we want saved with the entity, there is no limit to how much we can save.  Lookups
	//are done using a binary partitioned tree, so despite being text they are very fast.
	m_inventory = unserialize_from_db("inventory", this:Data()); //will return a blank table if nothing exists

	if (this:Data():Get("max_hp") == "") then
			//one time initialization of variables here, give him items to start with if we want...
		//GiveItem("red_potion");
		//Dump(m_inventory);
	end
	
	this:Data():SetNumIfNull("Money", 0);
	m_max_hp = this:Data():SetNumIfNull("max_hp", 20); //start with this many hitpoints
	m_hp = this:Data():SetNumIfNull("hp", this:Data():GetNum("max_hp"));

	LogMsg("Player has " .. #m_inventory .. " items.");

	//don't change the above variable names or else interface.lua won't be able to steal the data it wants

	//we keep track of the players keys
	ResetKeys();

end;

function GiveItem(name)
	AddItem(name, m_inventory);
end

function ResetKeys()

	m_bLeft = false;
	m_bRight = false;
	m_bUp = false
	m_bDown = false;
	
	m_bAttack = false;
	m_bAction = false;
	
end

function ProcessMovement()

	local facing = ConvertKeysToFacing(m_bLeft, m_bRight, m_bUp, m_bDown); //this function is in player_utils.lua
	
	if (facing != C_FACING_NONE) then
		//they are pressing towards a direction
		this:SetFacingTarget(facing); //turn the way we want
		this:GetBrainManager():SetStateByName("Walk");
	else
		this:GetBrainManager():SetStateByName("Idle");
	end
	
end

function SetupInputKeys()

	//GetInputManager:AddBinding("mouse_left", "OnMouseLeft", this:GetID());

	//Note: the "always" keyword means the key will always react, regardless of what else is also pressed.  Without it,
	//you couldn't shoot and walk at the same time

	GetInputManager:AddBinding("left,always", "OnLeft", this:GetID());
	GetInputManager:AddBinding("right,always", "OnRight", this:GetID());
	GetInputManager:AddBinding("up,always", "OnUp", this:GetID());
	GetInputManager:AddBinding("down,always", "OnDown", this:GetID());

	GetInputManager:AddBinding("control", "OnAttack", this:GetID()); //let this key attack,  note, control and shift alone don't need the "always" keyword
	GetInputManager:AddBinding("space,always", "OnAction", this:GetID()); //let this key be a generic "action" button


//let's also let the joystick control things.  Instead of "any" we could use "0", "1", etc to specify certain joysticks.
	GetInputManager:AddBinding("joy_any_left,always", "OnLeft", this:GetID());
	GetInputManager:AddBinding("joy_any_right,always", "OnRight", this:GetID());
	GetInputManager:AddBinding("joy_any_up,always", "OnUp", this:GetID());
	GetInputManager:AddBinding("joy_any_down,always", "OnDown", this:GetID());
	
	GetInputManager:AddBinding("joy_any_button_0", "OnAttack", this:GetID()); //let this key attack,  note, control and shift alone don't need the "always" keyword
	GetInputManager:AddBinding("joy_any_button_1", "OnAction", this:GetID()); //let this key be a generic "action" button
	


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

function OnDown(bKeyDown)
    m_bDown = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnAttack(bKeyDown)

	if (bKeyDown) then
			m_bAttack = true;
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

	if (this:GetBrainManager():InState("Attack") == false) then
	//make the sword slash
	this:GetBrainManager():SetStateByName("Attack"); //plays the assign attack animation if available
	//this:SetAnimFrame(0); //make sure the anim restarts
	GetSoundManager:Play("audio/effects/swing.ogg");
		
	//setup where the actual damage will happen
    Schedule(200, this:GetID(), 'ent = CreateEntity(this:GetMap(), this:GetPos(), "weapons/fists/fists.lua"); ent:RunFunction("SetParent", this);');

	//OnHurt(11);

	end

	m_bAttack = false;
end

function OnAttackLoop() //this is called when the Attack state animation reaches the end/looping point
	//attack animation is done, switch to another state
	this:GetBrainManager():SetStateByName("Idle");
end

function OnAction(bKeyDown)

	if (bKeyDown) then
		m_bAction = true;
	end
	
	return true;
end

function ProcessAction()

	m_bAction = false;
	
	local rayRange = 80;
	local raySpread = 8; //causes 5 rays to be shot in a spread formation, easier to detect hits.  0 to use 1 ray only
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
	 GetTextManager:Add("Hmm?", this);

end

function GetStrength()
	//do something real with this later, the GUI uses this
	return 1;
end


function IsDead()
	return m_hp <= 0;
end

function GetHP()
	return m_hp;
end

function GetMaxHP()
	return m_max_hp;
end

function OnHeal(healAmount)

		//draw the floating # in green
	GetTextManager:AddCustom(tostring(healAmount), this, this:GetPos(), Vector2(0, -0.3), Color(0,200,0,255), 2000, C_FONT_DEFAULT);

	m_hp = m_hp + healAmount;
	m_hp = math.min(m_hp, m_max_hp); //don't let us go over our max HP 
	
  UpdateGUI();

end

function Respawn()

	this:SetPosAndMapByTagName("Hol_Inn_PlayerRoom_Low");
//Heal the player
	m_hp = m_max_hp;
  UpdateGUI();
	
	this:GetBrainManager():SetStateByName("Idle");
	this:SetFacing(C_FACING_UP); //look up

//do the cutscene stuff

	local entDad = GetEntityByName("Father");
	local entMaid = GetEntityByName("InnMaid");
	
	if (entDad == nil or entMaid == nil) then
		//guess we won't do this then.
		LogError("Couldn't find NPC's we wanted for the cutscene.");
		return;
	end
	//move the Maid and dad next to our bed before the cutscene starts

	entDad:SetPosAndMapByTagName("Hol_Inn_PlayerRoom");
	entMaid:SetPosAndMapByTagName("Hol_Inn_PlayerRoom_Right");

	//dialog stuff

	//go back to the desk after chat is over	  
  entDad:GetGoalManager():GetActiveGoal():PushApproach(GetEntityByName("InnFatherHome"):GetID(), C_DISTANCE_CLOSE);
	entDad:GetGoalManager():PushNewGoal("Freeze"):AddDelay(C_TIME_FOREVER); //so he won't move/think
 	entMaid:GetGoalManager():PushNewGoal("Freeze"):AddDelay(C_TIME_FOREVER); //so he won't move/think

 local goal = this:GetGoalManager():PushNewGoal("Cutscene");
	goal:AddDelay(500);
	goal:AddSayByID("Look, he's waking up.", entMaid:GetID(), this:GetID());
	goal:AddSayByID("Yer damn lucky to be alive, kid.", entDad:GetID(), this:GetID());
	goal:AddSayByID("Enough lollygaggin', this Inn won't run itself.", entDad:GetID(), entMaid:GetID());
  
  //release the NPC's back to whatever
   goal:AddRunScriptString([[GetEntityByName("Father"):GetGoalManager():GetGoalByName("Freeze"):Kill();]]);
   goal:AddRunScriptString([[GetEntityByName("InnMaid"):GetGoalManager():GetGoalByName("Freeze"):Kill();]]);
	
end

function OnDieLoop()

	//they just died.  Let's move and revive them.
	  local goal = this:GetGoalManager():PushNewGoal("Respawn");
	  goal:AddSay("<passes out...>", C_FACING_NONE);
		goal:AddRunScriptString("Respawn()");
		goal:AddDelay(2000);

end


//simpler way to cause damage
function OnHurt(damage)
	OnDamage(nil, nil, nil, damage, nil, nil);
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
	 	 
	 	 if (m_hp > 0) then
		 	 
			 GetTextManager:AddCustom(tostring(damage), this, this:GetPos(), Vector2(0, -0.3), Color(200,0,0,255), 2000, C_FONT_DEFAULT);
	
			m_hp = m_hp - damage;
			
			if (m_hp <= 0) then
				//uh oh, we died
				 //play death anim
				 this:GetBrainManager():SetStateByName("Die");
				 //when it's done playing, it will call OnDieLoop, then we'll handle the respawn
			else
				 GetTextManager:Add("Ouch!", this);
	 		end
			
			 UpdateGUI();
			 
	  	GetSoundManager:Play("audio/player/pain" .. math.random(3) .. ".ogg");
			 this:GetBrainManager():Add("ColorFlash", "pulse_rate=200;g=-200;b=-200;remove_brain_by_pulses=1");
			
			//schedule when the player can move again
			Schedule(200, this:GetID(), 'this:GetBrainManager():Add("ColorFlash", "pulse_rate=50;a=-150;remove_brain_by_pulses=20");');
			m_invincibilityTimer = GetApp:GetGameTick()+1300;
		else
			//we're already dead, why is the AI attacking a dead body?
		end
end
