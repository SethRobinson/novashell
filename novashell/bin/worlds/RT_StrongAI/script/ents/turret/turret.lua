m_hp = 5;
m_copyKeyName = "IsCopy";
m_eyesight = 800;
m_thinkDelay = 750;
m_bHaveRunPostInit = false;

function OnInit() //run upon initialization
	this:SetType(C_TYPE_ENEMY);
	this:SetVisualProfile("~/turret.xml", "turret");
	this:LoadCollisionInfo("~/turret.col");
	this:SetCategories(C_CATEGORIES_NONE);
	this:SetVisualState(C_VISUAL_STATE_IDLE);
	this:SetFacing(C_FACING_LEFT);
	this:SetSpriteByVisualStateAndFacing();

end

function OnPostInit() //run during the entity's first logic update

	local col = this:GetBaseColor();
	
	if (this:Data():Exists(m_copyKeyName)) then
			//we're a copy, not persistent, and will interact with the player
			this:SetDensity(2);
			col:SetAlpha(255);
			this:SetPersistent(false);
			this:SetLayerByName("Main");
			this:SetCategory(C_CATEGORY_ENEMY, true);

			OnStartup();
	
	else
		//We're a master, not a copy.  If we haven't already made a copy of ourselves this game, do it

	  if (this:Data():GetNum("GameStartTime") != g_timeStart) then		

			//duplicate ourselves as a non-persistent version for the player to play with

		  this:Data():Set(m_copyKeyName,""); //put this here temporarily so when we copy ourselves, they will know they are just a copy
			local ent = this:Clone(this:GetMap(), this:GetPos()+Vector2(1,1)); //add an offset, easier to see the master
			this:Data():Delete(m_copyKeyName); //remove this, we only added for the benefit of the copy we just mademade
			this:Data():SetNum("GameStartTime",g_timeStart); //avoid making more copies even if we're reinitted
		end

		col:SetAlpha(100); //look all shadowy, makes it clearer in the editor that we're a copy
		this:SetCollisionCategories(C_CATEGORIES_NONE);
	end
	
		this:SetBaseColor(col);
	
end

function OnStartup()
	
	
		this:GetBrainManager():Add("StandardBase","");
		this:GetBrainManager():SetStateByName("Idle");
		this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
		this:SetIsCreature(true);
		Schedule(m_thinkDelay, this:GetID(), "OnLookAround()");
		
		if (this:Data():Exists("hp")) then
			//read our HP from data entered in the editor
			m_hp = this:Data():GetNum("hp");
		end
		
			if (this:Data():Exists("eyesight")) then
			//read our HP from data entered in the editor
			m_eyesight = this:Data():GetNum("eyesight");
		end
		
		if (this:Data():Exists("thinkdelay")) then
			//read our HP from data entered in the editor
			m_thinkDelay = this:Data():GetNum("thinkdelay");
		end
end


function OnLookAround()
	
	if (m_hp <= 0) then return; end; //already dead

	if (this:IsCloseToEntity(GetPlayer, m_eyesight))then
		if ( this:HasLineOfSightToPosition(GetPlayer:GetPos(), true) ) then
		
			if (this:Data():Exists("stationary") == false) then
	//chase him?
			this:GetGoalManager():RemoveAllSubgoals();
			this:GetGoalManager():PushMoveToPosition(GetPlayer:GetPos());
			end
			
			//shoot him?
			if (this:Data():Exists("shooting")) then
	
			local vDir = this:GetVectorToEntity(GetPlayer);
			
			local offsetDistance = 40;
			local vSpawnPosition = this:GetPos() + (vDir * offsetDistance);

	
			local entBullet = GetEntityByName("bullet_template_round"):Clone(this:GetMap(), vSpawnPosition);
			entBullet:RunFunction("SetDirectionAndOwnerID", vDir, this:GetID(), 1);
			entBullet:RunFunction("SetSpeed", 8);
			this:PlaySoundPositioned("audio/enemy_shoot.ogg");
	
		end
		
			
		end
	end
	
	Schedule(m_thinkDelay, this:GetID(), "OnLookAround()");
	
end

function OnKill() //run when removed
end

function OnHit(damage) 
	
	if (m_hp <= 0) then return; end; //already dead
	m_hp = m_hp - damage;
	
	if (m_hp <= 0) then
		//we died
		//LogMsg("Hit ", tostring(this:GetID()) .. " who has " .. tostring(m_hp));
			this:GetGoalManager():RemoveAllSubgoals();
	
			this:Stop();
			this:GetBrainManager():SetStateByName("Die");
			this:SetCollisionCategories(C_CATEGORIES_NONE);
			this:SetCollisionListenCategories(C_CATEGORIES_NONE);
			this:SetCategories(C_CATEGORIES_NONE); //so nothing else can 'listen' to our collisions

			this:PlaySoundPositioned("audio/die.ogg");
	else
	
		//show damage effect?
	//	if (this:GetBrainManager():GetBrainByName("ColorFlash") != nil) then
			this:GetBrainManager():Add("ColorFlash", "pulse_rate=110;r=100;g=100;b=100;remove_brain_by_pulses=1");
			this:PlaySoundPositioned("audio/glance.ogg");

		//	end;
	
	end
end


//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)
//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

		//if this is called, we must have hit the player

  	//let the player know by calling his OnDamage.  Send extra info just in case he wants it.
  	entity:RunFunction("OnDamage", vNormal, depth, nil, 5, 0, this);
end