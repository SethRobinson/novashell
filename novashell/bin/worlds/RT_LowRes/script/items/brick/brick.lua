
	m_bCracked = false;
	m_bMoving = false;
	
function OnInit() //run upon initialization
	this:SetDensity(0);

	this:SetVisualProfile("~/brick.xml", "brick");
	this:LoadCollisionInfo("~/brick.col");
	this:SetAnimByName("normal");
	this:SetAnimPause(true); 
	if ( toboolean(this:Data():SetIfNull("cracked", "false"))) then
			//we're cracked
			m_bCracked = true;
			this:SetAnimFrame(1); //if we wanted to start already cracked
	end
	
end

function OnPostInit() //run during the entity's first logic update
	this:SetEnableRotationPhysics(false);
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true); //all players
end

function OnKill() //run when removed

end

function Crumble(bBounce)
	//Bust up this brick
	if (m_bMoving) then
		return false;
	end
	
	if (this:GetAnimName() == "crumble") then
		return; //it's already dying..
	end
	
		//allow us to respond to collisions with turtles/etc for a short time

		if (bBounce) then
				this:SetCollisionListenCategory(C_CATEGORY_ENEMY, true); //OnCollision() will now respond to enemies
				m_bMoving = true;
				
				//move up a bit
				Schedule(0, this:GetID(), "this:SetPos(this:GetPos()+Vector2(0,-3))");
				Schedule(30, this:GetID(), "this:SetPos(this:GetPos()+Vector2(0,-3))");
				Schedule(60, this:GetID(), "this:SetPos(this:GetPos()+Vector2(0,-3))");
				Schedule(190, this:GetID(), "m_bMoving = false; this:SetCollisionListenCategory(C_CATEGORY_ENEMY, false);");
		
		end
	 
		if (m_bCracked == false) then
		 		
		 		//if we want to be able to crack it...
		 		
		 		//this:SetAnimFrame(1);
		 		//m_bCracked = true;
		 		
	 		//move back down?
		 		if (bBounce) then
			 		Schedule(120, this:GetID(), "this:SetPos(this:GetPos()+Vector2(0,3))");
			 		Schedule(150, this:GetID(), "this:SetPos(this:GetPos()+Vector2(0,3))");
			 		Schedule(180, this:GetID(), "this:SetPos(this:GetPos()+Vector2(0,3))");
				end
			
			return;
		end

	//if we got here, time to destroy the brick	
		
		this:SetAnimLoopCallback(true);
		this:SetAnimFrame(0);
		this:SetAnimByName("crumble");
	  this:SetAnimPause(false); 
	  this:SetPersistent(false); //make sure we don't save if they quit now
	  GetSoundManager:Play("audio/crumble.ogg");
end

function OnAnimLoop()
	//LogMsg("Anim looped");
	this:SetDeleteFlag(true);
end

function OnSave()

//save if we're cracked or not
	this:Data():Set("cracked", tostring(m_bCracked));
	
end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	//LogMsg("Ent " .. tostring(this:GetID()) .. " hit by #" .. entity:GetID() .. "  Hit Normal: Depth: " .. tostring(depth) .. " normal: " .. tostring(normal));

	if (entity:GetType() == C_TYPE_ENEMY) then
		//a turtle or something is walking on us, but if we're hearing this collision it means
		//the player just hit under us, so we can hurt them
	
			//LogMsg("Hit Static: Depth: " .. tostring(depth) .. " normal: " .. tostring(normal));
	
			if (vNormal.y > 0.1) then //make sure they are on top of us, not under
				//LogMsg("Bumping..");
				entity:RunFunction("OnBumped");
	 		end
			
			return;
	end
	
	if (m_bMoving == true) then
		return;
	end;
	
	//if hit from below...
	if (vNormal.y < -0.1 and depth > 0) then
	 	GetSoundManager:Play("audio/bump.ogg");
		
		Crumble(true); //true means the brick will bounce up and down, hurting people
  	return;
  end

	 if (m_bCracked == false) then
	 	//it's not cracked yet..
	 	return;
	 end

	//from above.. walking on us should crumble us.
	if (vNormal.y > 0.1 and depth > 0) then
		Crumble(false); //don't want the brick to move, so we send false
  return;
  end

end


