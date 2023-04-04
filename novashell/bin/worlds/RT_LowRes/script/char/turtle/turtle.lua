m_lastTurnTime = 0;
m_minTimeBetweenTurnsMS = 500;

C_MIN_TIME_BETWEEN_BUMPS_MS = 200;

STATE_NORMAL = 0;
STATE_ON_BACK = 1;
STATE_DEAD = 2;
m_bumpTimer = 0;

m_state = STATE_NORMAL;

function OnInit() //run upon initialization
	this:SetVisualProfile("~/turtle.xml", "turtle");
	this:LoadCollisionInfo("~/turtle.col");
	this:SetType(C_TYPE_ENEMY);
	this:SetCategory(C_CATEGORY_ENEMY, true);
	this:SetCollisionListenCategory(C_CATEGORY_TILEPIC, true);
	this:SetCollisionListenCategory(C_CATEGORY_ENTITY, true);
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
	
end

function OnMapInsert() //run as the entity is placed on an actual map, but before OnPostInit()

	if (string.findnocase(this:GetMap():GetName(), "palette")) then
	//the map its on has "Palette" in its name, let's assume we don't really want
	//to activate its AI
	else
		GetWatchManager:Add(this, C_TIME_FOREVER); //always function offscreen
	end

end

function OnPostInit() //run during the entity's first logic update

	this:SetDesiredSpeed(2);
	this:SetTurnSpeed(6); //turn instantly
	this:GetBrainManager():Add("StandardBase","");
	this:GetBrainManager():SendToBrainBase("control_filtery=0");
	//this:GetBrainManager():SendToBrainBase("air_control_filterx=0.0;air_control_filtery=0");
	
	this:GetBrainManager():SetStateByName("Walk");
	this:SetFacing(C_FACING_LEFT);
end

function OnKill() //run when removed

end

function TurnAround()

	if (m_lastTurnTime > GetApp:GetGameTick() or this:GetOnGround() == false) then
		//not yet
		return;
	end
	
	m_lastTurnTime = GetApp:GetGameTick() + m_minTimeBetweenTurnsMS;
	
	if (this:GetFacing() == C_FACING_LEFT) then
		this:SetFacing(C_FACING_RIGHT);
	else
		this:SetFacing(C_FACING_LEFT);
	end
end

function Kicked(vDir)

	//player kicks us off the level
	this:GetBrainManager():SendToBrainBase("control_filterx=0");
	GetSoundManager:Play("audio/kick.ogg");
	m_state = STATE_DEAD;
	
	//no longer collide with anything
	this:SetCollisionCategories(C_CATEGORIES_NONE);
	
	//no longer receive messages about any collisions
	this:SetCollisionListenCategories(C_CATEGORIES_NONE);

	this:SetEnableRotationPhysics(true);
	local speed = 2;
	this:AddForce(vDir*speed);
	this:SetAnimPause(true); //stop at this frame
	//disappear in a bit
 	Schedule(5000, this:GetID(), "this:SetDeleteFlag(true);");
 	Schedule(100, this:GetID(), "RotateDeath();");

end

function RotateDeath()
	this:SetRotation(this:GetRotation()+1);
	Schedule(100, this:GetID(), "RotateDeath();");

end


//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

if (entity == nil) then
	//collided with a tilepic, not an entity!
		if (vNormal.x > 0.7 or vNormal.x < -0.7) then
			TurnAround();
		end
	
	return;
end

//an entity..

	if (entity:GetType() == C_TYPE_CHARACTER) then
			
				if (m_state == STATE_DEAD) then
			return true; //we're dead, but register collision against characters, it looks funny		
	 	end
	
			if (m_state == STATE_NORMAL) then
				entity:RunFunction("OnHit", 1);
			else
				//I guess the player kicks us to death, since we're on our back
				Kicked(vNormal); //send it the direction we got kicked
			end
		end

	if (m_state == STATE_ON_BACK) then
		return;
	end
		
	//LogMsg("Hit Entity: Depth: " .. tostring(depth) .. " normal: " .. tostring(normal));
		if (vNormal.x > 0.7 or vNormal.x < -0.7) then
			TurnAround();
		end		
	end
	

function OnBumped()
		
		if (m_state == STATE_DEAD) then
			LogMsg("Huh?");
		end
			
			if (m_bumpTimer > GetApp:GetGameTick()) then
				//they just got bumped recently, don't let it happen again
				return;
			end
			
			m_bumpTimer = GetApp:GetGameTick() + C_MIN_TIME_BETWEEN_BUMPS_MS;
		
			if (m_state == STATE_NORMAL) then
				m_state = STATE_ON_BACK;
				this:GetBrainManager():SetStateByName("Die");
				this:GetBrainManager():SendToBrainBase("control_filterx=0");
	
				return;
			end
			
			if (m_state == STATE_ON_BACK) then
				m_state = STATE_NORMAL;
				this:GetBrainManager():SetStateByName("Walk");
				this:GetBrainManager():SendToBrainBase("control_filterx=1");

				return;
			end
			
end

function OnDieLoop()
	//if the die anim is done, let them be alive again..!
	OnBumped();
	GetSoundManager:Play("audio/turtle_out.ogg");

end
