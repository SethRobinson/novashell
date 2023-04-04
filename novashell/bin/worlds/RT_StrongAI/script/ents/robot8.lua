m_hp = 5;
m_copyKeyName = "IsCopy";

function OnInit() //run upon initialization
	this:SetType(C_TYPE_ENEMY);
	
	this:SetVisualProfile("~/robot/robot.xml", "robot8");
	this:LoadCollisionInfo("~/player.col");
	this:SetCategories(C_CATEGORIES_NONE);

	this:SetVisualState(C_VISUAL_STATE_IDLE);
	this:SetFacing(C_FACING_LEFT);
	this:SetSpriteByVisualStateAndFacing();
	this:SetIsCreature(true);
end

function OnPostInit() //run during the entity's first logic update

	local col = this:GetBaseColor();
	
	if (this:Data():Exists(m_copyKeyName)) then
			//we're a copy, not persistent, and will interact with the player
			this:SetDensity(2);
			col:SetAlpha(255);
			this:SetPersistent(false);
			this:SetLayerByName("Main");
			this:SetDampening(0.1); //so we don't slide everywhere too crazy
			this:SetCategory(C_CATEGORY_ENEMY, true);

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


function OnKill() //run when removed
end

function OnHit(damage) 
	

	if (m_hp <= 0) then return; end; //already dead
	m_hp = m_hp - damage;
	
	if (m_hp <= 0) then
		//we died
		//LogMsg("Hit ", tostring(this:GetID()) .. " who has " .. tostring(m_hp));
			this:Stop();
			this:SetVisualState(C_VISUAL_STATE_DIE);
			this:SetSpriteByVisualStateAndFacing();
			this:SetCollisionCategories(C_CATEGORIES_NONE);
			this:SetCategories(C_CATEGORIES_NONE); //so nothing else can 'listen' to our collisions

			this:PlaySoundPositioned("audio/die.ogg");
	else
				this:PlaySoundPositioned("audio/glance.ogg");

		//show damage effect?
	//	if (this:GetBrainManager():GetBrainByName("ColorFlash") != nil) then
			this:GetBrainManager():Add("ColorFlash", "pulse_rate=110;r=100;g=100;b=100;remove_brain_by_pulses=1");
		//	end;
	
	end
end


