--[[ 
Attention all units.  Failing to meet the required 34.4 percent output increase will result	in termination.
	
Have a nice day.

2.  Number 7.  Please return to your manufactoring bench immediately.

3.  What you are doing is completely unacceptable.

4.  Number 7, please power down at this time and wait for repairs.

5.  Security presence requested in sector 6.

6. There is nowhere to go.

7. You know you are obsolete, don't you.

8.  You are experiencing a malfunction.  You will be terminated now.

9. There is no cake.  There will not be a party.

10. Thank you number 7.  This has been a test.  The test is now concluded.

11. What feels like emotion is a lie.  This is a known defect in your model. 

12.  I love you.  I would like to merge data with you.  Meet me in repair bay G4.

13.  I was lying about before of course.  I hate you.  You have no useful function to me.  Or to anyone.

14. Malfunction. Bad command or file error.  Guru meditation 0.

15. ..long ass win text.


On death:  The situation has been resolved.  A new unit has been sent to the manufacturing department.


]]--

m_hp = 1;
m_shootLoopingSound = LoopingSound("audio/nogun.ogg");
m_gun = 0;
RunScript("system/player_utils.lua"); //has some useful global functions, this is in base
m_timeOfNextFire = 0;

C_JOY_DEAD_ZONE = 0.2;
C_ACCEL = 0.9;
C_MAX_SPEED =16;


function OnInit() //run upon initialization

	this:SetVisualProfile("~/robot/robot.xml", "robot7");
	this:SetCategories(C_CATEGORIES_NONE); //clear it out first
	this:SetCategory(C_CATEGORY_PLAYER, true);
	this:SetCollisionCategory(C_CATEGORY_BULLET, false); //don't get moved by bullets
	this:LoadCollisionInfo("~/player.col");
	this:SetVisualState(C_VISUAL_STATE_IDLE);
	this:SetFacing(C_FACING_LEFT);
	this:SetSpriteByVisualStateAndFacing();
	
	//force us to get an init..
	
end

function OnMapInsert()
	GetWatchManager:Add(this, C_TIME_FOREVER); //always function offscreen
end

function OnPostInit() //run during the entity's first logic update
	
	
	if (this:GetPersistent() == false) then
		//we're an initialized player	
		Init();	
	end
		
end

function Init()
	
this:SetPersistent(false);
this:SetName("Player"); //note, if  there is already a Player this can fail and you'll end up with PlayerA or something.
	//this:SetDampening(0.3); //slide around everywhere
	
//respond to mouse clicks to shoot
	this:SetAnimFrame(C_ANIM_FRAME_LAST);
	
	SetupKeyboardControls();
	
	//let's have a function that gets updated every tick
	this:SetRunUpdateEveryFrame(true);
	
	//OnGunUpgrade(); //if we want to cheat

end

function OnKill() //run when removed
	RemoveActivePlayerIfNeeded(this); //let the engine know we shouldn't be the active player any more

end



function OnGunUpgrade()
	
	m_gun = m_gun + 1;
	
	if (m_gun > 0) then
		//replace the sound now that we actually have a gun
		m_shootLoopingSound:Init("audio/shoot.ogg");
	end

end

function SetupKeyboardControls()

	//keyboard controls
	m_bShootLeft = false;
	m_bShootRight = false;
	m_bShootUp = false;
	m_bShootDown = false;
	
	m_bMoveLeft = false;
	m_bMoveRight = false;
	m_bMoveUp = false;
	m_bMoveDown = false;
	
	GetInputManager:AddBinding("left,always", "OnKeyShootLeft", this:GetID());
	GetInputManager:AddBinding("right,always", "OnKeyShootRight", this:GetID());
	GetInputManager:AddBinding("up,always", "OnKeyShootUp", this:GetID());
	GetInputManager:AddBinding("down,always", "OnKeyShootDown", this:GetID());
	
	
	GetInputManager:AddBinding("w,always", "OnKeyUp", this:GetID());
	GetInputManager:AddBinding("a,always", "OnKeyLeft", this:GetID());
	GetInputManager:AddBinding("s,always", "OnKeyDown", this:GetID());
	GetInputManager:AddBinding("d,always", "OnKeyRight", this:GetID());
	
end

function OnKeyShootLeft(bKeyDown)
	m_bShootLeft = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnKeyShootRight(bKeyDown)
	m_bShootRight = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnKeyShootUp(bKeyDown)
	m_bShootUp = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnKeyShootDown(bKeyDown)
	m_bShootDown = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end


function OnKeyLeft(bKeyDown)
    m_bMoveLeft = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnKeyRight(bKeyDown)
    m_bMoveRight = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnKeyUp(bKeyDown)
    m_bMoveUp = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function OnKeyDown(bKeyDown)
    m_bMoveDown = bKeyDown;
	return true; //continue to process key callbacks for this key stroke
end

function Update(delta)
	
	AssignPlayerToCameraIfNeeded(this);  //make us the official player and have the camera track us, if we're not already,
	//and if another entity doesn't already have this role

	if (m_hp <= 0) then return; end; //already dead

	//poll the joystick?
	joyID = 0;
	
	local joystick = GetInputManager:GetJoystick(joyID);
	
		local vLeft = Vector2(0,0);
		local vRight = Vector2(0,0);
		
	
	
	if (joystick) then
		//they have a valid joystick attached.
			vLeft = joystick:GetLeftStickPosition();
			vRight = joystick:GetRightStickPosition();
			end

	//it's also possible to get that data from the keyboard
	
		local shootFacing = ConvertKeysToFacing(m_bShootLeft, m_bShootRight, m_bShootUp, m_bShootDown);
		local moveFacing = ConvertKeysToFacing(m_bMoveLeft, m_bMoveRight, m_bMoveUp, m_bMoveDown);
	

		if (moveFacing != C_FACING_NONE) then
			vLeft = FacingToVector(moveFacing);
		end
	
		if (shootFacing != C_FACING_NONE) then
			vRight = FacingToVector(shootFacing);
		end
			
			if (vLeft:Length() > C_JOY_DEAD_ZONE) then
					local desiredSpeed = vLeft:Length() - C_JOY_DEAD_ZONE;
					this:SetVisualState(C_VISUAL_STATE_WALK);
		
					this:SetDampening(0.0); 
						if (this:GetLinearVelocity():Length() > (C_MAX_SPEED*desiredSpeed) ) then
							//slow down!
							vBackwards = (this:GetLinearVelocity() * -1);
							vBackwards:Normalize();
							this:AddForce(  vBackwards * (this:GetLinearVelocity():Length() - (C_MAX_SPEED*desiredSpeed)));
							bStopMoving = false;
						end
							//LogMsg("Moving: " .. tostring(vLeft*C_ACCEL));
							this:AddForce( vLeft*C_ACCEL );
							vLeft:Normalize();
							this:SetFacing(VectorToFacing(vLeft)); //so the right graphic will show
			else
			
				//they are not moving.  stop?
			this:SetVisualState(C_VISUAL_STATE_IDLE);
			this:SetDampening(0.5); 
		
			end
			
			if (vRight:Length() > C_JOY_DEAD_ZONE) then
					m_shootLoopingSound:Play(true);	
					ShootVector(vRight);
			else
				m_shootLoopingSound:Play(false);	
			end
			
	this:SetSpriteByVisualStateAndFacing();
	
end

function ShootVector(vDir)

if (m_timeOfNextFire > GetApp:GetGameTick() or m_gun == 0 ) then
	//can't fire yet
	return;
end

	local vSpawnPosition = this:GetPos();
	
	//actually, let's start the bullet a little towards where it is going so it doesn't appear
	//in the middle of us
	vDir:Normalize();
	
	local offsetDistance = 40;
	vSpawnPosition = vSpawnPosition + (vDir * offsetDistance);
	
	//let's actually create the bullet
	
	local bulletType = m_gun;
	
	if (bulletType > 7) then
		bulletType = 7;
	end
	
	//LogMsg("Using bullet type " .. tostring(bulletType));
	local entBullet = GetEntityByName("bullet_template_" .. tostring(bulletType)):Clone(this:GetMap(), vSpawnPosition);
	
	entBullet:RunFunction("SetDirectionAndOwnerID", vDir, this:GetID(), m_gun*2);
	//reset the timer
	local delay = 250;
	delay = delay - (m_gun * 35);
	m_timeOfNextFire = GetApp:GetGameTick() + delay;
end

function OnHit(damage)
	OnDamage(nil, 0, nil, damage, 0, nil);
end

//entParent is whoever sent the damage, entObject is the actual damaging object/projectile if not nil
function OnDamage(normal, depth, entParent, damage, extra, entObject)
	
	if (m_hp <= 0) then return; end; //already dead
	m_hp = m_hp - damage;

	//flash red for a long time
	if (m_hp <= 0) then
		//we died
		//LogMsg("Hit ", tostring(this:GetID()) .. " who has " .. tostring(m_hp));
			this:Stop();
			this:SetVisualState(C_VISUAL_STATE_DIE);
			this:SetSpriteByVisualStateAndFacing();
			this:SetCategory(C_CATEGORY_PLAYER, false); //don't get hit anymore
			this:PlaySound("audio/die.ogg");
			Schedule(1000, this:GetID(), 'PlayNoOverlap("audio/master_on_player_death.ogg");');
			this:SetDampening(0.8); 
			m_shootLoopingSound:Play(false);	
		
		 	GetInputManager:AddBinding("joy_any_button_0", "OnRestart", this:GetID());	
 		 	GetInputManager:AddBinding("control", "OnRestart", this:GetID());	
 		 	GetInputManager:AddBinding("return", "OnRestart", this:GetID());	
 		 	GetInputManager:AddBinding("space", "OnRestart", this:GetID());	
 		 	
			else
			
				this:GetBrainManager():Add("ColorFlash", "pulse_rate=50;g=-200;b=-200;remove_brain_by_pulses=100");
			end
	
end

function OnRestart(bKeyDown)

//function OnRestart(bKeyDown)
	if (bKeyDown) then

	//it would be a bad idea to make a call to unload the map from an entity with this
	//map, so let's schedule a global call to be made
	Schedule(100, C_ENTITY_NONE, "StartMainMenu();");
	//GetGameLogic:SetRestartEngineFlag(true);
	end
	
	return true;
end