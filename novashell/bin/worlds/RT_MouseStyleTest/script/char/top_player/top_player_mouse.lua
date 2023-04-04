	
function OnInit()
	
	this:SetVisualProfile("~/../top_troll/top_troll.xml", "top_troll");
	this:LoadCollisionInfo("~/../top_troll/top_troll.col");
	
end

function OnPostInit()

	this:GetBrainManager():Add("StandardBase","");
	this:GetBrainManager():SetStateByName("Idle");
 
 	GetGameLogic:SetPlayer(this);
	GetCamera:SetEntityTrackingByID(this:GetID());
	
	GetInputManager:AddBinding("mouse_left", "LeftMouseClick", this:GetID());
	
	
	Schedule(1000, C_ENTITY_NONE,
			'GetTextManager:AddCustomScreen("Click with the left mouse button to move around.", Vector2(C_SCREEN_X/2,C_SCREEN_Y-40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);'
		 );
	Schedule(5000, C_ENTITY_NONE,
			'GetTextManager:AddCustomScreen("Hold down TAB to move faster.", Vector2(C_SCREEN_X/2,C_SCREEN_Y-40), Vector2(0, 0), Color(200,200,0,255), 4000, C_FONT_DEFAULT);'
		 );
	
end

function OnKill()
	
end

function LeftMouseClick(bButtonDown)

if (bButtonDown == false) then
		return true; //continue to process other callbacks for this key/button press
	end
	
	local pt = ScreenToWorld(GetInputManager:GetMousePos());
	local ent = GetEntityByWorldPos(pt, this, true); //get all entities but ignore ourself, be pixel accurate
	
	
	//cancel whatever they were doing, if we didn't, it would just add to the queue...
	this:GetGoalManager():RemoveAllSubgoals();

	if (ent != nil) then
		
	
			//approach is different then move, it understands you want to face an object and calculates
			//a good place to stand.
			
			if (ent:GetName() != "") then
				GetTextManager:Add("Moving to look at " .. ent:GetName(), this);
			else
				GetTextManager:Add("Moving to look at an unnamed object.", this);
			
			end
					
			this:GetGoalManager():AddApproach(ent:GetID(), C_DISTANCE_CLOSE);
			return true;
		
		
	end


	//just move to a physical point.  We don't really handle what happpens if the point
	//is impossible to reach yet.
	if (this:IsValidPosition(this:GetMap(), pt, true)) then
		this:GetGoalManager():AddMoveToPosition(pt);
			
	else 
		GetTextManager:Add("I won't fit there", this);
	end
	
	return true; //continue to process other callbacks for this key/button press
	
end


function OnAction(enemy)

end

function OnDamage(normal, depth, enemy, damage, userVar, projectile)

	pushVec = this:GetPos() - enemy:GetPos();
	pushVec:Normalize();
	this:AddForce(pushVec * damage);
	GetTextManager:AddCustom(tostring(damage), this, this:GetPos(), Vector2(0, -0.3), Color(200,0,0,255), 2000, C_FONT_DEFAULT);
	this:GetBrainManager():Add("ColorFlash", "pulse_rate=200;g=-200;b=-200;remove_brain_by_pulses=1");
  
end


