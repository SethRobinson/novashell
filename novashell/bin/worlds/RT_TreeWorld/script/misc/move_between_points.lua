//these are existing waypoints/named objects on the map
//this script example shows how to make an entity move between two points 'manually', without using the
//pathfinding system.

m_tagA = "MyTagA"
m_tagB = "MyTagB"

m_closenessNeededToTarget = 50; //how close we need to to assume we reached our target
m_speed = 3; //how fast we move
m_curTargetPos = Vector2(0,0);

function SetNewTarget()

	if (m_curTargetPos == GetTagManager:GetPosFromName(m_tagA)) then
		//well, let's go to B then
		m_curTargetPos = GetTagManager:GetPosFromName(m_tagB);
	else
		//well, let's go to A then
		m_curTargetPos = GetTagManager:GetPosFromName(m_tagA);
	end
	
end

function OnInit() //run upon initialization
	this:SetGravityOverride(0);
  this:SetDampening(0);
	this:SetType(C_TYPE_PLATFORM);
  SetNewTarget();
end

function OnPostInit() //run during the entity's first logic update
	GetWatchManager:Add(this, C_TIME_FOREVER); //always function offscreen
	//let's have a function that gets run every logic tick
	this:SetRunUpdateEveryFrame(true);
end

//this is run every logic tick
function Update(step)

  //remove all movement, no momemtum
	this:AddForceConstant(this:GetLinearVelocity() *-1);
	
	//move towards target
	this:AddForceConstant(this:GetVectorToPosition(m_curTargetPos)*m_speed);

  if (this:GetDistanceFromPosition(m_curTargetPos) < m_closenessNeededToTarget) then
   	//we reached out target!  Set a new one.
   	SetNewTarget();
  end

end

function OnCollision(normal, depth, materialID, ent) //return true/false to allow/disallow the collision

	//LogMsg("Hit Static: Depth: " .. tostring(depth) .. " normal: " .. tostring(normal).." Ent Linear: " .. ent:GetLinearVelocity().y );
	if (normal.y >  0.9 ) then
		return true;
	end
	return false;
end
