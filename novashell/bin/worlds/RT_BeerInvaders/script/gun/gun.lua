//this 'gun' is actually our player.

//define some variables that will be active in this entity's namespace only

C_MIN_TIME_BETWEEN_SHOTS_MS = 200;

m_bLeft = false; //we'll remember the keystates for keyboard movement in case they don't like the mouse
m_bRight = false;
m_vLastMousePos = Vector2(0,0);
m_shotTimer = 0; //if this is less than GetApp:GetGameTick() then they can fire

function OnInit() //run upon initialization
	
	this:SetVisualProfile("~/gun.xml", "gun");
	this:LoadCollisionInfo("~/gun.col");
	this:SetCategory(C_CATEGORY_ENTITY, false);
	this:SetCategory(C_CATEGORY_PLAYER, true); //only want to be a player
	this:SetCollisionCategories(C_CATEGORIES_NONE); //disable all collisions
	this:SetCollisionCategory(C_CATEGORY_TILEPIC, true); //turn back on one
end

function OnPostInit() //run during the entity's first logic update
	//respond to mouse clicks to shoot
	this:SetAnimFrame(C_ANIM_FRAME_LAST);

	GetInputManager:AddBinding("left,always", "OnLeft", this:GetID());
	GetInputManager:AddBinding("right,always", "OnRight", this:GetID());

	GetInputManager:AddBinding("mouse_left", "OnMouseLeft", this:GetID());
	GetInputManager:AddBinding("control", "OnMouseLeft", this:GetID());
	
	//let's have a function that gets updated every tick
	this:SetRunUpdateEveryFrame(true);
	
	m_hitpoints = 1;
	
end

//this is run every logic tick
function Update(step)
	
	ProcessMovement();
	
end

function IsAlive()
	return m_hitpoints > 0;
end

function OnHit(damage) //run when a bullet notifies us that we got hit
	 
	 if (IsAlive()) then	
	 		//we're dead!
			this:GetBrainManager():Add("ColorFlash", "pulse_rate=50;g=-200;b=-200;remove_brain_by_pulses=10");
		 	//this:PlaySound("~/sound/explode.ogg");
		 	m_hitpoints = m_hitpoints-1;
		 	
		 	if (IsAlive() == false) then

		 			OnPlayerDeath(); 
		 		
		 	end
		 	
		 	
	end	
end

function ProcessMovement()

	if (IsAlive() == false) then return; end;
	
	local curSpeed = this:GetLinearVelocity():Length();
	local maxSpeed = 14;
	local direction = C_FACING_NONE;
	
	if (m_bRight) then
		direction = C_FACING_RIGHT;
	end

	if (m_bLeft) then
		direction = C_FACING_LEFT;
	end
	//LogMsg("Going", direction);
	if (direction != C_FACING_NONE) then
		
		this:SetDampening(0.0);

		//let's move the way they want
			vecAngle = 	FacingToVector(direction);
			powerMod = maxSpeed-curSpeed;
			this:AddForce( (vecAngle*maxSpeed) - this:GetLinearVelocity());

	else
		this:SetDampening(0.8);
		
	end
	
	if (m_vLastMousePos != GetInputManager:GetMousePos()) then
		//let's let the mouse control it too, but only if it's within range
		
		m_vLastMousePos = GetInputManager:GetMousePos();
		
		if (m_vLastMousePos.x > 0 and m_vLastMousePos.x < C_SCREEN_X) then
			this:SetPos(Vector2(m_vLastMousePos.x, this:GetPos().y)); //set only the X, we don't care about the mouses y
		end
	end

end

function OnKill() //run when removed

end

function ShootGun()

	if (IsAlive() == false) then return; end;
	
	if (m_shotTimer > GetApp:GetGameTick()) then
			return; //they aren't allowed to fire yet.
		end
		
	//reset the shot timer so they can't fire again too soon
	m_shotTimer = GetApp:GetGameTick() + C_MIN_TIME_BETWEEN_SHOTS_MS;
		
	this:SetAnimFrame(0); //rewind the animation so it will play
	this:PlaySound("audio/shoot.ogg");

	local vBulletDirection = Vector2(0,-1);
	local vSpawnPosition = this:GetPos();
	
	//actually, let's start the bullet a little towards where it is going so it doesn't appear
	//in the middle of us
	
	local offsetDistance = 50;
	vSpawnPosition = vSpawnPosition + (vBulletDirection * offsetDistance);
	
	//let's actually create the bullet
	local entBullet = GetEntityByName("bullet_template"):Clone(this:GetMap(), vSpawnPosition);
	entBullet:RunFunction("SetDirectionAndOwnerID", vBulletDirection, this:GetID());

end

function OnMouseLeft(bKeyDown)
	
	//we only care if they pressed down the button, not if they are releasing it
	if (bKeyDown == false) then return true; end;

	//shoot!
	ShootGun();

	return true; //keep calling callbacks for this keypress
end

function OnLeft(bKeyDown)
	m_bLeft = bKeyDown;
	return true;
end

function OnRight(bKeyDown)
	m_bRight = bKeyDown;
	return true;
end