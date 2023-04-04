//Our player controlled character.
RunScript("system/player_utils.lua"); //has some useful global functions, this is in base
RunScript("gui/interface_utils.lua"); //from base

function OnInit() //run upon initialization

	//control how it looks
	this:SetVisualProfile("~/dink.xml", "dink");
	
	//collision will be loaded/stored here, it's ok if it doesn't exist yet
	this:LoadCollisionInfo("~/dink.col");

end

function OnPostInit() //run during the entity's first logic update

//player has actually been placed on the map, let's get serious

	this:SetName("Player"); //note, if  there is already a Player this can fail and you'll end up with PlayerA or something.
	//It's important to call the main player "Player", because when you load a saved game, it looks for "Player"
	//and moves the camera there.  This functionality could easily be overidden though, it's just how into_menu.lua does it.
	
	SetDefaults();
	SetupInputKeys();

	//let's have a function that gets run every logic tick
	this:SetRunUpdateEveryFrame(true);
	
end

function OnKill() //run when removed

	RemoveActivePlayerIfNeeded(this); //let the engine know we shouldn't be the active player any more
	
end


//this is run every logic tick
function Update(step)
	
	AssignPlayerToCameraIfNeeded(this);  //make us the official player and have the camera track us, if we're not already,
	//and if another entity doesn't already have this role
	
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
	//this:SetMaxMovementSpeed(3.2);
	//this:SetDesiredSpeed(3.0);
	this:SetTurnSpeed(0.4);

	m_invincibilityTimer = 0; //means not invincible

	//we keep track of the players keys

	m_bLeft = false;
	m_bRight = false;
	m_bUp = false
	m_bDown = false;
	
	m_bAttack = false;
	m_bAction = false;

end;


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

	m_bAction = false;
	m_bAttack = false; 
	
end

function OnDialogOpenNotify(dialog) //called on active player if it exists (dialog.lua)

end


function ProcessAttack()

	if (this:GetBrainManager():InState("Attack") == false) then
	//make the sword slash
		
	this:GetBrainManager():SetStateByName("Attack"); //plays the assign attack animation if available

	GetSoundManager:Play("audio/effects/swing.ogg");
		//setup where the actual damage will happen
	    Schedule(100, this:GetID(), 'ApplyDamage();');
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

function ApplyDamage()

	//actually hurt the things by us
	
	ent = CreateEntity(this:GetMap(), this:GetPos(), "weapons/top_damage/top_damage.lua");
	
	//the damage script supports some basic properties to make it fit dink's punch better
	ent:RunFunction("SetDistanceFromBody", 20);
	ent:RunFunction("SetWeaponScaleMod", 0.6);
	ent:RunFunction("SetParent", this);
end
