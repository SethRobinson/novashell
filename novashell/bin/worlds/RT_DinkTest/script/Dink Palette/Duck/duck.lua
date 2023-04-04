
m_headFlySpeed = 10;
m_walkSpeed = 3;
function OnInit() //run upon initialization

	//control how it looks
	this:SetVisualProfile("~/duck.xml", "duck");
	//collision will be loaded/stored here, it's ok if it doesn't exist yet
	this:LoadCollisionInfo("~/duck.col");
    this:SetDesiredSpeed(m_walkSpeed);

	if (this:Data():Exists("Headless")) then
		//we've been killed
		SetupHeadless();
	end 

	m_followID = C_ENTITY_NONE;
	
	this:SetSpriteByVisualStateAndFacing(); //don't need this, but makes them face the right way before any logic is run

end

function OnPostInit() //run during the entity's first logic update

	this:GetBrainManager():Add("StandardBase", "");
	WonderAround();

end

function OnKill() //run when removed

end

function SetupHeadless()

	//switch graphics set to the headless one
	this:SetVisualProfile("~/duck_headless.xml", "duck_headless");
	//this:SetMaxMovementSpeed(1.7);
	
end

//entParent is whoever sent the damage, entObject is the actual damaging object/projectile if not nil
function OnDamage(normal, depth, entParent, strength, extra, entObject)
	this:PlaySoundPositioned("~/audio/squish.ogg");


	if (this:Data():Exists("Headless")) then
	
		//hey, we're already dead.  Let's float off into nothing.  I don't really have an
		//explosion sprite sequence handy or anything.
		this:GetBrainManager():Add('FadeOutAndDelete','');
		return;
	end
	
	
	this:Data():Set("Headless", "");
	SetupHeadless();
	
	//let's make the head fly off, it's fun. By adding the this: in front we allow the ~/ (start
	// path from script's dir) relative path to work
	local ent = this:CreateEntity(this:GetMap(), this:GetPos(), "~/duck_head.lua");
	ent:SetLayerID(this:GetLayerID());
	ent:SetVectorFacing(this:GetVectorFacing());
	
	//let's make the head fly in the right direction
	
	ent:SetScale(this:GetScale()); //if we were scaled, make the head match it
	local vForce = entParent:GetVectorFacing() * m_headFlySpeed;
	LogMsg("Adding ", vForce);
	ent:AddForce(vForce);
	ent:RunFunction("DoFlyAnim");
	
	m_followID = C_ENTITY_NONE; //stop following the player
	WonderAround();

end

function PlayRandomSound()

	local soundID = this:PlaySoundPositioned("~/audio/quack.ogg");
	
	//randomize the pitch
	GetSoundManager:SetSpeedFactor(soundID, 0.90 + (math.random() * 0.2) );

end

function WonderAround()

	//let's walk to a place close by
	
	this:GetGoalManager():RemoveAllSubgoals();
	

	if (m_followID == C_ENTITY_NONE) then
	
		local distance = 100 + math.random(150); //how far this location is from us
		//get a random unit direction vector out of the 8 main ones, we could also use sin/cos if we wanted
		vecRandomDir = FacingToVector(math.random(C_FACING_COUNT)-1);
		local newPos = this:GetPos() + vecRandomDir*distance;
			//first let's check to see if this is a valid position we can exist at
		//if (this:IsValidPosition(nil, newPos, false)) then  //world, pos, ignore living creatures
		if (this:CanWalkTo(newPos, false)) then //pos, ignore living creatures
		
				//if it's valid, go there.  Otherwise just idle a bit for no reason.	
				this:GetGoalManager():AddMoveToPosition(newPos);
					
		end
	else
		
	 //follow
	 
	 this:GetGoalManager():AddApproach(m_followID, C_DISTANCE_TALK);	
		
	end
	
	
	if (this:Data():Exists("Headless") == false) then
	
		//how about have them quack too
		if (math.random(5) == 1) then
			this:GetGoalManager():AddRunScriptString("PlayRandomSound();");
		end

		//we're not dead, so we'll pause and idle, less headless screaming
		this:GetGoalManager():AddDelay(700 + math.random(1300));
		
		end
	


	
	//after that, tell it to run this again to schedule more stuff
	
	this:GetGoalManager():AddRunScriptString("WonderAround();");

end


function OnResponse(text, key, dialog)  //dialog is passed as a SpecialEntity

	
	if (this:Data():Exists("Headless")) then
		
			//well, the duck is dead
		local msg = "There is no response from the place where the head used to be.";
		
		entChoice = CreateEntitySpecial("ChoiceDialog", "msg=" .. msg .. ";callback=OnResponse;");
		entChoice:Send("add_choice|Continue|");
		entChoice:Send("activate");	
		return;
	end
	
	
	if (key == "follow") then
	        m_followID =  g_playerID;
				//let this entity function offscreen for a while
				GetWatchManager:Add(this, 3000000); //always function offscreen
					   //this:SetMaxMovementSpeed(3.0);
					   	WonderAround();
						this:GetGoalManager():PushSay("Ok, you lead.", g_playerID);


    end

	if (key == "stop") then
	        m_followID =  C_ENTITY_NONE;
			this:GetGoalManager():PushSay("Fine.", g_playerID);
				GetWatchManager:Remove(this); //function for up to 30 seconds offscreen
	end

end

//dink pressed space while looking at us I guess

function OnAction(Ent)
	
		//note, the [[ and ]] is a lua way to make defining long strings easier.  You don't have to use it, msg = "crap"; works too.
	local msg;

	if (this:Data():Exists("Headless")) then
		//you know what?  He's dead.
		msg = "The duck's gaping maw stares at you intently.";
	else
		msg = "The duck stares at you intently.";
	end
	
entChoice = CreateEntitySpecial("ChoiceDialog", "msg=" .. msg .. ";callback=OnResponse;title=Duck;entity_id=" .. this:GetID());
	
	if (m_followID == C_ENTITY_NONE) then
		entChoice:Send("add_choice|Follow me, duck!|follow");
	else
		entChoice:Send("add_choice|Stop following me!|stop");
	end
	entChoice:Send("activate");

	
end

