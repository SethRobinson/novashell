//a pretty hacked up basic (ie, stupid) AI that doesn't use the GoalManager at all
//For an example of smarter AI, look at the script the ducks use in the Dink example, they 
//can walk approach a player and stop in front of him, even walking around trees and through doors

function OnInit()
	
	this:SetVisualProfile("~/top_troll.xml", "top_troll");
	this:LoadCollisionInfo("~/top_troll.col");
	
end

function OnPostInit()
	
	m_hp = 4;
		
	if this:GetScale().x < 1 then
		m_hp = 5; //he's small I guess, give him less HP
	 end
	
	this:GetBrainManager():Add("StandardBase","");
	this:GetBrainManager():SetStateByName("Idle");
	
	this:SetCategory(C_CATEGORY_ENEMY, true);
	this:SetCategory(C_CATEGORY_ENTITY, false);
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
	

	if (GameIsActive() == false) then
		//no profile is active, so we must be in the editor.  Let's not have them running around
		this:GetBrainManager():SetStateByName("Frozen");
	end

end

function OnAttackLoop()

	//attack anim is finished playing	
	this:GetBrainManager():SetStateByName("Idle");
end


function OnIdleLoop()

	if (this:GetGoalManager():GetGoalCount() > 0) then
		return;
	end
	
if (CanAttack()) then return; end;

	if (math.random(2) == 1) then
		this:GetBrainManager():SetStateByName("Walk");
	else
	if (math.random(20) == 1) then
		
		r = math.random(6);
		
		if (r == 1) then GetTextManager:Add("I'm gonna git' you.", this); end;
		if (r == 2) then GetTextManager:Add("Daddy's coming, bitch!", this); end;
		if (r == 3) then GetTextManager:Add("Prepare to eat club.", this); end;
		if (r == 4) then GetTextManager:Add("You gonna die!", this); end;
		if (r == 5) then GetTextManager:Add("I heart human flesh is my bumper sticker.", this); end;
		if (r == 6) then GetTextManager:Add("He's trying to flank!", this); end;
		
		end
	end
end

function CanAttack()

	if (GetPlayer and this:GetDistanceFromEntityByID(GetPlayer:GetID()) < 50) then
	
		//well, we're close enough to the player, but we better check to see if we're facing
		//towards him or not.
		
		//first, let's tell the AI we want to rotate towards him
		this:SetVectorFacingTarget(this:GetVectorToEntity(GetPlayer));
		
		//only if we're already looking at him should we swing
		
		local toleranceAngleInRadians = 1.3;
		if (this:IsFacingTarget(toleranceAngleInRadians)) then
		
			this:GetBrainManager():SetStateByName("Attack");
			GetSoundManager:Play("audio/effects/swing.ogg");
			
			//setup where the actual damage will happen
		    Schedule(200, this:GetID(), 'ent = CreateEntity(this:GetMap(), this:GetPos(), "weapons/top_damage/top_damage.lua"); ent:RunFunction("SetParent", this); ent:RunFunction("SetStrength", 5);');
			
			return true;
		end
	end

return false;
end

function OnWalkLoop()


	if (this:GetGoalManager():GetGoalCount() > 0) then
		return;
	end
	
	
//close enough to hit?

if (CanAttack()) then return; end;

if (math.random(5) == 1) then
		this:GetBrainManager():SetStateByName("Idle");
	else
	
	//walk towards player
		PlayBattleMusic() //from audio_utils.lua which was loaded in game_start.lua
	
	if GetPlayer != nil then
		v = GetPlayer:GetPos()-this:GetPos();
		v:Normalize();
		this:SetVectorFacingTarget(v);
		else
	
			//just walk randomly around
			this:SetFacingTarget(math.random(C_FACING_COUNT)-1);
		end

	end
	
end


function OnAction(enemy)

	GetTextManager:Add("Troll!", enemy);
   
end

function OnDamage(normal, depth, enemy, damage, userVar, projectile)

	pushVec = this:GetPos() - enemy:GetPos();
	pushVec:Normalize();

	this:AddForce(pushVec * damage);
	
	 if (m_hp <= 0) then
	 	return; //already dead
 	end


	m_hp = m_hp - damage;
	
	GetTextManager:AddCustom(tostring(damage), this, this:GetPos(), Vector2(0, -0.3), Color(200,0,0,255), 2000, C_FONT_DEFAULT);

   this:GetBrainManager():Add("ColorFlash", "pulse_rate=200;g=-200;b=-200;remove_brain_by_pulses=1");
 
  if (m_hp <= 0) then
  //kill it
	  
	  	this:GetBrainManager():SetStateByName("Die");
	  	GetSoundManager:Play("audio/bug_die.ogg");
	  	Schedule(15000, this:GetID(), "this:GetBrainManager():Add('FadeOutAndDelete','');");
	  
	    //this:LoadCollisionInfo("~/troll_dead.col");
		this:SetCollisionCategories(C_CATEGORIES_NONE);
		this:SetPersistent(false); //don't save us any more
    EndBattleMusicIfNoEnemiesAreClose();  //from audio_utils.lua which was loaded in game_start.lua
   else
     	GetSoundManager:Play("audio/bug_attack.ogg");
   end
  
end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)
//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	 if (m_hp > 0) then
	 //if we wanted him to hurt on touch..
	 //enemy:RunFunctionIfExists("OnDamage", normal, depth, enemy, 10, 0, nil);  	
	 end
  
return true;
end

