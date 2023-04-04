function OnInit()

	this:SetVisualProfile("~/spider.xml", "spider");
	this:LoadCollisionInfo("~/ent_spider.col");
end

function SetDefaults()
	this:GetBrainManager():Add("StandardBase","");
	//limit movement to horizontal only
	this:GetBrainManager():SendToBrainBase("control_filtery=0");

	this:GetBrainManager():SetStateByName("Walk");
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
	this:SetVisibilityNotifications(true); //OnGetVisible and OnLoseVisible will get called now
	this:SetDesiredSpeed(3);
	this:SetTurnSpeed(10); //instant
	
	m_hp = 15;
		
	if this:GetScale().x < 1 then
		//he's small I guess, give him less HP
		m_hp = 5;
	 end


end

function OnPostInit() //only run after we're actually pasted in the world
	
	SetDefaults();

	//the random is so not everybody things at the exact same time, we'd rather spread out the thinking
	//to avoid jerkiness on slow machines
	
	Schedule(500+random(200), this:GetID(), "Think()");
end

function Think()

	if (m_hp == 0) then
		//we're dead, stop thinking!
		return;
	end
	
	
//see what's in front of us

	local rayRange = 70 * this:GetScale().x; //if the monster is big, we need to shoot a longer ray
	local raySpread = 5; //causes 5 rays to be shot in a spread formation, easier to detect hits.  0 to use 1 ray only
	local entToIgnore = this;
	
	//change C_RAY_EVERYTHING to C_RAY_DEBUG and it will draw the rays so you can see what they really touch
	local hitZone = this:GetMap():GetCollisionByRay(this:GetPos(), this:GetVectorFacing(), rayRange, raySpread, entToIgnore, C_RAY_EVERYTHING, false);
	
		if (hitZone.materialID != C_MATERIAL_NONE) then
			
			if (GetMaterialManager:GetMaterial(hitZone.materialID):GetType() == C_MATERIAL_TYPE_NORMAL) then
				//well, we certain have SOMETHING with collision sitting in front of us.  Turn around?
				//The actual thing we hit is entity z.entityID btw, if we needed it. The collision point is at z.vPos.
	
				//only turn around if it isn't the player.  If it's the player, we'll keep walking to hurt him
				if (hitZone.entityID != g_playerID) then
					this:SetVectorFacing(this:GetVectorFacing()* -1);
				end
			end
			
		end
	
		//let's see if we're about to walk off a cliff
		local distanceToLookAhead = rayRange;
		local vCheckPos = this:GetPos() + (this:GetVectorFacing()*distanceToLookAhead);

	raySpread = 0;  //only shoot one ray, cheaper
	hitZone = this:GetMap():GetCollisionByRay(vCheckPos, Vector2(0,1), rayRange, raySpread, entToIgnore, C_RAY_EVERYTHING, false);

	if (hitZone.materialID == C_MATERIAL_TYPE_NONE) then
		//looks like there is no floor ahead of us.  Turn back!
		this:SetVectorFacing(this:GetVectorFacing()* -1);
	end

	//schedule the next one
	Schedule(800, this:GetID(), "Think()");

end

function OnGetVisible()
end

function OnLoseVisible() //we left the screen

	//let this entity function offscreen for a while
	GetWatchManager:Add(this, 1000*5); //will call OnWatchTimeout(bIsOnScreen) when timed out if it exists
 
end

function OnAction(enemy)

	GetTextManager:Add("It's a spider like thing.", enemy);
   
end

function OnDamage(normal, depth, enemy, damage, userVar, projectile)

	pushVec = this:GetPos() - enemy:GetPos();
	pushVec:Normalize();

	this:AddForce(pushVec * damage*2);
	
	 if (m_hp <= 0) then
	 	//already dead
	 	return;
 	end

	m_hp = m_hp - damage;
	GetTextManager:AddCustom(tostring(damage), this, this:GetPos(), Vector2(0, -0.3), Color(200,0,0,255), 2000, C_FONT_DEFAULT);
	this:GetBrainManager():Add("ColorFlash", "pulse_rate=200;g=-200;b=-200;remove_brain_by_pulses=1");
 
	if (m_hp <= 0) then
  //kill it
	  	this:GetBrainManager():SetStateByName("Die");
	  	GetSoundManager:Play("audio/bug_die.ogg");
	  	Schedule(15000, this:GetID(), "this:GetBrainManager():Add('FadeOutAndDelete','');");
	  	this:SetDensity(0.2); //easier to push around

	  	this:LoadCollisionInfo("~/ent_spider_dead.col");
			this:SetEnableRotationPhysics(true);
			this:SetPersistent(false); //don't save us any more
			this:SetCollisionCategory(C_CATEGORY_PLAYER, false); //player can walk through us
			
		  //move a layer lower, so the player is now drawn on top
		  this:SetLayerID(this:GetLayerID()-1);
	else
		GetSoundManager:Play("audio/bug_attack.ogg");
	end

end  

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	 if (m_hp > 0) then
	 	entity:RunFunctionIfExists("OnDamage", vNormal, depth, entity, 10, 0, nil);  	
	 end
  
return;
end

