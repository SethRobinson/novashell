
function OnInit() //run upon initialization
	
	this:SetCategory(C_CATEGORY_ENTITY, false);
	this:SetCategory(C_CATEGORY_ENEMY, true);
	this:SetVisualProfile("~/can.xml", "can");
	this:LoadCollisionInfo("~/can.col");
	this:SetEnableRotationPhysics(true);
	
end

function OnPostInit() //run during the entity's first logic update

	//let's have it bob up and down a bit
	this:GetBrainManager():Add("Bob","distance=5;speed=2;visual_interval_ms=10");
	//this:SetDampening(0.1); //slide around everywhere
	
	m_maxHitpoints = 1; //make a variable to store hitpoints in
	m_hitpoints = m_maxHitpoints;
	
	OnAddEnemy(); //run this in init_level.lua so it can keep track of how many of us there are
	
	//let's think about shooting every so often
	ScheduleThink();
end

function ScheduleThink()
	Schedule(1000+random(3000), this:GetID(), "OnThink();");
end

function HoveringOverPlayer()
	local entGun = GetEntityByName("Gun");
	
	if (entGun != nil) then
		
		//abs turns it into a positive number so the next statement can be simpler
		distanceFromPlayerX = math.abs(this:GetPos().x-entGun:GetPos().x);
		
		if (distanceFromPlayerX < 200) then
			return true; //we're close enough, say yes
		end
	end

return false; //we're not really close enough to the player horizontally
end

function OnShootStart()

	//blink green as a warning, then launch something at the player
	this:GetBrainManager():Add("ColorFlash", "pulse_rate=50;r=-200;b=-200;remove_brain_by_pulses=10");
	Schedule(600, this:GetID(), "OnShootFinish()"); //actually launch the bullet 
end

function OnShootFinish()

	//instead of using the Clone method we used in init_level.lua, let's try another
	//method, just creating something and assigning a script to it.
	
	//we ALWAYS only deal with entity pointers, so even though the pointer is local, CreateEntity
	//is always making something that will exist until it's specifically deleted
	
	local vOffset = Vector2(0,50); //if we spawn in the middle it looks kind of funny
	local entBullet = CreateEntity(this:GetMap(), this:GetPos()+vOffset, "can/can_bullet.lua");
	
end

function OnThink()

	//LogMsg("Thinking!");

	if (HoveringOverPlayer()) then
		OnShootStart();
	end

	//when we should think again
	ScheduleThink();
end

function OnHit(damage) //run when a bullet notifies us that we got hit
	 	 	if (m_hitpoints <= 0) then
	 	 		LogMsg("Already dead..");
	 	 		return;
	 	 	end
		
	 	m_hitpoints = m_hitpoints - 1;
	 	
	 	if (m_hitpoints <= 0) then
	 		//we're dead!
		 	this:GetBrainManager():Add("FadeOutAndDelete", "fade_speed_ms=50");
			this:PlaySound("~/sound/explode.ogg");
			m_hitpoints = 0;

			OnRemoveEnemy(); //run this in init_level.lua so it can keep track of how many of us there are
	
			//make an explosion here, actually defined in base/script/effects/effect_utils.lua
			CreateExplosion(this:GetPos());
	
		end

	//change the color to match the damage.  0 is no color, 255 is full color, so by 
	//limiting the non-red colors, we make it look redder as it takes more damage.
	greenAndBlueColor = (m_hitpoints/m_maxHitpoints)*255;
	this:SetBaseColor(Color(255, greenAndBlueColor, greenAndBlueColor, 255));
	
end


function OnKill() //run when removed
end

